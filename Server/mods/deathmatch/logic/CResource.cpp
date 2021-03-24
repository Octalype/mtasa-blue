/*****************************************************************************
 *
 *  PROJECT:     Multi Theft Auto
 *  LICENSE:     See LICENSE in the top level directory
 *  PURPOSE:     Single resource unit handler
 *
 *  Multi Theft Auto is available from https://multitheftauto.com/
 *
 *****************************************************************************/

#include "StdInc.h"
#include "CResource.h"
#include "CResourceScriptItem.h"
#include "CResourceConfigItem.h"
#include "CResourceHTMLItem.h"
#include "CResourceClientFileItem.h"
#include "CResourceClientConfigItem.h"
#include "MetaFileParser.h"
#include "ResourceFileRouter.h"

extern CServerInterface* g_pServerInterface;
extern CGame*            g_pGame;

namespace fs = std::filesystem;

static bool IsRegularFile(const fs::path& filePath)
{
    std::error_code ec;
    return fs::is_regular_file(filePath, ec) && !ec;
}

static void unzConvertToEpochTime(const tm_unz& tmu_date, struct tm& calendarTime)
{
    memset(&calendarTime, 0, sizeof(calendarTime));
    calendarTime.tm_sec = tmu_date.tm_sec;
    calendarTime.tm_min = tmu_date.tm_min;
    calendarTime.tm_hour = tmu_date.tm_hour;
    calendarTime.tm_mday = tmu_date.tm_mday;
    calendarTime.tm_mon = tmu_date.tm_mon;
    calendarTime.tm_year = tmu_date.tm_year - 1900;
    calendarTime.tm_isdst = -1;
}

CResource::CResource(CResourceManager* pResourceManager, bool bIsZipped, const char* szAbsPath, const char* szResourceName)
    : m_pResourceManager(pResourceManager), m_bResourceIsZip(bIsZipped), m_strResourceName(SStringX(szResourceName)), m_strAbsPath(SStringX(szAbsPath))
{
    m_uiScriptID = CIdArray::PopUniqueId(this, EIdClass::RESOURCE);

    Load();
}

bool CResource::Load()
{
    if (m_eState != EResourceState::None)
        return true;

    m_strCircularInclude = "";
    m_metaChecksum = CChecksum();
    m_bIsPersistent = false;
    m_bLinked = false;
    m_pResourceElement = nullptr;
    m_pResourceDynamicElementRoot = nullptr;
    m_bProtected = false;
    m_bStartedManually = false;
    m_bDoneDbConnectMysqlScan = false;

    m_bClientConfigs = true;
    m_bClientScripts = true;
    m_bClientFiles = true;

    m_bOOPEnabledInMetaXml = false;

    m_pVM = nullptr;
    // @@@@@ Set some type of HTTP access here

    // Register the time we loaded this resource and zero out the time we started it
    time(&m_timeLoaded);
    m_timeStarted = 0;

    // Store the actual directory and zip paths for fast access
    m_strResourceDirectoryPath = PathJoin(m_strAbsPath, m_strResourceName, "/");
    m_strResourceCachePath = PathJoin(g_pServerInterface->GetServerModPath(), "resource-cache", "unzipped", m_strResourceName, "/");

    m_rootDirectory = fs::path{m_strAbsPath.c_str(), fs::path::format::generic_format} / m_strResourceName.c_str();

    m_archiveFilePath = m_rootDirectory;
    m_archiveFilePath += ".zip";

    if (m_bResourceIsZip)
    {
        m_staticRootDirectory = fs::path{m_strResourceCachePath, fs::path::format::generic_format};

        if (!UnzipResource())
        {
            return false;
        }

        m_zipHash = CChecksum::GenerateChecksumFromFileUnsafe(m_archiveFilePath.string());
    }
    else
    {
        m_staticRootDirectory = fs::path{m_strResourceDirectoryPath, fs::path::format::generic_format};
    }

    // Load the meta.xml file
    m_metaFilePath = m_staticRootDirectory / "meta.xml";

    if (!IsRegularFile(m_metaFilePath))
    {
        m_strFailureReason = "Couldn't find meta.xml file for resource '"s + m_strResourceName + "'\n"s;
        CLogger::ErrorPrintf(m_strFailureReason.c_str());
        return false;
    }

    mtasa::MetaFileParser meta{m_strResourceName};
    std::string           parserError = meta.Parse(m_metaFilePath);

    if (!parserError.empty())
    {
        m_strFailureReason = SString("Couldn't parse meta file for resource '%s' [%s]\n", m_strResourceName.c_str(), parserError.c_str());
        CLogger::ErrorPrintf(m_strFailureReason.c_str());
        return false;
    }

    // Process resource meta information from parser
    if (!ProcessMeta(meta))
        return false;

    // Generate a CRC for this resource
    if (!GenerateChecksums())
        return false;

    // Register this resource name in the embedded http server
    m_httpRouter = std::make_unique<mtasa::ResourceFileRouter>(*this);

    m_eState = EResourceState::Loaded;
    m_bDoneUpgradeWarnings = false;
    return true;
}

bool CResource::Unload()
{
    if (m_eState == EResourceState::Running)
        Stop(true);

    TidyUp();

    if (m_pNodeStorage)
    {
        delete m_pNodeStorage;
        m_pNodeStorage = nullptr;
    }

    if (m_pNodeSettings)
    {
        delete m_pNodeSettings;
        m_pNodeSettings = nullptr;
    }

    m_strResourceCachePath = "";
    m_strResourceDirectoryPath = "";
    m_eState = EResourceState::None;
    return true;
}

void CResource::Reload()
{
    Unload();
    Load();
}

CResource::~CResource()
{
    CIdArray::PushUniqueId(this, EIdClass::RESOURCE, m_uiScriptID);

    m_bDestroyed = true;

    Unload();

    // Overkill, but easiest way to stop crashes:
    // Go through all other resources and make sure we are not in m_IncludedResources, m_Dependents and m_TemporaryIncludes
    auto iter = m_pResourceManager->IterBegin();

    for (; iter != m_pResourceManager->IterEnd(); ++iter)
    {
        if (*iter != this)
            (*iter)->InvalidateIncludedResourceReference(this);
    }

    m_strResourceName = "";
}

void CResource::TidyUp()
{
    // Go through each resource file and delete it
    for (CResourceFile* pResourceFile : m_ResourceFiles)
        delete pResourceFile;

    m_ResourceFiles.clear();

    // Go through each included resource item and delete it
    for (CIncludedResources* pIncludedResources : m_IncludedResources)
        delete pIncludedResources;

    m_IncludedResources.clear();

    // Go through each of the dependent resources (those that include this one) and remove the reference to this
    for (CResource* pDependent : m_Dependents)
        pDependent->InvalidateIncludedResourceReference(this);

    m_httpRouter.reset();

    std::exchange(m_serverFiles, {});
    std::exchange(m_clientFiles, {});
}

bool CResource::GetInfoValue(const char* szKey, std::string& strValue) const
{
    auto iter = m_Info.find(std::string(szKey));

    if (iter == m_Info.end())
        return false;

    strValue = iter->second;
    return true;
}

void CResource::SetInfoValue(const char* szKey, const char* szValue, bool bSave)
{
    bool bFoundExisting = false;

    // Try to find an existing value with a matching key
    auto iter = m_Info.find(std::string(szKey));

    if (iter != m_Info.end())
    {
        bFoundExisting = true;

        if (!szValue)
            m_Info.erase(iter);
        else
            iter->second = szValue;
    }

    // If there was no match and we were going to delete the key, we are done
    if (!bFoundExisting && !szValue)
        return;

    // If we were going to set a new value, create a new key and add it to our list
    if (!bFoundExisting)
    {
        MapSet(m_Info, std::string(szKey), std::string(szValue));
    }

    if (!bSave)
        return;

    std::unique_ptr<CXMLFile> document{g_pServerInterface->GetXML()->CreateXML(m_metaFilePath.string().c_str())};

    if (document == nullptr || !document->Parse())
        return;

    CXMLNode* root = document->GetRootNode();

    if (root == nullptr)
        return;

    // Get info attributes
    CXMLNode* info = root->FindSubNode("info");

    if (info == nullptr)
        info = root->CreateSubNode("info");

    if (!szValue)
    {
        // Delete existing
        info->GetAttributes().Delete(szKey);
    }
    else
    {
        // Update/add
        CXMLAttribute* pAttr = info->GetAttributes().Find(szKey);

        if (pAttr)
            pAttr->SetValue(szValue);
        else
            info->GetAttributes().Create(szKey)->SetValue(szValue);
    }

    document->Write();
}

