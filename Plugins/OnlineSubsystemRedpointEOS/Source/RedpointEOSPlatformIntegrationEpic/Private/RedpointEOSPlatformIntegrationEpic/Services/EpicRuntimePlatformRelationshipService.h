// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSAPI/EventHandle.h"
#include "RedpointEOSPlatform/Services/RuntimePlatformRelationshipService.h"
#include "RedpointEOSPlatformIntegrationEpic/EpicExternalUserInfo.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1578170046, Redpoint::EOS::Platform::Integration::Epic::Services)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Platform::Services;
using namespace ::Redpoint::EOS::Platform::Types;
using namespace ::Redpoint::EOS::Platform::Integration::Epic;

class FEpicRuntimePlatformRelationshipService : public IRuntimePlatformRelationshipService,
                                                public TSharedFromThis<FEpicRuntimePlatformRelationshipService>
{
private:
    class FPlatformStorage
    {
    public:
        FEventHandlePtr OnBlockedUsersUpdate;
        FEventHandlePtr OnFriendsUpdate;
        TSet<int32> LocalUsersSignedIn;
    };
    class FUserStorage
    {
    public:
        TOptional<FString> EpicGamesAccountId;
        IRuntimePlatformRelationshipService::FOnRelationshipsChanged OnRelationshipsChangedDelegate;
        TMap<FString, TSharedRef<FEpicExternalUserInfo>> EpicExternalUserInfoCache;
    };
    TMap<FRuntimePlatformServiceCallContext::FPlatformKey, FPlatformStorage> PlatformStorage;
    TMap<FRuntimePlatformServiceCallContext::FUserKey, FUserStorage> UserStorage;

public:
    FEpicRuntimePlatformRelationshipService();
    UE_NONCOPYABLE(FEpicRuntimePlatformRelationshipService);
    virtual ~FEpicRuntimePlatformRelationshipService() override = default;

    virtual FString GetCacheKey() const override;

    virtual bool AllowRelationshipCaching() const override;

    virtual void GetRelationships(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        ERelationshipType RelationshipTypes,
        const FOnGetRelationshipComplete &OnComplete) override;

    virtual FOnRelationshipsChanged &OnRelationshipsChanged(
        const FRuntimePlatformServiceCallContextRef &CallContext) override;

    void RegisterEvents(const FRuntimePlatformServiceCallContextRef &CallContext, const FSimpleDelegate &OnComplete);

    void UnregisterEvents(const FRuntimePlatformServiceCallContextRef &CallContext);
};

}

namespace Redpoint::EOS::Platform::Integration::Epic::Services
{
REDPOINT_EOS_FILE_NS_EXPORT(
    1578170046,
    Redpoint::EOS::Platform::Integration::Epic::Services,
    FEpicRuntimePlatformRelationshipService)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()