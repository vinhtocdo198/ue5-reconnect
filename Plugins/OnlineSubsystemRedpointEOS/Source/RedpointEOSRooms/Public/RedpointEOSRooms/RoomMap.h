// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Containers/Map.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Room.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2568715311, Redpoint::EOS::Rooms)
{
using namespace ::Redpoint::EOS::Rooms;

template <typename ValueType>
struct TRoomMapHashableKeyFuncs : BaseKeyFuncs<TPair<FRoomRef, ValueType>, ValueType, false>
{
    typedef typename TTypeTraits<FRoomRef>::ConstPointerType KeyInitType;
    typedef const TPairInitializer<
        typename TTypeTraits<FRoomRef>::ConstInitType,
        typename TTypeTraits<ValueType>::ConstInitType> &ElementInitType;

    static FORCEINLINE KeyInitType GetSetKey(ElementInitType Element)
    {
        return Element.Key;
    }

    static FORCEINLINE bool Matches(KeyInitType A, KeyInitType B)
    {
        return A->GetBoundForLocalUser() == B->GetBoundForLocalUser() && A->GetRoomId()->IsEqual(B->GetRoomId());
    }

    template <typename ComparableKey> static FORCEINLINE bool Matches(KeyInitType A, ComparableKey B)
    {
        return A->GetBoundForLocalUser() == B->GetBoundForLocalUser() && A->GetRoomId()->IsEqual(B->GetRoomId());
    }

    static FORCEINLINE uint32 GetKeyHash(KeyInitType K)
    {
        return HashCombine(
            K->GetBoundForLocalUser().GetHandle(),
            HashCombine(GetTypeHash(K->GetRoomId()->GetProviderName()), GetTypeHash(K->GetRoomId()->GetRoomId())));
    }

    template <typename ComparableKey> static FORCEINLINE uint32 GetKeyHash(ComparableKey K)
    {
        return HashCombine(
            K->GetBoundForLocalUser().GetHandle(),
            HashCombine(GetTypeHash(K->GetRoomId()->GetProviderName()), GetTypeHash(K->GetRoomId()->GetRoomId())));
    }
};

template <typename ValueType>
class TRoomMap : public TMap<FRoomRef, ValueType, FDefaultSetAllocator, TRoomMapHashableKeyFuncs<ValueType>>
{
};

}

namespace Redpoint::EOS::Rooms
{
REDPOINT_EOS_FILE_NS_EXPORT(2568715311, Redpoint::EOS::Rooms, TRoomMapHashableKeyFuncs)
REDPOINT_EOS_FILE_NS_EXPORT(2568715311, Redpoint::EOS::Rooms, TRoomMap)
}

REDPOINT_EOS_CODE_GUARD_END()