std::future<SString> CResource::GenerateChecksumForFile(CResourceFile* pResourceFile)
{
    return SharedUtil::async([pResourceFile, this] {
        SString strPath;

        // TODO: Get rid of GetFilePath, we can store the absolute path in CResourceFile
        if (!GetFilePath(pResourceFile->GetName(), strPath))
            return SString();

        std::vector<char> buffer;
        FileLoad(strPath, buffer);
        auto        fileSize = static_cast<unsigned int>(buffer.size()); // NOTE(botder): This is safe, FileLoad has a 1GB limit
        const char* pFileContents = fileSize ? buffer.data() : "";
        CChecksum   Checksum = CChecksum::GenerateChecksumFromBuffer(pFileContents, fileSize);
        pResourceFile->SetLastChecksum(Checksum);
        pResourceFile->SetLastFileSize(fileSize);

        // Check if file is blocked
        char szHashResult[33];
        CMD5Hasher::ConvertToHex(pResourceFile->GetLastChecksum().md5, szHashResult);
        SString strBlockReason = m_pResourceManager->GetBlockedFileReason(szHashResult);

        if (!strBlockReason.empty())
        {
            return SString("file '%s' is blocked (%s)", pResourceFile->GetName(), *strBlockReason);
        }

        // Copy file to http holding directory
        switch (pResourceFile->GetType())
        {
            case CResourceFile::RESOURCE_FILE_TYPE_CLIENT_SCRIPT:
            case CResourceFile::RESOURCE_FILE_TYPE_CLIENT_CONFIG:
            case CResourceFile::RESOURCE_FILE_TYPE_CLIENT_FILE:
            {
                SString strCachedFilePath = pResourceFile->GetCachedPathFilename();

                if (!g_pRealNetServer->ValidateHttpCacheFileName(strCachedFilePath))
                {
                    FileDelete(strCachedFilePath);
                    return SString("ERROR: Resource '%s' client filename '%s' not allowed\n", GetName().c_str(), *ExtractFilename(strCachedFilePath));
                }

                CChecksum cachedChecksum = CChecksum::GenerateChecksumFromFileUnsafe(strCachedFilePath);

                if (pResourceFile->GetLastChecksum() != cachedChecksum)
                {
                    if (!FileSave(strCachedFilePath, pFileContents, fileSize))
                    {
                        return SString("Could not copy '%s' to '%s'\n", *strPath, *strCachedFilePath);
                    }

                    // If script is 'no client cache', make sure there is no trace of it in the output dir
                    if (pResourceFile->IsNoClientCache())
                        FileDelete(pResourceFile->GetCachedPathFilename(true));
                }

                break;
            }
            default:
                break;
        }

        return SString();
    });
}

bool CResource::GenerateChecksums()
{
    std::vector<std::future<SString>> checksumTasks;
    checksumTasks.reserve(m_ResourceFiles.size());

    for (CResourceFile* pResourceFile : m_ResourceFiles)
    {
        checksumTasks.push_back(GenerateChecksumForFile(pResourceFile));
    }

    bool bOk = true;

    for (auto& task : checksumTasks)
    {
        const auto& result = task.get();
        if (!result.empty())
        {
            m_strFailureReason = result;
            CLogger::LogPrintf(result);
            bOk = false;
        }
    }

    m_metaChecksum = CChecksum::GenerateChecksumFromFileUnsafe(m_metaFilePath.string());
    return bOk;
}

bool CResource::HasResourceChanged()
{
    // TODO: Clean this up after CResourceFile refactor

    if (m_bResourceIsZip)
    {
        // Zip file might have changed
        CChecksum checksum = CChecksum::GenerateChecksumFromFileUnsafe(m_archiveFilePath.string());

        if (checksum != m_zipHash)
            return true;
    }

    std::string strPath;

    for (CResourceFile* pResourceFile : m_ResourceFiles)
    {
        // TODO: Get rid of GetFilePath, we can store the absolute path in CResourceFile
        if (GetFilePath(pResourceFile->GetName(), strPath))
        {
            CChecksum checksum = CChecksum::GenerateChecksumFromFileUnsafe(strPath);

            if (pResourceFile->GetLastChecksum() != checksum)
                return true;

            // Also check if file in http cache has been externally altered
            switch (pResourceFile->GetType())
            {
                case CResourceFile::RESOURCE_FILE_TYPE_CLIENT_SCRIPT:
                case CResourceFile::RESOURCE_FILE_TYPE_CLIENT_CONFIG:
                case CResourceFile::RESOURCE_FILE_TYPE_CLIENT_FILE:
                {
                    string    strCachedFilePath = pResourceFile->GetCachedPathFilename();
                    CChecksum cachedChecksum = CChecksum::GenerateChecksumFromFileUnsafe(strCachedFilePath);

                    if (cachedChecksum != checksum)
                        return true;

                    break;
                }
                default:
                    break;
            }
        }
    }

    CChecksum checksum = CChecksum::GenerateChecksumFromFileUnsafe(m_metaFilePath.string());
    return (checksum != m_metaChecksum);
}

void CResource::ApplyUpgradeModifications()
{
    CResourceChecker().ApplyUpgradeModifications(this, (m_bResourceIsZip ? m_archiveFilePath.string() : ""s));
}

//
// Output deprecated function usage and version issues
//
void CResource::LogUpgradeWarnings()
{
    CResourceChecker().LogUpgradeWarnings(this, m_archiveFilePath.string(), m_strMinClientReqFromSource, m_strMinServerReqFromSource, m_strMinClientReason,
                                          m_strMinServerReason);
    SString strStatus;

    if (!GetCompatibilityStatus(strStatus))
    {
        SString strReason = SString("WARNING: %s will not start as %s\n", m_strResourceName.c_str(), *strStatus);
        CLogger::LogPrint(strReason);
    }
    else if (!strStatus.empty())
    {
        SString strReason = SString("WARNING: %s requires upgrade as %s\n", m_strResourceName.c_str(), *strStatus);
        CLogger::LogPrint(strReason);
        CLogger::LogPrintf("Use the 'upgrade' command to perform a basic upgrade of resources.\n");
    }
}

//
// Determine outcome of version rules
//
// Fatal error: Returns false with error message in strOutStatus
// Warning:     Returns true with warning message in strOutStatus
// Ok:          Returns true with no message in strOutStatus
//
bool CResource::GetCompatibilityStatus(SString& strOutStatus)
{
    // Check declared version strings are valid
    if (!IsValidVersionString(m_strMinServerRequirement) || !IsValidVersionString(m_strMinClientRequirement))
    {
        strOutStatus = "<min_mta_version> section in the meta.xml contains invalid version strings";
        return false;
    }

    // Check this server can run this resource
#if MTASA_VERSION_BUILD != 0
    CMtaVersion strServerVersion = CStaticFunctionDefinitions::GetVersionSortable();
    if (m_strMinServerRequirement > strServerVersion)
    {
        strOutStatus = SString("this server version is too low (%s required)", *m_strMinServerRequirement);
        return false;
    }

    // This should not happen
    if (m_strMinServerReqFromSource > strServerVersion)
    {
        strOutStatus = "server has come back from the future";
        return false;
    }
#endif

    // Check if calculated version is higher than declared version
    if (m_strMinClientReqFromSource > m_strMinClientFromMetaXml)
    {
        strOutStatus = "<min_mta_version> section in the meta.xml is incorrect or missing (expected at least ";
        strOutStatus += SString("client %s because of '%s')", *m_strMinClientReqFromSource, *m_strMinClientReason);
        m_strMinClientRequirement = m_strMinClientReqFromSource;            // Apply higher version requirement
    }
    else if (m_strMinServerReqFromSource > m_strMinServerFromMetaXml)
    {
        strOutStatus = "<min_mta_version> section in the meta.xml is incorrect or missing (expected at least ";
        strOutStatus += SString("server %s because of '%s')", *m_strMinServerReqFromSource, *m_strMinServerReason);
    }

    // See if any connected client are below min requirements
    {
        uint uiNumIncompatiblePlayers = 0;
        for (std::list<CPlayer*>::const_iterator iter = g_pGame->GetPlayerManager()->IterBegin(); iter != g_pGame->GetPlayerManager()->IterEnd(); iter++)
            if ((*iter)->IsJoined() && m_strMinClientRequirement > (*iter)->GetPlayerVersion() && !(*iter)->ShouldIgnoreMinClientVersionChecks())
                uiNumIncompatiblePlayers++;

        if (uiNumIncompatiblePlayers > 0)
        {
            strOutStatus = SString("%d connected player(s) below required client version %s", uiNumIncompatiblePlayers, *m_strMinClientRequirement);
            return false;
        }
    }

    return true;
}

