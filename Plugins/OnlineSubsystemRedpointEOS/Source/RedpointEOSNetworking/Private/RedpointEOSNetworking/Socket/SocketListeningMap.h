// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSNetworking/Socket/SocketListeningMapKey.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3442240798, Redpoint::EOS::Networking::Socket)
{
using namespace ::Redpoint::EOS::Networking::Socket;

template <typename ValueType>
struct TSocketListeningMapHashableKeyFuncs : BaseKeyFuncs<TPair<FSocketListeningMapKey, ValueType>, ValueType, false>
{
    typedef typename TTypeTraits<FSocketListeningMapKey>::ConstPointerType KeyInitType;
    typedef const TPairInitializer<
        typename TTypeTraits<FSocketListeningMapKey>::ConstInitType,
        typename TTypeTraits<ValueType>::ConstInitType> &ElementInitType;

    static FORCEINLINE KeyInitType GetSetKey(ElementInitType Element)
    {
        return Element.Key;
    }

    static FORCEINLINE bool Matches(KeyInitType A, KeyInitType B)
    {
        return GetTypeHash(A) == GetTypeHash(B);
    }

    template <typename ComparableKey> static FORCEINLINE bool Matches(KeyInitType A, ComparableKey B)
    {
        return GetTypeHash(A) == GetTypeHash(B);
    }

    static FORCEINLINE uint32 GetKeyHash(KeyInitType Key)
    {
        return GetTypeHash(Key);
    }

    template <typename ComparableKey> static FORCEINLINE uint32 GetKeyHash(ComparableKey Key)
    {
        return GetTypeHash(Key);
    }
};

template <typename ValueType>
class TSocketListeningMap : public TMap<
                                FSocketListeningMapKey,
                                ValueType,
                                FDefaultSetAllocator,
                                TSocketListeningMapHashableKeyFuncs<ValueType>>
{
};

}

namespace Redpoint::EOS::Networking::Socket
{
REDPOINT_EOS_FILE_NS_EXPORT(3442240798, Redpoint::EOS::Networking::Socket, TSocketListeningMap)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()