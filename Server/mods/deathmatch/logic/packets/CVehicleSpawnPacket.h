/*****************************************************************************
 *
 *  PROJECT:     Multi Theft Auto
 *  LICENSE:     See LICENSE in the top level directory
 *  PURPOSE:     Vehicle spawn packet class
 *
 *  Multi Theft Auto is available from https://multitheftauto.com/
 *
 *****************************************************************************/

#pragma once

#include "CPacket.h"
#include <vector>

class CVehicle;

class CVehicleSpawnPacket final : public CPacket
{
public:
    ePacketID     GetPacketID() const { return PACKET_ID_VEHICLE_SPAWN; };
    unsigned long GetFlags() const { return PACKET_HIGH_PRIORITY | PACKET_RELIABLE | PACKET_SEQUENCED; };

    bool Write(NetBitStreamInterface& BitStream) const;

    void Add(CVehicle* pVehicle) { m_List.push_back(pVehicle); };
    void Clear() { m_List.clear(); };

private:
    std::vector<CVehicle*> m_List;
};