bool CResource::Start(std::list<CResource*>* pDependents, bool bManualStart, const SResourceStartOptions& StartOptions)
{
    if (m_eState == EResourceState::Running)
        return true;

    if (m_eState != EResourceState::Loaded)
        return false;

    if (m_bDestroyed)
        return false;

    m_eState = EResourceState::Starting;

    CLuaArguments PreStartArguments;
    PreStartArguments.PushResource(this);

    if (!g_pGame->GetMapManager()->GetRootElement()->CallEvent("onResourcePreStart", PreStartArguments))
    {
        // Start cancelled by another resource
        m_strFailureReason = "Start cancelled by script\n";
        m_eState = EResourceState::Loaded;
        return false;
    }

    // CheckForIssues
    if (!m_bDoneUpgradeWarnings)
    {
        m_bDoneUpgradeWarnings = true;
        CResourceChecker().LogUpgradeWarnings(this, m_archiveFilePath.string(), m_strMinClientReqFromSource, m_strMinServerReqFromSource, m_strMinClientReason,
                                              m_strMinServerReason);
    }

    // MTA version check
    SString strStatus;

    if (!GetCompatibilityStatus(strStatus))
    {
        m_strFailureReason = SString("Not starting resource %s as %s\n", m_strResourceName.c_str(), strStatus.c_str());
        CLogger::LogPrint(m_strFailureReason);
        m_eState = EResourceState::Loaded;
        return false;
    }
    else if (!strStatus.empty())
    {
        SString strReason = SString("WARNING: %s requires upgrade as %s\n", m_strResourceName.c_str(), *strStatus);
        CLogger::LogPrint(strReason);
        CLogger::LogPrintf("Use the 'upgrade' command to perform a basic upgrade of resources.\n");
    }

    // Check the included resources are linked
    if (!m_bLinked)
    {
        if (!LinkToIncludedResources())
        {
            m_eState = EResourceState::Loaded;
            return false;
        }
    }

    m_bIsPersistent = false;

    // Create an element group for us
    m_pDefaultElementGroup = new CElementGroup();

    // Grab the root element
    m_pRootElement = g_pGame->GetMapManager()->GetRootElement();

    // Create the temporary storage node
    m_pNodeStorage = g_pServerInterface->GetXML()->CreateDummyNode();

    // Create the Resource Element
    m_pResourceElement = new CDummy(g_pGame->GetGroups(), m_pRootElement);
    m_pResourceElement->SetTypeName("resource");

    // Contains elements created at runtime by scripts etc (i.e. not in maps)
    m_pResourceDynamicElementRoot = new CDummy(g_pGame->GetGroups(), m_pResourceElement);
    m_pResourceDynamicElementRoot->SetTypeName("map");
    m_pResourceDynamicElementRoot->SetName("dynamic");

    // Verify resource element id and dynamic element root id
    if (m_pResourceElement->GetID() == INVALID_ELEMENT_ID || m_pResourceDynamicElementRoot->GetID() == INVALID_ELEMENT_ID)
    {
        // Destroy the dynamic element root
        g_pGame->GetElementDeleter()->Delete(m_pResourceDynamicElementRoot);
        m_pResourceDynamicElementRoot = nullptr;

        // Destroy the resource element
        g_pGame->GetElementDeleter()->Delete(m_pResourceElement);
        m_pResourceElement = nullptr;

        // Remove the temporary XML storage node
        if (m_pNodeStorage)
        {
            delete m_pNodeStorage;
            m_pNodeStorage = nullptr;
        }

        m_pRootElement = nullptr;

        // Destroy the element group attached directly to this resource
        delete m_pDefaultElementGroup;
        m_pDefaultElementGroup = nullptr;

        m_eState = EResourceState::Loaded;
        m_strFailureReason = SString("Start up of resource %s cancelled by element id starvation", m_strResourceName.c_str());
        CLogger::LogPrintf("%s\n", m_strFailureReason.c_str());
        return false;
    }

    // Set the Resource Element name
    m_pResourceElement->SetName(m_strResourceName.c_str());

    // Create the virtual machine for this resource
    CreateVM(m_bOOPEnabledInMetaXml);

    // We're now active
    CLogger::LogPrintf(LOGLEVEL_LOW, "Starting %s\n", m_strResourceName.c_str());

    // Remember the time we started
    time(&m_timeStarted);

    if (m_bSyncMapElementDataDefined)
        m_pResourceManager->ApplySyncMapElementDataOption(this, m_bSyncMapElementData);

    // Start all our sub resourcefiles
    for (CResourceFile* pResourceFile : m_ResourceFiles)
    {
        bool bAbortStart = false;

        // Check if file is blocked
        char szHashResult[33];
        CMD5Hasher::ConvertToHex(pResourceFile->GetLastChecksum().md5, szHashResult);
        SString strBlockReason = m_pResourceManager->GetBlockedFileReason(szHashResult);

        if (!strBlockReason.empty())
        {
            m_strFailureReason = SString("File '%s' is blocked (%s)", pResourceFile->GetName(), strBlockReason.c_str());
            CLogger::LogPrintf("Failed to start resource '%s' - %s\n", GetName().c_str(), m_strFailureReason.c_str());
            bAbortStart = true;
        }

        // Start if applicable
        if ((pResourceFile->GetType() == CResourceFile::RESOURCE_FILE_TYPE_MAP && StartOptions.bMaps) ||
            (pResourceFile->GetType() == CResourceFile::RESOURCE_FILE_TYPE_CONFIG && StartOptions.bConfigs) ||
            (pResourceFile->GetType() == CResourceFile::RESOURCE_FILE_TYPE_SCRIPT && StartOptions.bScripts) ||
            (pResourceFile->GetType() == CResourceFile::RESOURCE_FILE_TYPE_CLIENT_SCRIPT && StartOptions.bClientScripts) ||
            (pResourceFile->GetType() == CResourceFile::RESOURCE_FILE_TYPE_HTML && StartOptions.bHTML))
        {
            // Start. Failed?
            if (!pResourceFile->Start())
            {
                // Log it
                m_strFailureReason = SString("Failed to start resource item %s which is required\n", pResourceFile->GetName());
                CLogger::LogPrintf("Failed to start resource item %s in %s\n", pResourceFile->GetName(), m_strResourceName.c_str());
                bAbortStart = true;
            }
        }

        // Handle abort
        if (bAbortStart)
        {
            // Stop all the resource items without any warnings
            for (CResourceFile* pResourceFile : m_ResourceFiles)
                pResourceFile->Stop();

            DestroyVM();

            // Remove the temporary XML storage node
            if (m_pNodeStorage)
            {
                delete m_pNodeStorage;
                m_pNodeStorage = nullptr;
            }

            // Destroy the element group attached directly to this resource
            if (m_pDefaultElementGroup)
                delete m_pDefaultElementGroup;

            m_pDefaultElementGroup = nullptr;

            // Make sure we remove the resource elements from the players that have joined
            CEntityRemovePacket removePacket;

            if (m_pResourceElement)
            {
                removePacket.Add(m_pResourceElement);
                g_pGame->GetElementDeleter()->Delete(m_pResourceElement);
                m_pResourceElement = nullptr;
            }

            if (m_pResourceDynamicElementRoot)
            {
                removePacket.Add(m_pResourceDynamicElementRoot);
                g_pGame->GetElementDeleter()->Delete(m_pResourceDynamicElementRoot);
                m_pResourceDynamicElementRoot = nullptr;
            }

            g_pGame->GetPlayerManager()->BroadcastOnlyJoined(removePacket);
            m_eState = EResourceState::Loaded;
            return false;
        }
    }

    if (StartOptions.bIncludedResources)
    {
        // Copy the list over included resources because reloading them might change the list
        std::list<CIncludedResources*> includedResources = m_IncludedResources;

        // Start our included resources that haven't been started
        for (CIncludedResources* pIncludedResources : includedResources)
        {
            // Has it already been loaded?
            CResource* pIncluded = pIncludedResources->GetResource();

            if (pIncluded)
            {
                // Included resource has changed?
                if (pIncluded->HasResourceChanged())
                {
                    // Reload it if it's not already started
                    if (!pIncluded->IsActive())
                    {
                        m_pResourceManager->Reload(pIncluded);
                    }
                    else
                    {
                        CLogger::LogPrintf("WARNING: Included resource %s has changed but unable to reload due to resource already being in use\n",
                                           pIncluded->GetName().c_str());
                    }
                }

                // Make us dependant of it
                pIncluded->AddDependent(this);
            }
        }
    }

    // Add the resources depending on us
    if (pDependents)
    {
        for (CResource* pDependent : *pDependents)
            AddDependent(pDependent);
    }

    m_eState = EResourceState::Running;

    // Call the onResourceStart event. If it returns false, cancel this script again
    CLuaArguments Arguments;
    Arguments.PushResource(this);

    if (!m_pResourceElement->CallEvent("onResourceStart", Arguments))
    {
        // We're no longer active. stop the resource
        CLogger::LogPrintf("Start up of resource %s cancelled by script\n", m_strResourceName.c_str());
        m_strFailureReason = "Start up of resource cancelled by script\n";
        Stop(true);
        return false;
    }

    m_bStartedManually = bManualStart;

    // Remember the client files state
    m_bClientConfigs = StartOptions.bClientConfigs;
    m_bClientScripts = StartOptions.bClientScripts;
    m_bClientFiles = StartOptions.bClientFiles;

    m_pResourceManager->ApplyMinClientRequirement(this, m_strMinClientRequirement);

    // Broadcast new resourceelement that is loaded and tell the players that a new resource was started
    g_pGame->GetMapManager()->BroadcastResourceElements(m_pResourceElement, m_pDefaultElementGroup);
    g_pGame->GetPlayerManager()->BroadcastOnlyJoined(CResourceStartPacket(m_strResourceName.c_str(), this));
    SendNoClientCacheScripts();
    m_bClientSync = true;

    // HACK?: stops resources getting loaded twice when you change them then manually restart
    GenerateChecksums();

    m_pResourceManager->OnResourceStart(this);
    return true;
}

