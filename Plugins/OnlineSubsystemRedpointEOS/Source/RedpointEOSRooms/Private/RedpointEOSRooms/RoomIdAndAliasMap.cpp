// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/RoomIdAndAliasMap.h"

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSRooms/Features/AliasRoomFeature.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2675616144, Redpoint::EOS::Rooms)
{
using namespace ::Redpoint::EOS::Rooms;

FRoomRef &FRoomIdAndAliasMap::operator[](const FName &Key)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    return this->RoomsByAlias[Key];
}

FRoomRef &FRoomIdAndAliasMap::operator[](const FRoomIdRef &Key)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    return this->RoomsById[Key];
}

const FRoomRef &FRoomIdAndAliasMap::operator[](const FName &Key) const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    return this->RoomsByAlias[Key];
}

const FRoomRef &FRoomIdAndAliasMap::operator[](const FRoomIdRef &Key) const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    return this->RoomsById[Key];
}

bool FRoomIdAndAliasMap::Contains(const FName &Key) const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    return this->RoomsByAlias.Contains(Key);
}

bool FRoomIdAndAliasMap::Contains(const FRoomIdRef &Key) const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    return this->RoomsById.Contains(Key);
}

void FRoomIdAndAliasMap::Add(const FRoomRef &Value)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    auto RoomId = Value->GetRoomId();
    if (this->RoomsById.Contains(RoomId))
    {
        checkf(false, TEXT("Room with ID '%s' has already been added to map."), *RoomId->GetRoomId());
        return;
    }

    FName RoomAlias;
    auto RoomAliasFeature = Value->GetRoomFeature<Features::IAliasRoomFeature>();
    if (RoomAliasFeature.IsValid())
    {
        RoomAlias = RoomAliasFeature->Alias();
        if (this->RoomsByAlias.Contains(RoomAlias))
        {
            checkf(false, TEXT("Room with alias '%s' has already been added to map."), *RoomAlias.ToString());
            return;
        }
    }

    this->RoomsById.Add(RoomId, Value);
    if (RoomAliasFeature.IsValid())
    {
        this->RoomsByAlias.Add(RoomAlias, Value);
    }
    this->Rooms.Add(Value);
}

void FRoomIdAndAliasMap::Remove(const FName &Key)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    auto Room = this->RoomsByAlias[Key];
    this->RoomsById.Remove(Room->GetRoomId());
    this->Rooms.Remove(Room);
    this->RoomsByAlias.Remove(Key);
}

void FRoomIdAndAliasMap::Remove(const FRoomIdRef &Key)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    auto Room = this->RoomsById[Key];
    auto RoomAliasFeature = Room->GetRoomFeature<Features::IAliasRoomFeature>();
    if (RoomAliasFeature.IsValid())
    {
        this->RoomsByAlias.Remove(RoomAliasFeature->Alias());
    }
    this->Rooms.Remove(Room);
    this->RoomsById.Remove(Key);
}

TArray<FRoomRef>::RangedForConstIteratorType FRoomIdAndAliasMap::begin() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    return this->Rooms.begin();
}

TArray<FRoomRef>::RangedForConstIteratorType FRoomIdAndAliasMap::end() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    return this->Rooms.end();
}

const TArray<FRoomRef> &FRoomIdAndAliasMap::GetRooms() const
{
    return this->Rooms;
}

}

REDPOINT_EOS_CODE_GUARD_END()