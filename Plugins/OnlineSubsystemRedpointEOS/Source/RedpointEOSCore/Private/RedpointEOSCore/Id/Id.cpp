// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSCore/Id/Id.h"

#include "RedpointEOSCore/Id/EOSString_OpaqueId.h"
#include "RedpointEOSCore/Id/OnlineAccountIdRegistryRedpointEOS.h"
#include "RedpointEOSCore/Id/OnlineAccountIdRegistryRedpointEOSImpl.h"
#include "RedpointEOSCore/Logging.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3130306466, Redpoint::EOS::Core::Id)
{
using namespace ::Redpoint::EOS::Core;
using namespace ::Redpoint::EOS::Core::Id;
using namespace UE::Online;

bool IsValidId(const UE::Online::FAccountId &AccountId)
{
    return AccountId.IsValid() && IOnlineAccountIdRegistryRedpointEOS::GetRegistered().Get(AccountId).IsValid();
}

bool IsDedicatedServerId(const UE::Online::FAccountId &AccountId)
{
    return AccountId.GetOnlineServicesType() == EOnlineServices::Epic && AccountId.GetHandle() == 1 &&
           IOnlineAccountIdRegistryRedpointEOS::GetRegistered().Get(AccountId).IsValid();
}

bool IsProductUserId(const UE::Online::FAccountId &AccountId)
{
    return AccountId.GetOnlineServicesType() == EOnlineServices::Epic && AccountId.GetHandle() >= 1000 &&
           IOnlineAccountIdRegistryRedpointEOS::GetRegistered().Get(AccountId).IsValid();
}

EOS_ProductUserId GetProductUserId(const UE::Online::FAccountId &AccountId)
{
    if (AccountId.GetOnlineServicesType() == EOnlineServices::Epic && AccountId.GetHandle() >= 1000)
    {
        TSharedPtr<const FUniqueNetIdEOS> UserId = IOnlineAccountIdRegistryRedpointEOS::GetRegistered().Get(AccountId);
        if (UserId != nullptr && UserId->HasValidProductUserId())
        {
            return UserId->GetProductUserId();
        }
    }
    UE_LOG(
        LogRedpointEOSCore,
        Warning,
        TEXT("Redpoint::EOS::Core::Id::GetProductUserId called with invalid AccountId (%u)!"),
        AccountId.GetHandle());
    return nullptr;
}

FString GetUserIdString(const UE::Online::FAccountId &AccountId)
{
    if (AccountId.GetOnlineServicesType() == EOnlineServices::Epic && AccountId.GetHandle() >= 1000)
    {
        TSharedPtr<const FUniqueNetIdEOS> UserId = IOnlineAccountIdRegistryRedpointEOS::GetRegistered().Get(AccountId);
        if (UserId != nullptr)
        {
            return UserId->ToString();
        }
    }
    UE_LOG(
        LogRedpointEOSCore,
        Warning,
        TEXT("Redpoint::EOS::Core::Id::GetUserIdString called with invalid AccountId (%u)!"),
        AccountId.GetHandle());
    return TEXT("");
}

FString GetUserIdDebugString(const UE::Online::FAccountId &AccountId)
{
    if (AccountId.GetOnlineServicesType() == EOnlineServices::Epic)
    {
        if (AccountId.GetHandle() >= 1000)
        {
            TSharedPtr<const FUniqueNetIdEOS> UserId =
                IOnlineAccountIdRegistryRedpointEOS::GetRegistered().Get(AccountId);
            if (UserId != nullptr)
            {
                return UserId->ToDebugString();
            }
        }
        else if (AccountId.GetHandle() == 1)
        {
            return TEXT("(dedicated server)");
        }
        else if (AccountId.GetHandle() == 0)
        {
            return TEXT("(invalid account ID)");
        }
    }
    UE_LOG(
        LogRedpointEOSCore,
        Warning,
        TEXT("Redpoint::EOS::Core::Id::GetProductUserIdString called with invalid AccountId (%u)!"),
        AccountId.GetHandle());
    return TEXT("(invalid account ID)");
}

UE::Online::FAccountId GetAccountId(const FUniqueNetId &UserId)
{
    return IOnlineAccountIdRegistryRedpointEOS::GetRegistered().Get(UserId);
}

UE::Online::FAccountId GetAccountId(const TSharedRef<const FUniqueNetId> &UserId)
{
    return IOnlineAccountIdRegistryRedpointEOS::GetRegistered().Get(UserId);
}

UE::Online::FAccountId GetAccountId(const EOS_ProductUserId &UserId)
{
    return IOnlineAccountIdRegistryRedpointEOS::GetRegistered().Get(UserId);
}

UE::Online::FAccountId GetAccountId(const FUniqueNetIdRepl &UserId)
{
    if (UserId.IsV1())
    {
        auto Ptr = UserId.GetV1();
        if (Ptr.IsValid())
        {
            return IOnlineAccountIdRegistryRedpointEOS::GetRegistered().Get(*Ptr);
        }
        else
        {
            return GetInvalidAccountId();
        }
    }
    else
    {
        return UserId.GetV2();
    }
}

UE::Online::FAccountId GetDedicatedServerAccountId()
{
    return ((FOnlineAccountIdRegistryRedpointEOSImpl &)IOnlineAccountIdRegistryRedpointEOS::GetRegistered())
        .GetDedicatedServerId();
}

UE::Online::FAccountId GetInvalidAccountId()
{
    return ((FOnlineAccountIdRegistryRedpointEOSImpl &)IOnlineAccountIdRegistryRedpointEOS::GetRegistered())
        .GetInvalidId();
}

TOptional<UE::Online::FAccountId> TryParseAccountId(const FString &UserIdString)
{
    TSharedPtr<const FUniqueNetIdEOS> UniqueNetId = FUniqueNetIdEOS::ParseFromString(UserIdString);
    if (UniqueNetId.IsValid())
    {
        return IOnlineAccountIdRegistryRedpointEOS::GetRegistered().Get(*UniqueNetId);
    }
    return TOptional<UE::Online::FAccountId>();
}

TSharedPtr<const FUniqueNetIdEOS> GetUniqueNetId(const UE::Online::FAccountId &AccountId)
{
    if (IsDedicatedServerId(AccountId))
    {
        return FUniqueNetIdEOS::DedicatedServerId();
    }
    else if (AccountId.GetOnlineServicesType() == EOnlineServices::Epic && AccountId.GetHandle() >= 1000)
    {
        return IOnlineAccountIdRegistryRedpointEOS::GetRegistered().Get(AccountId);
    }
    UE_LOG(
        LogRedpointEOSCore,
        Warning,
        TEXT("Redpoint::EOS::Core::Id::GetUniqueNetId called with invalid AccountId (%u)!"),
        AccountId.GetHandle());
    return nullptr;
}

TSharedRef<const FUniqueNetIdEOS> GetUniqueNetIdRef(const UE::Online::FAccountId &AccountId)
{
    if (IsDedicatedServerId(AccountId))
    {
        return FUniqueNetIdEOS::DedicatedServerId();
    }
    else if (AccountId.GetOnlineServicesType() == EOnlineServices::Epic && AccountId.GetHandle() >= 1000)
    {
        auto UserId = IOnlineAccountIdRegistryRedpointEOS::GetRegistered().Get(AccountId);
        if (UserId.IsValid() && UserId->IsValid())
        {
            return UserId.ToSharedRef();
        }
    }
    UE_LOG(
        LogRedpointEOSCore,
        Warning,
        TEXT("Redpoint::EOS::Core::Id::GetUniqueNetIdRef called with invalid AccountId (%u)!"),
        AccountId.GetHandle());
    return FUniqueNetIdEOS::InvalidId();
}

FUniqueNetIdRepl GetUniqueNetIdRepl(const UE::Online::FAccountId &AccountId)
{
    // We should use this call instead of SetUniqueNetId when all of Unreal Engine expects to be using OSSv2.
    //
    // Value.SetAccountId(AccountId);

    FUniqueNetIdRepl Value;
    Value.SetUniqueNetId(GetUniqueNetId(AccountId));
    return Value;
}

void SerializeAccountId(FArchive &Archive, UE::Online::FAccountId &AccountId)
{
    if (Archive.IsLoading())
    {
        TArray<uint8> RepData;
        Archive << RepData;
        AccountId = IOnlineAccountIdRegistryRedpointEOS::GetRegistered().FromReplicationData(RepData);
    }
    else
    {
        auto RepData = IOnlineAccountIdRegistryRedpointEOS::GetRegistered().ToReplicationData(AccountId);
        Archive << RepData;
    }
}

void DumpAccountIdRegistry()
{
    return IOnlineAccountIdRegistryRedpointEOS::GetRegistered().DumpRegistry();
}

EOS_EpicAccountId GetEpicGamesAccountId(const FString &EpicGamesAccountId)
{
    EOS_EpicAccountId AccountId;
    if (EOSString_EpicAccountId::FromString(EpicGamesAccountId, AccountId) == EOS_EResult::EOS_Success)
    {
        return AccountId;
    }
    return nullptr;
}

FString GetEpicGamesUserIdString(const EOS_EpicAccountId &AccountId)
{
    FString AccountIdString;
    if (EOSString_EpicAccountId::ToString(AccountId, AccountIdString) != EOS_EResult::EOS_Success)
    {
        return TEXT("");
    }
    return AccountIdString;
}

}

REDPOINT_EOS_CODE_GUARD_END()