bool CResource::Stop(bool bManualStop)
{
    if (m_eState == EResourceState::Loaded)
        return true;

    if (m_eState != EResourceState::Running)
        return false;

    if (m_bStartedManually && !bManualStop)
        return false;

    m_pResourceManager->OnResourceStop(this);

    m_eState = EResourceState::Stopping;
    m_pResourceManager->RemoveMinClientRequirement(this);
    m_pResourceManager->RemoveSyncMapElementDataOption(this);

    // Tell the log that we've stopped this resource
    CLogger::LogPrintf(LOGLEVEL_LOW, "Stopping %s\n", m_strResourceName.c_str());

    // Tell the modules we are stopping
    g_pGame->GetLuaManager()->GetLuaModuleManager()->ResourceStopping(m_pVM->GetVirtualMachine());

    // Tell all the players that have joined that this resource is stopped
    g_pGame->GetPlayerManager()->BroadcastOnlyJoined(CResourceStopPacket(m_usNetID));
    m_bClientSync = false;

    // Call the onResourceStop event on this resource element
    CLuaArguments Arguments;
    Arguments.PushResource(this);
    Arguments.PushBoolean(m_bDestroyed);
    m_pResourceElement->CallEvent("onResourceStop", Arguments);

    // Remove us from the resources we depend on (they might unload too first)
    for (CIncludedResources* pIncludedResources : m_IncludedResources)
    {
        CResource* pResource = pIncludedResources->GetResource();

        if (pResource)
            pResource->RemoveDependent(this);
    }

    // Temorary includes??
    for (CResource* pResource : m_TemporaryIncludes)
        pResource->RemoveDependent(this);

    m_TemporaryIncludes.clear();

    // Stop all the resource files we have. The files we share with our clients we remove from the resource file list.
    for (CResourceFile* pResourceFile : m_ResourceFiles)
    {
        if (!pResourceFile->Stop())
            CLogger::LogPrintf("Failed to stop resource item %s in %s\n", pResourceFile->GetName(), m_strResourceName.c_str());
    }

    // Tell the module manager we have stopped
    g_pGame->GetLuaManager()->GetLuaModuleManager()->ResourceStopped(m_pVM->GetVirtualMachine());

    // Remove the temporary XML storage node
    if (m_pNodeStorage)
    {
        delete m_pNodeStorage;
        m_pNodeStorage = nullptr;
    }

    // Destroy the element group attached directly to this resource
    if (m_pDefaultElementGroup)
    {
        delete m_pDefaultElementGroup;
        m_pDefaultElementGroup = nullptr;
    }

    // Destroy the virtual machine for this resource
    DestroyVM();

    // Remove the resource element from the client
    CEntityRemovePacket removePacket;

    if (m_pResourceElement)
    {
        removePacket.Add(m_pResourceElement);
        g_pGame->GetElementDeleter()->Delete(m_pResourceElement);
        m_pResourceElement = nullptr;
    }

    // Remove the dynamic resource element from the client
    if (m_pResourceDynamicElementRoot)
    {
        removePacket.Add(m_pResourceDynamicElementRoot);
        g_pGame->GetElementDeleter()->Delete(m_pResourceDynamicElementRoot);
        m_pResourceDynamicElementRoot = nullptr;
    }

    // Broadcast the packet to joined players
    g_pGame->GetPlayerManager()->BroadcastOnlyJoined(removePacket);

    m_eState = EResourceState::Loaded;
    return true;
}

bool CResource::CreateVM(bool bEnableOOP)
{
    if (!m_pVM)
    {
        m_pVM = g_pGame->GetLuaManager()->CreateVirtualMachine(this, bEnableOOP);
        m_pResourceManager->NotifyResourceVMOpen(this, m_pVM);
    }

    if (!m_pVM)
        return false;

    m_pVM->SetScriptName(m_strResourceName.c_str());
    return true;
}

bool CResource::DestroyVM()
{
    // Remove all player keybinds on this VM
    list<CPlayer*>::const_iterator iter = g_pGame->GetPlayerManager()->IterBegin();

    for (; iter != g_pGame->GetPlayerManager()->IterEnd(); iter++)
    {
        CKeyBinds* pBinds = (*iter)->GetKeyBinds();

        if (pBinds)
            pBinds->RemoveAllKeys(m_pVM);
    }

    // Delete the events on this VM
    m_pRootElement->DeleteEvents(m_pVM, true);
    g_pGame->GetElementDeleter()->CleanUpForVM(m_pVM);

    // Delete the virtual machine
    m_pResourceManager->NotifyResourceVMClose(this, m_pVM);
    g_pGame->GetLuaManager()->RemoveVirtualMachine(m_pVM);
    m_pVM = nullptr;
    return true;
}

bool CResource::HasGoneAway()
{
    if (m_bResourceIsZip)
    {
        return !IsRegularFile(m_archiveFilePath);
    }
    else
    {
        return !IsRegularFile(m_metaFilePath);
    }
}

bool CResource::GetFilePath(const char* szFilename, string& strPath)
{
    // Always prefer the local resource directory, as scripts may
    // have added new files to the regular folder, rather than the zip
    strPath = m_strResourceDirectoryPath + szFilename;
    if (FileExists(strPath))
        return true;

    // If this is a zipped resource, try to use the unzipped file
    if (!IsResourceZip())
        return false;

    strPath = m_strResourceCachePath + szFilename;
    return FileExists(strPath);
}

bool CResource::GetDefaultSetting(const char* szName, char* szValue, size_t sizeBuffer)
{
    // Look through its subnodes for settings with a matching name
    unsigned int uiCount = m_pNodeSettings->GetSubNodeCount();
    unsigned int i = 0;
    std::string  strTagName;

    for (; i < uiCount; i++)
    {
        // Grab its tag name
        CXMLNode* pTemp = m_pNodeSettings->GetSubNode(i);
        strTagName = pTemp->GetTagName();

        // Check that its "setting"
        if (!stricmp(strTagName.c_str(), "setting"))
        {
            // Grab the name attribute and compare it to the name we look for
            CXMLAttribute* pName = m_pNodeSettings->GetAttributes().Find("name");

            if (pName && !strcmp(szName, pName->GetValue().c_str()))
            {
                strncpy(szValue, pName->GetValue().c_str(), std::min(sizeBuffer, pName->GetValue().size()));
                szValue[sizeBuffer - 1] = '\0';
                return true;
            }
        }
    }

    return false;
}

bool CResource::SetDefaultSetting(const char* szName, const char* szValue)
{
    // TODO: Add implementaion here
    return false;
}

bool CResource::RemoveDefaultSetting(const char* szName)
{
    // TODO: Add implementaion here
    return false;
}

bool CResource::AddMapFile(const char* szName, const char* szFullFilename, int iDimension)
{
    if (!IsLoaded() || m_bResourceIsZip)
        return false;

    std::unique_ptr<CXMLFile> document{g_pServerInterface->GetXML()->CreateXML(m_metaFilePath.string().c_str())};

    if (document == nullptr || !document->Parse())
        return false;

    CXMLNode* root = document->GetRootNode();

    if (root == nullptr)
        return false;

    // Create a new map subnode
    CXMLNode* map = root->CreateSubNode("map");

    if (map == nullptr)
        return false;

    // Set the src and dimension attributes
    map->GetAttributes().Create("src")->SetValue(szName);
    map->GetAttributes().Create("dimension")->SetValue(iDimension);

    // If we're loaded, add it to the resourcefiles too
    m_ResourceFiles.push_back(new CResourceMapItem(this, szName, szFullFilename, &map->GetAttributes(), iDimension));

    // Success, write and destroy XML
    document->Write();
    return true;
}

