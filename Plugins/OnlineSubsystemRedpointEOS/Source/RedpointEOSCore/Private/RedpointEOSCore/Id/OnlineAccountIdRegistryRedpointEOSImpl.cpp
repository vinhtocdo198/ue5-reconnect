// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSCore/Id/OnlineAccountIdRegistryRedpointEOSImpl.h"

#include "RedpointEOSCore/Id/EOSString_OpaqueId.h"
#include "RedpointEOSCore/Logging.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(554787920, Redpoint::EOS::Core::Id)
{
using namespace ::Redpoint::EOS::Core;
using namespace ::Redpoint::EOS::Core::Id;

TSharedPtr<const FUniqueNetIdEOS> FOnlineAccountIdRegistryRedpointEOSImpl::LookupByAccountId(uint32 InHandle) const
{
    if (InHandle == 0)
    {
        return FUniqueNetIdEOS::InvalidId();
    }
    else if (InHandle == 1)
    {
        return FUniqueNetIdEOS::DedicatedServerId();
    }
    const TSharedPtr<const FUniqueNetIdEOS> *UserId = this->AccountIdToUniqueNetId.Find(InHandle);
    if (UserId == nullptr || !UserId->IsValid())
    {
        return nullptr;
    }
    return *UserId;
}

FOnlineAccountIdRegistryRedpointEOSImpl::FOnlineAccountIdRegistryRedpointEOSImpl()
    : AccountIdToUniqueNetId()
    , UniqueNetIdToAccountId()
    , NextAccountId(1000)
{
    // @note: This class is initialised before the SDK is loaded, so we can not populate maps with FUniqueNetIdEOS
    // values yet!
}

FOnlineAccountIdRegistryRedpointEOSImpl &FOnlineAccountIdRegistryRedpointEOSImpl::Instance()
{
    static FOnlineAccountIdRegistryRedpointEOSImpl StaticInstance;
    return StaticInstance;
}

FAccountId FOnlineAccountIdRegistryRedpointEOSImpl::GetInvalidId() const
{
    return FAccountId(EOnlineServices::Epic, 0);
}

FAccountId FOnlineAccountIdRegistryRedpointEOSImpl::GetDedicatedServerId() const
{
    return FAccountId(EOnlineServices::Epic, 1);
}

FAccountId FOnlineAccountIdRegistryRedpointEOSImpl::Get(const FUniqueNetId &InUserId)
{
    if (InUserId.GetType() != FUniqueNetIdEOS::GetTypeStatic())
    {
        return FAccountId();
    }

    FAccountId *ExistingId = this->UniqueNetIdToAccountId.Find(InUserId.AsShared());
    if (ExistingId == nullptr)
    {
        if (InUserId == *FUniqueNetIdEOS::InvalidId())
        {
            this->AccountIdToUniqueNetId.Add(0, FUniqueNetIdEOS::InvalidId());
            this->UniqueNetIdToAccountId.Add(*FUniqueNetIdEOS::InvalidId(), this->GetInvalidId());
            return this->GetInvalidId();
        }
        else if (InUserId == *FUniqueNetIdEOS::DedicatedServerId())
        {
            this->AccountIdToUniqueNetId.Add(1, FUniqueNetIdEOS::DedicatedServerId());
            this->UniqueNetIdToAccountId.Add(*FUniqueNetIdEOS::DedicatedServerId(), this->GetDedicatedServerId());
            return this->GetDedicatedServerId();
        }
        else
        {
            uint32 AllocatedAccountId = this->NextAccountId++;
            this->AccountIdToUniqueNetId.Add(
                AllocatedAccountId,
                StaticCastSharedRef<const FUniqueNetIdEOS>(InUserId.AsShared()));
            this->UniqueNetIdToAccountId.Add(InUserId, FAccountId(EOnlineServices::Epic, AllocatedAccountId));
            return FAccountId(EOnlineServices::Epic, AllocatedAccountId);
        }
    }
    else
    {
        return *ExistingId;
    }
}

FAccountId FOnlineAccountIdRegistryRedpointEOSImpl::Get(const TSharedRef<const FUniqueNetId> &InUserId)
{
    return this->Get(*InUserId);
}

FAccountId FOnlineAccountIdRegistryRedpointEOSImpl::Get(const EOS_ProductUserId &InProductUserId)
{
    if (EOSString_ProductUserId::IsValid(InProductUserId))
    {
        return this->Get(*MakeShared<FUniqueNetIdEOS>(InProductUserId));
    }
    else
    {
        return this->GetInvalidId();
    }
}

TSharedPtr<const FUniqueNetIdEOS> FOnlineAccountIdRegistryRedpointEOSImpl::Get(const FAccountId &InAccountId) const
{
    return this->LookupByAccountId(InAccountId.GetHandle());
}

#if REDPOINT_EOS_UE_5_3_OR_LATER
FString FOnlineAccountIdRegistryRedpointEOSImpl::ToString(const FAccountId &AccountId) const
{
    checkf(
        AccountId.GetOnlineServicesType() == EOnlineServices::Epic,
        TEXT("Expected AccountId type to be EOnlineServices::Epic"));
    TSharedPtr<const FUniqueNetIdEOS> UserId = this->LookupByAccountId(AccountId.GetHandle());
    if (!UserId.IsValid())
    {
        return TEXT("");
    }
    return UserId->ToString();
}
#endif

FString FOnlineAccountIdRegistryRedpointEOSImpl::ToLogString(const FAccountId &AccountId) const
{
    checkf(
        AccountId.GetOnlineServicesType() == EOnlineServices::Epic,
        TEXT("Expected AccountId type to be EOnlineServices::Epic"));
    TSharedPtr<const FUniqueNetIdEOS> UserId = this->LookupByAccountId(AccountId.GetHandle());
    if (!UserId.IsValid())
    {
        return TEXT("");
    }
    return UserId->ToDebugString();
}

TArray<uint8> FOnlineAccountIdRegistryRedpointEOSImpl::ToReplicationData(const FAccountId &AccountId) const
{
    checkf(
        AccountId.GetOnlineServicesType() == EOnlineServices::Epic,
        TEXT("Expected AccountId type to be EOnlineServices::Epic"));
    TSharedPtr<const FUniqueNetIdEOS> UserId = this->LookupByAccountId(AccountId.GetHandle());
    if (!UserId.IsValid())
    {
        return TArray<uint8>();
    }
    return TArray<uint8>(UserId->GetBytes(), UserId->GetSize());
}

FAccountId FOnlineAccountIdRegistryRedpointEOSImpl::FromReplicationData(const TArray<uint8> &ReplicationData)
{
    FString Data;
    Data.Empty(ReplicationData.Num());
    for (int i = 0; i < ReplicationData.Num(); i++)
    {
        if (ReplicationData[i] == 0)
        {
            break;
        }
        Data += (ANSICHAR)ReplicationData[i];
    }
    TSharedPtr<const FUniqueNetIdEOS> UserId = FUniqueNetIdEOS::ParseFromString(Data);
    if (!UserId.IsValid())
    {
        return GetInvalidId();
    }
    return this->Get(UserId.ToSharedRef());
}

void FOnlineAccountIdRegistryRedpointEOSImpl::DumpRegistry() const
{
    if (UE_LOG_ACTIVE(LogRedpointEOSCore, VeryVerbose))
    {
        UE_LOG(
            LogRedpointEOSCore,
            VeryVerbose,
            TEXT("AccountIdToUniqueNetId (%d):"),
            this->AccountIdToUniqueNetId.Num());
        for (const auto &KV : this->AccountIdToUniqueNetId)
        {
            UE_LOG(
                LogRedpointEOSCore,
                VeryVerbose,
                TEXT("%u = %s"),
                KV.Key,
                KV.Value.IsValid() ? *KV.Value->ToString() : TEXT("(null)"));
        }

        UE_LOG(
            LogRedpointEOSCore,
            VeryVerbose,
            TEXT("UniqueNetIdToAccountId (%d):"),
            this->UniqueNetIdToAccountId.Num());
        for (const auto &KV : this->UniqueNetIdToAccountId)
        {
            UE_LOG(LogRedpointEOSCore, VeryVerbose, TEXT("%s = %u"), *KV.Key->ToString(), KV.Value.GetHandle());
        }
    }
}

}

REDPOINT_EOS_CODE_GUARD_END()