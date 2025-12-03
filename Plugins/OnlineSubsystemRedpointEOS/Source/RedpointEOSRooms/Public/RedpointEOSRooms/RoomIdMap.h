// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Containers/Map.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/RoomId.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(983575976, Redpoint::EOS::Rooms)
{
using namespace ::Redpoint::EOS::Rooms;

template <typename ValueType>
struct TRoomIdMapHashableKeyFuncs : BaseKeyFuncs<TPair<FRoomIdRef, ValueType>, ValueType, false>
{
    typedef typename TTypeTraits<FRoomIdRef>::ConstPointerType KeyInitType;
    typedef const TPairInitializer<
        typename TTypeTraits<FRoomIdRef>::ConstInitType,
        typename TTypeTraits<ValueType>::ConstInitType> &ElementInitType;

    static FORCEINLINE KeyInitType GetSetKey(ElementInitType Element)
    {
        return Element.Key;
    }

    static FORCEINLINE bool Matches(KeyInitType A, KeyInitType B)
    {
        return A->IsEqual(B);
    }

    template <typename ComparableKey> static FORCEINLINE bool Matches(KeyInitType A, ComparableKey B)
    {
        return A->IsEqual(B);
    }

    static FORCEINLINE uint32 GetKeyHash(KeyInitType K)
    {
        return HashCombine(GetTypeHash(K->GetProviderName()), GetTypeHash(K->GetRoomId()));
    }

    template <typename ComparableKey> static FORCEINLINE uint32 GetKeyHash(ComparableKey K)
    {
        return HashCombine(GetTypeHash(K->GetProviderName()), GetTypeHash(K->GetRoomId()));
    }
};

template <typename ValueType>
class TRoomIdMap : public TMap<FRoomIdRef, ValueType, FDefaultSetAllocator, TRoomIdMapHashableKeyFuncs<ValueType>>
{
};

}

namespace Redpoint::EOS::Rooms
{
REDPOINT_EOS_FILE_NS_EXPORT(983575976, Redpoint::EOS::Rooms, TRoomIdMapHashableKeyFuncs)
REDPOINT_EOS_FILE_NS_EXPORT(983575976, Redpoint::EOS::Rooms, TRoomIdMap)
}

REDPOINT_EOS_CODE_GUARD_END()