bool CResource::AddConfigFile(const char* szName, const char* szFullFilepath, int iType)
{
    if (!IsLoaded() || m_bResourceIsZip)
        return false;

    std::unique_ptr<CXMLFile> document{g_pServerInterface->GetXML()->CreateXML(m_metaFilePath.string().c_str())};

    if (document == nullptr || !document->Parse())
        return false;

    CXMLNode* root = document->GetRootNode();

    if (root == nullptr)
        return false;

    // Create a new config subnode
    CXMLNode* config = root->CreateSubNode("config");

    if (config == nullptr)
        return false;

    // Set the src attributes
    config->GetAttributes().Create("src")->SetValue(szName);

    // Also set the type attribute (server or client)
    if (iType == CResourceScriptItem::RESOURCE_FILE_TYPE_CLIENT_CONFIG)
        config->GetAttributes().Create("type")->SetValue("client");
    else if (iType == CResourceScriptItem::RESOURCE_FILE_TYPE_CONFIG)
        config->GetAttributes().Create("type")->SetValue("server");

    // If we're loaded, add it to the resourcefiles too
    m_ResourceFiles.push_back(new CResourceConfigItem(this, szName, szFullFilepath, &config->GetAttributes()));

    document->Write();
    return true;
}

bool CResource::IncludedFileExists(const char* szName, int iType)
{
    for (CResourceFile* pResourceFile : m_ResourceFiles)
    {
        // Is it the required type?
        if (iType == CResourceFile::RESOURCE_FILE_TYPE_NONE || pResourceFile->GetType() == iType)
        {
            // Check if the name compares equal (case independant)
            if (!stricmp(pResourceFile->GetName(), szName))
                return true;
        }
    }

    return false;
}

bool CResource::RemoveFile(const char* szName)
{
    if (!IsLoaded() || m_bResourceIsZip)
        return false;

    std::optional<ResourceFilePath> resourceFilePath = ProduceResourceFilePath(fs::path{szName, fs::path::format::generic_format}, false);

    if (!resourceFilePath.has_value())
        return false;

    // TODO: Find the CResourceFile with the file path above

    std::unique_ptr<CXMLFile> document{g_pServerInterface->GetXML()->CreateXML(m_metaFilePath.string().c_str())};

    if (document == nullptr || !document->Parse())
        return false;

    CXMLNode* root = document->GetRootNode();

    if (root == nullptr)
        return false;

    CXMLNode* fileNode = nullptr;

    for (auto nodeIter = root->ChildrenBegin(); nodeIter != root->ChildrenEnd(); ++nodeIter)
    {
        CXMLNode*   node = *nodeIter;
        const char* name = node->GetTagName().c_str();

        if (!stricmp("map", name) || !stricmp("config", name) || !stricmp("script", name) || !stricmp("html", name))
        {
            CXMLAttribute* source = node->GetAttributes().Find("src");

            if (source != nullptr && !stricmp(szName, source->GetValue().c_str()))
            {
                fileNode = node;
                break;
            }
        }
    }

    if (fileNode == nullptr)
        return false;

    root->DeleteSubNode(fileNode);

    if (File::Delete(resourceFilePath->absolute.string().c_str()) != 0)
        CLogger::LogPrintf("WARNING: Problems deleting the actual file, but was removed from resource");

    document->Write();
    return true;
}

bool CResource::LinkToIncludedResources()
{
    m_bLinked = true;

    // Loop through the list of included resources
    for (CIncludedResources* pIncludedResources : m_IncludedResources)
    {
        // If we failed creating the link
        if (!pIncludedResources->CreateLink())
        {
            m_bLinked = false;

            if (m_strFailureReason.empty())
                m_strFailureReason = SString("Failed to link to %s", pIncludedResources->GetName().c_str());
        }
    }

    return m_bLinked;
}

bool CResource::CheckIfStartable()
{
    // return straight away if we know we've already got a circular include, otherwise
    // it spams it every few seconds
    if (m_eState == EResourceState::None)
        return false;

    // Check that the included resources aren't circular
    m_strCircularInclude = "";
    std::vector<CResource*> vecCircular;

    if (IsIncludedResourceRecursive(&vecCircular))
    {
        char szOldString[512];
        char szTrail[512] = "";

        // Make a string telling what the circular path is
        for (CResource* pResource : vecCircular)
        {
            if (pResource)
            {
                strcpy(szOldString, szTrail);
                snprintf(szTrail, 510, "-> %s %s", pResource->GetName().c_str(), szOldString);
            }
        }

        // Remember why we failed and return false
        m_strCircularInclude = SString("%s %s", m_strResourceName.c_str(), szTrail);
        m_strFailureReason = SString("Circular include error: %s", m_strCircularInclude.c_str());
        return false;
    }

    // Check if all the included resources are startable
    for (CIncludedResources* pIncludedResources : m_IncludedResources)
    {
        CResource* pResource = pIncludedResources->GetResource();

        if (!pResource || !pResource->CheckIfStartable())
            return false;
    }

    return true;
}

bool CResource::IsIncludedResourceRecursive(std::vector<CResource*>* past)
{
    past->push_back(this);

    // CLogger::LogPrintf ( "%s\n", this->GetName().c_str () );

    // Loop through the included resources
    for (CIncludedResources* pIncludedResources : m_IncludedResources)
    {
        // If one item from the past is equal to one of our dependencies, then there's circular dependencies
        for (CResource* pResource : *past)
        {
            if (pResource == pIncludedResources->GetResource())
                return true;
        }

        // Check further down the tree. If this returns true, we pass that true backwards
        CResource* pResource = pIncludedResources->GetResource();

        if (pResource && pResource->IsIncludedResourceRecursive(past))
            return true;
    }

    // Remove us from the tree again because it wasn't circular this path
    past->pop_back();
    return false;
}

void CResource::AddTemporaryInclude(CResource* pResource)
{
    if (!ListContains(m_TemporaryIncludes, pResource))
        m_TemporaryIncludes.push_back(pResource);
}

void CResource::AddDependent(CResource* pResource)
{
    if (!ListContains(m_Dependents, pResource))
    {
        m_Dependents.push_back(pResource);
        CheckState();
    }
}

void CResource::RemoveDependent(CResource* pResource)
{
    m_Dependents.remove(pResource);
    CheckState();
}

bool CResource::IsDuplicateServerFile(const fs::path& relativeFilePath)
{
    return std::find(m_serverFiles.begin(), m_serverFiles.end(), relativeFilePath) != m_serverFiles.end();
}

bool CResource::IsDuplicateClientFile(const fs::path& relativeFilePath)
{
    std::string lowercasePath = relativeFilePath.generic_string();
    std::transform(lowercasePath.begin(), lowercasePath.end(), lowercasePath.begin(), [](unsigned char c) { return tolower(c); });

    for (const auto& [path, string] : m_clientFiles)
    {
        if (lowercasePath == string || relativeFilePath == path)
            return true;
    }

    return false;
}

void CResource::AddServerFilePath(const ResourceFilePath& resourceFilePath)
{
    m_serverFiles.push_back(resourceFilePath.relative);
}

void CResource::AddClientFilePath(const ResourceFilePath& resourceFilePath)
{
    std::string lowercasePath = resourceFilePath.relative.generic_string();
    std::transform(lowercasePath.begin(), lowercasePath.end(), lowercasePath.begin(), [](unsigned char c) { return tolower(c); });
    m_clientFiles.push_back(std::make_pair(resourceFilePath.relative, std::move(lowercasePath)));
}

bool CResource::ProcessMeta(const mtasa::MetaFileParser& meta)
{
    m_strMinServerFromMetaXml = meta.minServerVersion;
    m_strMinServerRequirement = meta.minServerVersion;

    m_strMinClientFromMetaXml = meta.minClientVersion;
    m_strMinClientRequirement = meta.minClientVersion;

    m_bOOPEnabledInMetaXml = meta.useOOP;

    m_iDownloadPriorityGroup = meta.downloadPriorityGroup;

    m_pNodeSettings = meta.settingsNode;

    if (meta.syncMapElementDataDefined)
    {
        m_bSyncMapElementData = meta.syncMapElementData;
        m_bSyncMapElementDataDefined = true;
    }
    else
    {
        m_bSyncMapElementData = true;
        m_bSyncMapElementDataDefined = false;
    }

    // TODO:
    /*
    // Find the acl requets
    CXMLNode* pNodeAclRequest = pRoot->FindSubNode("aclrequest", 0);

    if (pNodeAclRequest)
        RefreshAutoPermissions(pNodeAclRequest);
    else
        RemoveAutoPermissions();
    */

    return ProcessMetaInfo(meta) && ProcessMetaIncludes(meta) && ProcessMetaMaps(meta) && ProcessMetaFiles(meta) && ProcessMetaScripts(meta) &&
           ProcessMetaHtmls(meta) && ProcessMetaExports(meta) && ProcessMetaConfigs(meta);
}

bool CResource::ProcessMetaInfo(const mtasa::MetaFileParser& meta)
{
    m_Info.clear();

    for (const auto& [key, value] : meta.info)
    {
        m_Info[key] = value;
    }

    return true;
}

bool CResource::ProcessMetaIncludes(const mtasa::MetaFileParser& meta)
{
    for (const mtasa::MetaDependencyItem& item : meta.dependencies)
    {
        SVersion minVersion{item.minVersion.major, item.minVersion.minor, item.minVersion.revision};
        SVersion maxVersion{item.maxVersion.major, item.maxVersion.minor, item.maxVersion.revision};
        m_IncludedResources.push_back(new CIncludedResources{m_pResourceManager, item.resourceName, minVersion, maxVersion, this});
    }

    return true;
}

