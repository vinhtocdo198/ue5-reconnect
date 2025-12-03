// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSRooms/Room.h"
#include "RedpointEOSRooms/RoomIdMap.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2675616144, Redpoint::EOS::Rooms)
{
using namespace ::Redpoint::EOS::Rooms;

class FRoomIdAndAliasMap
{
private:
    TRoomIdMap<FRoomRef> RoomsById;
    TMap<FName, FRoomRef> RoomsByAlias;
    TArray<FRoomRef> Rooms;

public:
    FRoomRef &operator[](const FName &Key);
    FRoomRef &operator[](const FRoomIdRef &Key);
    const FRoomRef &operator[](const FName &Key) const;
    const FRoomRef &operator[](const FRoomIdRef &Key) const;

    bool Contains(const FName &Key) const;
    bool Contains(const FRoomIdRef &Key) const;

    void Add(const FRoomRef &Value);

    void Remove(const FName &Key);
    void Remove(const FRoomIdRef &Key);

    TArray<FRoomRef>::RangedForConstIteratorType begin() const;
    TArray<FRoomRef>::RangedForConstIteratorType end() const;

    const TArray<FRoomRef> &GetRooms() const;
};

}

namespace Redpoint::EOS::Rooms
{
REDPOINT_EOS_FILE_NS_EXPORT(2675616144, Redpoint::EOS::Rooms, FRoomIdAndAliasMap)
}

REDPOINT_EOS_CODE_GUARD_END()