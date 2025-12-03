// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "Containers/Map.h"
#include "Containers/Set.h"
#include "RedpointEOSInterfaces/Interfaces/OnlineLobbyInterface.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

struct TLobbyIdSetHashableKeyFuncs
    : BaseKeyFuncs<TSharedRef<const FOnlineLobbyId>, TSharedRef<const FOnlineLobbyId>, false>
{
    typedef typename TTypeTraits<TSharedRef<const FOnlineLobbyId>>::ConstPointerType KeyInitType;
    typedef typename TTypeTraits<TSharedRef<const FOnlineLobbyId>>::ConstPointerType ElementInitType;

    static FORCEINLINE KeyInitType GetSetKey(ElementInitType Element)
    {
        return Element;
    }

    static FORCEINLINE FString GetKeyValue(KeyInitType K)
    {
        return FString::Printf(TEXT("%s"), *K->ToString());
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

class TLobbyIdSet : public TSet<TSharedRef<const FOnlineLobbyId>, TLobbyIdSetHashableKeyFuncs, FDefaultSetAllocator>
{
private:
    using TLobbyIdSetBase = TSet<TSharedRef<const FOnlineLobbyId>, TLobbyIdSetHashableKeyFuncs, FDefaultSetAllocator>;
    using TLobbyIdSetBaseConst =
        const TSet<TSharedRef<const FOnlineLobbyId>, TLobbyIdSetHashableKeyFuncs, FDefaultSetAllocator>;

public:
    bool Contains(const FOnlineLobbyId &Key) const
    {
        return TLobbyIdSetBaseConst::Contains(Key.AsShared());
    }
    void Add(const FOnlineLobbyId &Key)
    {
        TLobbyIdSetBase::Add(Key.AsShared());
    }
    void Remove(const FOnlineLobbyId &Key)
    {
        TLobbyIdSetBase::Remove(Key.AsShared());
    }
};

#endif

REDPOINT_EOS_CODE_GUARD_END()