bool CResource::ProcessMetaMaps(const mtasa::MetaFileParser& meta)
{
    for (const mtasa::MetaFileItem& item : meta.maps)
    {
        std::optional<ResourceFilePath> resourceFilePath = ProduceResourceFilePath(item.sourceFile, false);

        if (!resourceFilePath.has_value() || !IsRegularFile(resourceFilePath->absolute))
        {
            std::string filePath = item.sourceFile.string();

            m_strFailureReason =
                SString("Couldn't find map %.*s for resource %.*s\n", filePath.size(), filePath.data(), m_strResourceName.size(), m_strResourceName.c_str());

            CLogger::ErrorPrintf(m_strFailureReason);
            return false;
        }

        std::string filePath = resourceFilePath->relative.string();

        if (IsDuplicateServerFile(resourceFilePath->relative))
        {
            CLogger::LogPrintf("WARNING: Duplicate map file in resource '%.*s': '%.*s'\n", m_strResourceName.size(), m_strResourceName.c_str(), filePath.size(),
                               filePath.data());
        }

        m_ResourceFiles.push_back(new CResourceMapItem{this, filePath.c_str(), resourceFilePath->absolute.string().c_str(), nullptr, item.dimension});

        AddServerFilePath(resourceFilePath.value());
    }

    return true;
}

bool CResource::ProcessMetaFiles(const mtasa::MetaFileParser& meta)
{
    for (const mtasa::MetaFileItem& item : meta.files)
    {
        std::optional<ResourceFilePath> resourceFilePath = ProduceResourceFilePath(item.sourceFile, true);

        if (!resourceFilePath.has_value() || !IsRegularFile(resourceFilePath->absolute))
        {
            std::string filePath = item.sourceFile.string();

            m_strFailureReason =
                SString("Couldn't find file %.*s for resource %.*s\n", filePath.size(), filePath.data(), m_strResourceName.size(), m_strResourceName.c_str());

            CLogger::ErrorPrintf(m_strFailureReason);
            return false;
        }

        std::string filePath = resourceFilePath->relative.string();

        if (!resourceFilePath->isWindowsCompatible)
        {
            m_strFailureReason = SString("Client file path %.*s for resource %.*s is not supported on Windows\n", filePath.size(), filePath.data(),
                                         m_strResourceName.size(), m_strResourceName.c_str());
            CLogger::ErrorPrintf(m_strFailureReason);
            return false;
        }

        if (IsDuplicateClientFile(resourceFilePath->relative))
        {
            CLogger::LogPrintf("WARNING: Ignoring duplicate client file in resource '%.*s': '%.*s'\n", m_strResourceName.size(), m_strResourceName.c_str(),
                               filePath.size(), filePath.data());
            continue;
        }

        m_ResourceFiles.push_back(
            new CResourceClientFileItem{this, filePath.c_str(), resourceFilePath->absolute.string().c_str(), nullptr, !item.isClientOptional});

        AddClientFilePath(resourceFilePath.value());
    }

    return true;
}

bool CResource::ProcessMetaScripts(const mtasa::MetaFileParser& meta)
{
    for (const mtasa::MetaFileItem& item : meta.scripts)
    {
        std::optional<ResourceFilePath> resourceFilePath = ProduceResourceFilePath(item.sourceFile, item.isForClient);

        if (!resourceFilePath.has_value() || !IsRegularFile(resourceFilePath->absolute))
        {
            std::string filePath = item.sourceFile.string();

            m_strFailureReason =
                SString("Couldn't find script %.*s for resource %.*s\n", filePath.size(), filePath.data(), m_strResourceName.size(), m_strResourceName.c_str());

            CLogger::ErrorPrintf(m_strFailureReason);
            return false;
        }

        std::string filePath = resourceFilePath->relative.string();

        if (item.isForClient && !resourceFilePath->isWindowsCompatible)
        {
            m_strFailureReason = SString("Client script path %.*s for resource %.*s is not supported on Windows\n", filePath.size(), filePath.data(),
                                         m_strResourceName.size(), m_strResourceName.c_str());
            CLogger::ErrorPrintf(m_strFailureReason);
            return false;
        }

        if (item.isForServer && IsDuplicateServerFile(resourceFilePath->relative))
        {
            CLogger::LogPrintf("WARNING: Duplicate script file in resource '%.*s': '%.*s'\n", m_strResourceName.size(), m_strResourceName.c_str(),
                               filePath.size(), filePath.data());
        }

        bool createForClient = item.isForClient;

        if (item.isForClient && IsDuplicateClientFile(resourceFilePath->relative))
        {
            createForClient = false;

            CLogger::LogPrintf("WARNING: Ignoring duplicate client script file in resource '%.*s': '%.*s'\n", m_strResourceName.size(),
                               m_strResourceName.c_str(), filePath.size(), filePath.data());
        }

        if (item.isForServer)
        {
            m_ResourceFiles.push_back(new CResourceScriptItem{this, filePath.c_str(), resourceFilePath->absolute.string().c_str(), nullptr});

            AddServerFilePath(resourceFilePath.value());
        }

        if (createForClient)
        {
            auto resourceFile = new CResourceClientScriptItem{this, filePath.c_str(), resourceFilePath->absolute.string().c_str(), nullptr};
            resourceFile->SetNoClientCache(!item.isClientCacheable);
            m_ResourceFiles.push_back(resourceFile);

            AddClientFilePath(resourceFilePath.value());
        }
    }

    return true;
}

bool CResource::ProcessMetaHtmls(const mtasa::MetaFileParser& meta)
{
    CResourceHTMLItem* firstHtmlFile = nullptr;
    bool               hasDefaultHtmlPage = false;

    for (const mtasa::MetaFileItem& item : meta.htmls)
    {
        std::optional<ResourceFilePath> resourceFilePath = ProduceResourceFilePath(item.sourceFile, false);

        if (!resourceFilePath.has_value() || !IsRegularFile(resourceFilePath->absolute))
        {
            std::string filePath = item.sourceFile.string();

            m_strFailureReason =
                SString("Couldn't find html %.*s for resource %.*s\n", filePath.size(), filePath.data(), m_strResourceName.size(), m_strResourceName.c_str());

            CLogger::ErrorPrintf(m_strFailureReason);
            return false;
        }

        std::string filePath = resourceFilePath->relative.string();

        if (IsDuplicateServerFile(resourceFilePath->relative))
        {
            CLogger::LogPrintf("WARNING: Duplicate html file in resource '%.*s': '%.*s'\n", m_strResourceName.size(), m_strResourceName.c_str(),
                               filePath.size(), filePath.data());
        }

        bool isDefault = item.isHttpDefault;

        if (isDefault)
        {
            if (hasDefaultHtmlPage)
            {
                isDefault = false;

                CLogger::LogPrintf("Only one html item can be default per resource, ignoring %.*s in %.*s\n", filePath.size(), filePath.data(),
                                   m_strResourceName.size(), m_strResourceName.c_str());
            }
            else
            {
                hasDefaultHtmlPage = true;
            }
        }

        auto resourceFile = new CResourceHTMLItem{this,
                                                  filePath.c_str(),
                                                  resourceFilePath->absolute.string().c_str(),
                                                  nullptr,
                                                  isDefault,
                                                  !!item.isHttpRaw,
                                                  !!item.isHttpRestricted,
                                                  m_bOOPEnabledInMetaXml};

        m_ResourceFiles.push_back(resourceFile);

        if (firstHtmlFile == nullptr)
            firstHtmlFile = resourceFile;

        AddServerFilePath(resourceFilePath.value());
    }

    if (firstHtmlFile != nullptr && !hasDefaultHtmlPage)
        firstHtmlFile->SetDefaultPage(true);

    return true;
}

bool CResource::ProcessMetaExports(const mtasa::MetaFileParser& meta)
{
    for (const mtasa::MetaExportItem& item : meta.exports)
    {
        if (item.isForServer)
        {
            m_ExportedFunctions.push_back(
                CExportedFunction{item.functionName, !!item.isHttpAccessible, CExportedFunction::EXPORTED_FUNCTION_TYPE_SERVER, !!item.isACLRestricted});
        }

        if (item.isForClient)
        {
            m_ExportedFunctions.push_back(
                CExportedFunction{item.functionName, !!item.isHttpAccessible, CExportedFunction::EXPORTED_FUNCTION_TYPE_CLIENT, !!item.isACLRestricted});
        }
    }

    return true;
}

