// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Containers/Map.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/RoomInviteId.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(4205839278, Redpoint::EOS::Rooms)
{
using namespace ::Redpoint::EOS::Rooms;

template <typename ValueType>
struct TRoomInviteIdMapHashableKeyFuncs : BaseKeyFuncs<TPair<FRoomInviteId, ValueType>, ValueType, false>
{
    typedef typename TTypeTraits<FRoomInviteId>::ConstPointerType KeyInitType;
    typedef const TPairInitializer<
        typename TTypeTraits<FRoomInviteId>::ConstInitType,
        typename TTypeTraits<ValueType>::ConstInitType> &ElementInitType;

    static FORCEINLINE KeyInitType GetSetKey(ElementInitType Element)
    {
        return Element.Key;
    }

    static FORCEINLINE bool Matches(KeyInitType A, KeyInitType B)
    {
        return A == B;
    }

    template <typename ComparableKey> static FORCEINLINE bool Matches(KeyInitType A, ComparableKey B)
    {
        return A == B;
    }

    static FORCEINLINE uint32 GetKeyHash(KeyInitType K)
    {
        return GetTypeHash(K);
    }

    template <typename ComparableKey> static FORCEINLINE uint32 GetKeyHash(ComparableKey K)
    {
        return GetTypeHash(K);
    }
};

template <typename ValueType>
class TRoomInviteIdMap
    : public TMap<FRoomInviteId, ValueType, FDefaultSetAllocator, TRoomInviteIdMapHashableKeyFuncs<ValueType>>
{
};

}

namespace Redpoint::EOS::Rooms
{
REDPOINT_EOS_FILE_NS_EXPORT(4205839278, Redpoint::EOS::Rooms, TRoomInviteIdMapHashableKeyFuncs)
REDPOINT_EOS_FILE_NS_EXPORT(4205839278, Redpoint::EOS::Rooms, TRoomInviteIdMap)
}

REDPOINT_EOS_CODE_GUARD_END()