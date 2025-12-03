// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Online/CoreOnline.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

namespace Redpoint::EOS::Core::Id
{

template <typename ValueType>
struct TUniqueNetIdMapHashableKeyFuncs
    : BaseKeyFuncs<TPair<TSharedRef<const FUniqueNetId>, ValueType>, ValueType, false>
{
    typedef typename TTypeTraits<TSharedRef<const FUniqueNetId>>::ConstPointerType KeyInitType;
    typedef const TPairInitializer<
        typename TTypeTraits<TSharedRef<const FUniqueNetId>>::ConstInitType,
        typename TTypeTraits<ValueType>::ConstInitType> &ElementInitType;

    static FORCEINLINE KeyInitType GetSetKey(ElementInitType Element)
    {
        return Element.Key;
    }

    static FORCEINLINE FString GetKeyValue(KeyInitType K)
    {
        return FString::Printf(TEXT("%s_%s"), *K->GetType().ToString(), *K->ToString());
    }

    static FORCEINLINE bool Matches(KeyInitType A, KeyInitType B)
    {
        return GetKeyValue(A) == GetKeyValue(B);
    }

    template <typename ComparableKey> static FORCEINLINE bool Matches(KeyInitType A, ComparableKey B)
    {
        return GetKeyValue(A) == GetKeyValue(B);
    }

    static FORCEINLINE uint32 GetKeyHash(KeyInitType Key)
    {
        return GetTypeHash(GetKeyValue(Key));
    }

    template <typename ComparableKey> static FORCEINLINE uint32 GetKeyHash(ComparableKey Key)
    {
        return GetTypeHash(GetKeyValue(Key));
    }
};

template <typename ValueType>
class TUniqueNetIdMap : public TMap<
                            TSharedRef<const FUniqueNetId>,
                            ValueType,
                            FDefaultSetAllocator,
                            TUniqueNetIdMapHashableKeyFuncs<ValueType>>
{
private:
    using TUniqueNetIdMapBase = TMap<
        TSharedRef<const FUniqueNetId>,
        ValueType,
        FDefaultSetAllocator,
        TUniqueNetIdMapHashableKeyFuncs<ValueType>>;
    using TUniqueNetIdMapBaseConst = const TMap<
        TSharedRef<const FUniqueNetId>,
        ValueType,
        FDefaultSetAllocator,
        TUniqueNetIdMapHashableKeyFuncs<ValueType>>;

public:
    FORCEINLINE ValueType &operator[](const FUniqueNetId &Key)
    {
        return TUniqueNetIdMapBase::operator[](Key.AsShared());
    }
    FORCEINLINE const ValueType &operator[](const FUniqueNetId &Key) const
    {
        return TUniqueNetIdMapBaseConst::operator[](Key.AsShared());
    }

    bool Contains(const FUniqueNetId &Key) const
    {
        return TUniqueNetIdMapBaseConst::Contains(Key.AsShared());
    }
    void Add(const FUniqueNetId &Key, ValueType Value)
    {
        TUniqueNetIdMapBase::Add(Key.AsShared(), Value);
    }
    void Remove(const FUniqueNetId &Key)
    {
        TUniqueNetIdMapBase::Remove(Key.AsShared());
    }
};

}