bool CResource::ProcessMetaConfigs(const mtasa::MetaFileParser& meta)
{
    for (const mtasa::MetaFileItem& item : meta.configs)
    {
        std::optional<ResourceFilePath> resourceFilePath = ProduceResourceFilePath(item.sourceFile, item.isForClient);

        if (!resourceFilePath.has_value() || !IsRegularFile(resourceFilePath->absolute))
        {
            std::string filePath = item.sourceFile.string();

            m_strFailureReason =
                SString("Couldn't find config %.*s for resource %.*s\n", filePath.size(), filePath.data(), m_strResourceName.size(), m_strResourceName.c_str());

            CLogger::ErrorPrintf(m_strFailureReason);
            return false;
        }

        std::string filePath = resourceFilePath->relative.string();

        if (item.isForClient && !resourceFilePath->isWindowsCompatible)
        {
            m_strFailureReason = SString("Client config path %.*s for resource %.*s is not supported on Windows\n", filePath.size(), filePath.data(),
                                         m_strResourceName.size(), m_strResourceName.c_str());
            CLogger::ErrorPrintf(m_strFailureReason);
            return false;
        }

        if (item.isForServer && IsDuplicateServerFile(resourceFilePath->relative))
        {
            CLogger::LogPrintf("WARNING: Duplicate config file in resource '%.*s': '%.*s'\n", m_strResourceName.size(), m_strResourceName.c_str(),
                               filePath.size(), filePath.data());
        }

        bool createForClient = item.isForClient;

        if (item.isForClient && IsDuplicateClientFile(resourceFilePath->relative))
        {
            createForClient = false;

            CLogger::LogPrintf("WARNING: Ignoring duplicate client config file in resource '%.*s': '%.*s'\n", m_strResourceName.size(),
                               m_strResourceName.c_str(), filePath.size(), filePath.data());
        }

        if (item.isForServer)
        {
            m_ResourceFiles.push_back(new CResourceConfigItem{this, filePath.c_str(), resourceFilePath->absolute.string().c_str(), nullptr});

            AddServerFilePath(resourceFilePath.value());
        }

        if (createForClient)
        {
            m_ResourceFiles.push_back(new CResourceClientConfigItem{this, filePath.c_str(), resourceFilePath->absolute.string().c_str(), nullptr});

            AddClientFilePath(resourceFilePath.value());
        }
    }

    return true;
}

static std::unordered_set<std::string> reservedWindowsFileNames = {"CON"s,  "PRN"s,  "AUX"s,  "NUL"s,  "COM1"s, "COM2"s, "COM3"s, "COM4"s,
                                                                   "COM5"s, "COM6"s, "COM7"s, "COM8"s, "COM9"s, "LPT1"s, "LPT2"s, "LPT3"s,
                                                                   "LPT4"s, "LPT5"s, "LPT6"s, "LPT7"s, "LPT8"s, "LPT9"s};

std::optional<CResource::ResourceFilePath> CResource::ProduceResourceFilePath(const fs::path& relativePath, bool windowsPlatformCheck)
{
    if (relativePath.is_absolute())
        return std::nullopt;

    // NOTE(botder): The path provided to `std::filesystem::canonical` must exist
    std::error_code errorCode;
    fs::path        absoluteFilePath = fs::canonical(m_staticRootDirectory / relativePath, errorCode);

    if (errorCode)
        return std::nullopt;

    // Check if the resulting absolute file path is inside our resource directory
    auto [rootIter, absoluteIter] = std::mismatch(m_staticRootDirectory.begin(), m_staticRootDirectory.end(), absoluteFilePath.begin(), absoluteFilePath.end());

    if (rootIter == m_staticRootDirectory.end() || ++rootIter != m_staticRootDirectory.end())
        return std::nullopt;

    fs::path relativeFilePath = fs::relative(absoluteFilePath, m_staticRootDirectory, errorCode);

    if (errorCode)
        return std::nullopt;

    // Check if the relative file path is allowed on a Windows operating system
    // See https://docs.microsoft.com/en-us/windows/win32/fileio/naming-a-file
    bool isWindowsCompatible = true;

    if (windowsPlatformCheck)
    {
        std::string fileName = relativeFilePath.filename().string();

        if (fileName.back() == '.' || fileName.back() == ' ')
        {
            isWindowsCompatible = false;
        }
        else if (reservedWindowsFileNames.find(fileName) != reservedWindowsFileNames.end())
        {
            isWindowsCompatible = false;
        }
        else
        {
            for (unsigned char c : fileName)
            {
                if (c < 32 || c == '<' || c == '>' || c == ':' || c == '"' || c == '/' || c == '\\' || c == '|' || c == '?' || c == '*')
                {
                    isWindowsCompatible = false;
                    break;
                }
            }
        }
    }

    return ResourceFilePath{absoluteFilePath, relativeFilePath, isWindowsCompatible};
}

