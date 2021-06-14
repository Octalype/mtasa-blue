project "CEF"
	targetname "CEF"
	language "C++"
	kind "StaticLib"

	includedirs { "." }
	-- Check this: https://bitbucket.org/chromiumembedded/cef/src/48908c919591afadac3383effbc5f21a2d40f637/cmake/cef_variables.cmake.in?at=master&fileviewer=file-view-default
	defines { "__STDC_CONSTANT_MACROS", "__STDC_FORMAT_MACROS", "_FILE_OFFSET_BITS=64",
		"_WINDOWS", "UNICODE", "_UNICODE", "WINVER=0x0602", "_WIN32_WINNT=0x602", "NOMINMAX", "WIN32_LEAN_AND_MEAN",  "_HAS_EXCEPTIONS=0",
		"PSAPI_VERSION=1", "WRAPPING_CEF_SHARED" }

	files {
		"premake5.lua",
		"libcef_dll/**.cc",
		"**.h",
	}

	local path = buildpath(".")
	local cef_path = "%{wks.location}/../Vendor/cef3/"
	postbuildcommands {
		"{COPY} \""..cef_path.."Release/*\" \""..path.."mta\"",
		"{COPY} \""..cef_path.."Resources/icudtl.dat\" \""..path.."mta\"",
	-- The .pak line below will now only generate chrome_100_percent.pak and chrome_200_percent.pak (see https://bitbucket.org/chromiumembedded/cef/commits/8424f166ccef)
		"{COPY} \""..cef_path.."Resources/*.pak\" \""..path.."mta/cef\"",
		"{COPY} \""..cef_path.."Resources/locales/*\" \""..path.."mta/cef/locales\""
	}

	filter "architecture:x64"
		flags { "ExcludeFromBuild" }
	filter "system:not windows"
		flags { "ExcludeFromBuild" }
