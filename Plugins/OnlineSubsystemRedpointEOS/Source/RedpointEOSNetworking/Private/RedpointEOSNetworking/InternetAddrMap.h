// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "IPAddress.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(450748605, Redpoint::EOS::Networking)
{

template <typename ValueType>
struct TInternetAddrMapHashableKeyFuncs
    : BaseKeyFuncs<TPair<TSharedRef<const FInternetAddr>, ValueType>, ValueType, false>
{
    typedef typename TTypeTraits<TSharedRef<const FInternetAddr>>::ConstPointerType KeyInitType;
    typedef const TPairInitializer<
        typename TTypeTraits<TSharedRef<const FInternetAddr>>::ConstInitType,
        typename TTypeTraits<ValueType>::ConstInitType> &ElementInitType;

    static FORCEINLINE KeyInitType GetSetKey(ElementInitType Element)
    {
        return Element.Key;
    }

    static FORCEINLINE bool Matches(KeyInitType A, KeyInitType B)
    {
        return *A == *B;
    }

    template <typename ComparableKey> static FORCEINLINE bool Matches(KeyInitType A, ComparableKey B)
    {
        return *A == *B;
    }

    static FORCEINLINE uint32 GetKeyHash(KeyInitType Key)
    {
        return GetTypeHash(*Key);
    }

    template <typename ComparableKey> static FORCEINLINE uint32 GetKeyHash(ComparableKey Key)
    {
        return GetTypeHash(*Key);
    }
};

template <typename ValueType>
class TInternetAddrMap : public TMap<
                             TSharedRef<const FInternetAddr>,
                             ValueType,
                             FDefaultSetAllocator,
                             TInternetAddrMapHashableKeyFuncs<ValueType>>
{
};

}

namespace Redpoint::EOS::Networking
{
REDPOINT_EOS_FILE_NS_EXPORT(450748605, Redpoint::EOS::Networking, TInternetAddrMap)
}

REDPOINT_EOS_CODE_GUARD_END()