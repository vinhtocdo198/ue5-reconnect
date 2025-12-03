// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Interfaces/OnlinePartyInterface.h"
#include "Online/CoreOnline.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1327450806, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party)
{

typedef TTuple<TSharedRef<const FUniqueNetId>, TSharedRef<const FOnlinePartyId>> FOnlinePartyIdMapRedpointEOSKey;

template <typename ValueType>
struct TOnlinePartyIdMapRedpointEOSHashableKeyFuncs
    : BaseKeyFuncs<TPair<FOnlinePartyIdMapRedpointEOSKey, ValueType>, ValueType, false>
{
    typedef typename TTypeTraits<FOnlinePartyIdMapRedpointEOSKey>::ConstPointerType KeyInitType;
    typedef const TPairInitializer<
        typename TTypeTraits<FOnlinePartyIdMapRedpointEOSKey>::ConstInitType,
        typename TTypeTraits<ValueType>::ConstInitType> &ElementInitType;

    static FORCEINLINE KeyInitType GetSetKey(ElementInitType Element)
    {
        return Element.Key;
    }

    static FORCEINLINE bool Matches(KeyInitType A, KeyInitType B)
    {
        return *A.Key == *B.Key && A.Value->ToString() == B.Value->ToString();
    }

    template <typename ComparableKey> static FORCEINLINE bool Matches(KeyInitType A, ComparableKey B)
    {
        return *A.Key == *B.Key && A.Value->ToString() == B.Value->ToString();
    }

    static FORCEINLINE uint32 GetKeyHash(KeyInitType K)
    {
        return HashCombine(GetTypeHash(*K.Key), GetTypeHash(K.Value->ToString()));
    }

    template <typename ComparableKey> static FORCEINLINE uint32 GetKeyHash(ComparableKey K)
    {
        return HashCombine(GetTypeHash(*K.Key), GetTypeHash(K.Value->ToString()));
    }
};

template <typename ValueType>
class TOnlinePartyIdMapRedpointEOS : public TMap<
                                         FOnlinePartyIdMapRedpointEOSKey,
                                         ValueType,
                                         FDefaultSetAllocator,
                                         TOnlinePartyIdMapRedpointEOSHashableKeyFuncs<ValueType>>
{
public:
    FORCEINLINE ValueType FindRef(const FUniqueNetId &LocalUserId, const FOnlinePartyId &PartyId)
    {
        return FindRef(FOnlinePartyIdMapRedpointEOSKey(LocalUserId.AsShared(), PartyId.AsShared()));
    }

    FORCEINLINE ValueType
    FindRef(const TSharedRef<const FUniqueNetId> &LocalUserId, const TSharedRef<const FOnlinePartyId> &PartyId)
    {
        return FindRef(FOnlinePartyIdMapRedpointEOSKey(LocalUserId, PartyId));
    }
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party
{
REDPOINT_EOS_FILE_NS_EXPORT(
    1327450806,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party,
    FOnlinePartyIdMapRedpointEOSKey)
REDPOINT_EOS_FILE_NS_EXPORT(
    1327450806,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party,
    TOnlinePartyIdMapRedpointEOSHashableKeyFuncs)
REDPOINT_EOS_FILE_NS_EXPORT(
    1327450806,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party,
    TOnlinePartyIdMapRedpointEOS)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()