bool CResource::CallExportedFunction(const char* szFunctionName, CLuaArguments& Arguments, CLuaArguments& Returns, CResource& Caller)
{
    if (m_eState != EResourceState::Running)
        return false;

    for (CExportedFunction& Exported : m_ExportedFunctions)
    {
        // Verify that the exported function is marked as "Server" (since both Client and Server exported functions exist here)
        if (Exported.GetType() == CExportedFunction::EXPORTED_FUNCTION_TYPE_SERVER)
        {
            bool bRestricted = Exported.IsRestricted();

            if (!strcmp(Exported.GetFunctionName().c_str(), szFunctionName))
            {
                char szFunctionRightName[512];
                snprintf(szFunctionRightName, 512, "%s.function.%s", m_strResourceName.c_str(), szFunctionName);

                CAccessControlListManager* pACLManager = g_pGame->GetACLManager();

                if (pACLManager->CanObjectUseRight(Caller.GetName().c_str(), CAccessControlListGroupObject::OBJECT_TYPE_RESOURCE, m_strResourceName.c_str(),
                                                   CAccessControlListRight::RIGHT_TYPE_RESOURCE, !bRestricted) &&
                    pACLManager->CanObjectUseRight(Caller.GetName().c_str(), CAccessControlListGroupObject::OBJECT_TYPE_RESOURCE, szFunctionRightName,
                                                   CAccessControlListRight::RIGHT_TYPE_RESOURCE, !bRestricted))
                {
                    if (Arguments.CallGlobal(m_pVM, szFunctionName, &Returns))
                    {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool CResource::CheckState()
{
    if (m_Dependents.empty() && !m_bIsPersistent)
    {
        Stop(false);
        return false;
    }
    else
    {
        return Start();
    }
}

void CResource::OnPlayerJoin(CPlayer& Player)
{
    Player.Send(CResourceStartPacket(m_strResourceName.c_str(), this));
    SendNoClientCacheScripts(&Player);
}

void CResource::SendNoClientCacheScripts(CPlayer* pPlayer)
{
    if (!IsClientScriptsOn())
        return;

    std::vector<CPlayer*> vecPlayers;

    // Send it to either a single player or all the players in the server.
    if (pPlayer)
        vecPlayers.push_back(pPlayer);
    else
    {
        std::list<CPlayer*>::const_iterator iter = g_pGame->GetPlayerManager()->IterBegin();

        for (; iter != g_pGame->GetPlayerManager()->IterEnd(); ++iter)
        {
            vecPlayers.push_back(*iter);
        }
    }

    if (!vecPlayers.empty())
    {
        // Decide what scripts to send
        CResourceClientScriptsPacket Packet(this);
        bool                         bEmptyPacket = true;

        for (CResourceFile* pResourceFile : m_ResourceFiles)
        {
            if (pResourceFile->GetType() == CResourceFile::RESOURCE_FILE_TYPE_CLIENT_SCRIPT)
            {
                CResourceClientScriptItem* pClientScript = static_cast<CResourceClientScriptItem*>(pResourceFile);

                if (pClientScript->IsNoClientCache() == true)
                {
                    Packet.AddItem(pClientScript);
                    bEmptyPacket = false;
                }
            }
        }

        // Send them!
        if (!bEmptyPacket)
        {
            for (std::vector<CPlayer*>::iterator iter = vecPlayers.begin(); iter != vecPlayers.end(); ++iter)
            {
                (*iter)->Send(Packet);
            }
        }
    }
}

bool CResource::UnzipResource()
{
    std::string archiveFilePath = m_archiveFilePath.string();

#ifdef WIN32
    zlib_filefunc_def ffunc;
    fill_win32_filefunc(&ffunc);
    unzFile zipHandle = unzOpen2(archiveFilePath.c_str(), &ffunc);
#else
    unzFile zipHandle = unzOpen(archiveFilePath.c_str());
#endif

    if (zipHandle == nullptr)
    {
        m_strFailureReason = SString("Couldn't open archive file for resource '%.*s'", m_strResourceName.size(), m_strResourceName.c_str());
        CLogger::ErrorPrintf(m_strFailureReason);
        return false;
    }

    std::unique_ptr<unzFile, decltype(&unzClose)> zipCloser{reinterpret_cast<unzFile*>(zipHandle), &unzClose};

    if (unzGoToFirstFile(zipHandle) != UNZ_OK)
    {
        m_strFailureReason =
            SString("Failed to process archive file for resource '%.*s': first file not found", m_strResourceName.size(), m_strResourceName.c_str());
        CLogger::ErrorPrintf(m_strFailureReason);
        return false;
    }

    constexpr uLong fileNameBufferSize = 65535;
    auto            fileNameBuffer = std::make_unique<char[]>(fileNameBufferSize);

    constexpr unsigned int outputBufferSize = 8192;
    auto                   outputBuffer = std::make_unique<char[]>(outputBufferSize);

    std::error_code errorCode;

    do
    {
        unz_file_info fileInfo;
        memset(&fileInfo, 0, sizeof(fileInfo));

        if (unzGetCurrentFileInfo(zipHandle, &fileInfo, fileNameBuffer.get(), fileNameBufferSize, nullptr, 0, nullptr, 0) != UNZ_OK)
            return false;

        std::string_view fileName{fileNameBuffer.get(), fileInfo.size_filename};

        if (fileName.back() == '/')
            continue;

        fs::path outputFilePath = m_staticRootDirectory / fileName;

        struct tm zipFileWriteTime;
        unzConvertToEpochTime(fileInfo.tmu_date, zipFileWriteTime);

        if (IsRegularFile(outputFilePath))
        {
            // Compare file write time (in epoch) to avoid heavy CRC checksum calculation
            struct tm fileWriteTime;

            if (GetFileLastWriteTime(outputFilePath.c_str(), fileWriteTime))
            {
                time_t zipEpoch = mktime(&zipFileWriteTime);
                time_t outputEpoch = mktime(&fileWriteTime);

                if (zipEpoch != -1 && outputEpoch != -1 && difftime(zipEpoch, outputEpoch) == 0)
                    continue;
            }

            unsigned long crc = CRCGenerator::GetCRCFromFile(outputFilePath.string().c_str());

            if (crc == fileInfo.crc)
            {
                SetFileLastWriteTime(outputFilePath.c_str(), zipFileWriteTime);
                continue;
            }
        }

        fs::path outputDirectory = outputFilePath.parent_path();

        if (fs::create_directories(outputDirectory, errorCode); errorCode)
        {
            m_strFailureReason = SString("Processing archive file '%.*s' for resource '%.*s' failed: couldn't create parent directory", fileName.size(),
                                         fileName.data(), m_strResourceName.size(), m_strResourceName.c_str());
            CLogger::ErrorPrintf(m_strFailureReason);
            return false;
        }

        FILE* outputStream = File::Fopen(outputFilePath.string().c_str(), "wb");

        if (outputStream == nullptr)
        {
            m_strFailureReason = SString("Decompressing archive file '%.*s' for resource '%.*s' failed: couldn't create output file", fileName.size(),
                                         fileName.data(), m_strResourceName.size(), m_strResourceName.c_str());
            CLogger::ErrorPrintf(m_strFailureReason);
            return false;
        }

        std::unique_ptr<FILE, decltype(&fclose)> fileCloser{outputStream, &fclose};

        if (unzOpenCurrentFile(zipHandle) != UNZ_OK)
        {
            m_strFailureReason = SString("Decompressing archive file '%.*s' for resource '%.*s' failed: archive file open error", fileName.size(),
                                         fileName.data(), m_strResourceName.size(), m_strResourceName.c_str());
            CLogger::ErrorPrintf(m_strFailureReason);
            return false;
        }

        std::unique_ptr<unzFile, decltype(&unzCloseCurrentFile)> currentFileCloser{reinterpret_cast<unzFile*>(zipHandle), &unzCloseCurrentFile};

        for (;;)
        {
            int numBytes = unzReadCurrentFile(zipHandle, outputBuffer.get(), outputBufferSize);

            if (numBytes < 0)
            {
                m_strFailureReason = SString("Decompressing archive file '%.*s' for resource '%.*s' failed: archive file read error", fileName.size(),
                                             fileName.data(), m_strResourceName.size(), m_strResourceName.c_str());
                CLogger::ErrorPrintf(m_strFailureReason);
                return false;
            }

            if (numBytes == 0)
                break;

            if (fwrite(outputBuffer.get(), numBytes, 1, outputStream) != 1)
            {
                m_strFailureReason = SString("Decompressing archive file '%.*s' for resource '%.*s' failed: output file write error", fileName.size(),
                                             fileName.data(), m_strResourceName.size(), m_strResourceName.c_str());
                CLogger::ErrorPrintf(m_strFailureReason);
                return false;
            }
        }

        // NOTE(botder): We must flush and close the file before applying the last write time
        fileCloser.reset();

        SetFileLastWriteTime(outputFilePath.c_str(), zipFileWriteTime);
    } while (unzGoToNextFile(zipHandle) == UNZ_OK);

    return true;
}

bool CIncludedResources::CreateLink()            // just a pointer to it
{
    // Grab the resource that we are
    m_pResource = m_pResourceManager->GetResource(m_strResourceName.c_str());

    m_bBadVersion = false;

    // Does it exist?
    if (!m_pResource)
        m_bExists = false;
    else
        m_bExists = true;

    if (m_pResource)
    {
        // Grab the version and check that it's in range
        /*unsigned int uiVersion = m_pResource->GetVersion ();
        if ( uiVersion < m_uiMinimumVersion || uiVersion > m_uiMaximumVersion )
        {
            CLogger::ErrorPrintf ( "Incompatible version of %s for resource %s\n", m_szResourceName, m_pOwner->GetName().c_str () );
            CLogger::ErrorPrintf ( "Version between %u and %u needed, version %u installed\n", m_uiMinimumVersion, m_uiMaximumVersion, uiVersion );
            m_bExists = false;
            m_bBadVersion = true;
        }*/
    }

    return m_bExists;
}

void CResource::InvalidateIncludedResourceReference(CResource* pResource)
{
    for (CIncludedResources* pIncludedResources : m_IncludedResources)
    {
        if (pIncludedResources->GetResource() == pResource)
            pIncludedResources->InvalidateReference();
    }

    m_TemporaryIncludes.remove(pResource);
    assert(this != pResource);
    m_Dependents.remove(pResource);
}

//
// Check ACL 'function right' cache for this resource.
// Will automatically clear the cache if ACL has changed.
//
// Returns true if setting was found in cache
//
bool CResource::CheckFunctionRightCache(lua_CFunction f, bool* pbOutAllowed)
{
    uint uiACLRevision = g_pGame->GetACLManager()->GetGlobalRevision();

    // Check validity of cache
    if (m_uiFunctionRightCacheRevision != uiACLRevision)
    {
        m_FunctionRightCacheMap.clear();
        m_uiFunctionRightCacheRevision = uiACLRevision;
    }

    // Read cache
    bool* pResult = MapFind(m_FunctionRightCacheMap, f);

    if (!pResult)
        return false;

    *pbOutAllowed = *pResult;
    return true;
}

//
// Update ACL 'function right' cache for this resource.
//
void CResource::UpdateFunctionRightCache(lua_CFunction f, bool bAllowed)
{
    MapSet(m_FunctionRightCacheMap, f, bAllowed);
}

//
// Check resource files for probable use of dbConnect + mysql
//
bool CResource::IsUsingDbConnectMysql()
{
    if (!m_bDoneDbConnectMysqlScan)
    {
        m_bDoneDbConnectMysqlScan = true;

        for (CResourceFile* pResourceFile : m_ResourceFiles)
        {
            if (pResourceFile->GetType() == CResourceFile::RESOURCE_FILE_TYPE_SCRIPT)
            {
                SString strLuaSource;
                FileLoad(pResourceFile->GetFullName(), strLuaSource);

                for (size_t curPos = 0; curPos < strLuaSource.length(); curPos++)
                {
                    curPos = strLuaSource.find("dbConnect", curPos);

                    if (curPos == SString::npos)
                        break;

                    size_t foundPos = strLuaSource.find("mysql", curPos);

                    if (foundPos > curPos && foundPos < curPos + 40)
                    {
                        m_bUsingDbConnectMysql = true;
                    }
                }
            }
        }
    }

    return m_bUsingDbConnectMysql;
}

//
// Return true if file access should be denied to other resources
//
bool CResource::IsFileDbConnectMysqlProtected(const SString& strAbsFilename, bool bReadOnly)
{
    if (!IsUsingDbConnectMysql())
        return false;

    SString strFilename = ExtractFilename(strAbsFilename);

    if (strFilename.CompareI("meta.xml"))
    {
        if (!bReadOnly)
        {
            // No write access to meta.xml
            return true;
        }
    }

    for (CResourceFile* pResourceFile : m_ResourceFiles)
    {
        if (pResourceFile->GetType() == CResourceFile::RESOURCE_FILE_TYPE_SCRIPT)
        {
            SString strResourceFilename = ExtractFilename(pResourceFile->GetName());

            if (strFilename.CompareI(strResourceFilename))
            {
                // No read/write access to server script files
                return true;
            }
        }
    }

    return false;
}
