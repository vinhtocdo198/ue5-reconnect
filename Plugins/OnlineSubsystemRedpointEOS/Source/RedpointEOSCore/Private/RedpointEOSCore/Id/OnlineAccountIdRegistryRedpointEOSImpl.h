// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/OnlineAccountIdRegistryRedpointEOS.h"
#include "RedpointEOSCore/Id/UniqueNetIdEOS.h"
#include "RedpointEOSCore/Id/UserIdMap.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(554787920, Redpoint::EOS::Core::Id)
{
using namespace UE::Online;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Core::Id;

class FOnlineAccountIdRegistryRedpointEOSImpl : public IOnlineAccountIdRegistryRedpointEOS
{
private:
    TMap<uint32, TSharedPtr<const FUniqueNetIdEOS>> AccountIdToUniqueNetId;
    TUserIdMap<FAccountId> UniqueNetIdToAccountId;
    uint32 NextAccountId;

    TSharedPtr<const FUniqueNetIdEOS> LookupByAccountId(uint32 InHandle) const;

public:
    FOnlineAccountIdRegistryRedpointEOSImpl();
    UE_NONCOPYABLE(FOnlineAccountIdRegistryRedpointEOSImpl);
    virtual ~FOnlineAccountIdRegistryRedpointEOSImpl() override = default;

    static FOnlineAccountIdRegistryRedpointEOSImpl &Instance();

    FAccountId GetInvalidId() const;
    FAccountId GetDedicatedServerId() const;
    virtual FAccountId Get(const FUniqueNetId &InUserId) override;
    virtual FAccountId Get(const TSharedRef<const FUniqueNetId> &InUserId) override;
    virtual FAccountId Get(const EOS_ProductUserId &InProductUserId) override;
    virtual TSharedPtr<const FUniqueNetIdEOS> Get(const FAccountId &InAccountId) const override;

#if REDPOINT_EOS_UE_5_3_OR_LATER
    virtual FString ToString(const FAccountId &AccountId) const override;
#endif
    virtual FString ToLogString(const FAccountId &AccountId) const override;
    virtual TArray<uint8> ToReplicationData(const FAccountId &AccountId) const override;
    virtual FAccountId FromReplicationData(const TArray<uint8> &ReplicationData) override;

    virtual void DumpRegistry() const override;
};

}

namespace Redpoint::EOS::Core::Id
{
REDPOINT_EOS_FILE_NS_EXPORT(554787920, Redpoint::EOS::Core::Id, FOnlineAccountIdRegistryRedpointEOSImpl)
}

REDPOINT_EOS_CODE_GUARD_END()