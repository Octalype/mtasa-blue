/*****************************************************************************
 *
 *  PROJECT:     Multi Theft Auto
 *  LICENSE:     See LICENSE in the top level directory
 *  PURPOSE:     Header for object start sync packet class
 *
 *  Multi Theft Auto is available from https://multitheftauto.com/
 *
 *****************************************************************************/

#pragma once

#include "CPacket.h"

class CObject;

class CObjectStartSyncPacket final : public CPacket
{
public:
    CObjectStartSyncPacket(CObject* pObject) { m_pObject = pObject; };

    ePacketID     GetPacketID() const { return PACKET_ID_OBJECT_STARTSYNC; };
    unsigned long GetFlags() const { return PACKET_HIGH_PRIORITY | PACKET_RELIABLE | PACKET_SEQUENCED; };

    bool Write(NetBitStreamInterface& BitStream) const;

private:
    CObject* m_pObject;
};
