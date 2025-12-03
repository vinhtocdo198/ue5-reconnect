// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "Interfaces/OnlineFriendsInterface.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineSubsystemTypes.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSCore/Id/UniqueNetIdMap.h"
#include "RedpointEOSCore/Utils/AsyncTaskGraph.h"
#include "RedpointEOSPlatform/RuntimePlatformIntegration.h"
#include "RedpointEOSPlatform/Services/RuntimePlatformRelationshipService.h"
#include "RedpointEOSPlatform/Types/PlatformStorage.h"
#include "RedpointEOSPlatformIntegrationOSSv1/ResolveUserIdToExternalAccountIdInfo.h"
#include "RedpointEOSUserCache/UserInfo.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2555362757, Redpoint::EOS::Platform::Integration::OSSv1::Services)
{
using namespace ::Redpoint::EOS::Platform;
using namespace ::Redpoint::EOS::Platform::Services;
using namespace ::Redpoint::EOS::Platform::Types;
using namespace ::Redpoint::EOS::UserCache;
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Platform::Integration::OSSv1;

class REDPOINTEOSPLATFORMINTEGRATIONOSSV1_API FOSSv1RuntimePlatformRelationshipService
    : public IRuntimePlatformRelationshipService,
      public TSharedFromThis<FOSSv1RuntimePlatformRelationshipService>
{
private:
    EOS_EExternalAccountType ExternalAccountTypeToHandle;
    FName SubsystemName;
    FResolveUserIdToExternalAccountIdInfo ExternalInfoResolver;
    FString FriendsListName;
    FString RecentPlayersNamespace;
    struct FPlatformStorage
    {
        bool bGlobalEventsRegistered;
        TMap<int32, FOnRelationshipsChanged> OnRelationshipsChangedDelegate;
        TMap<int32, FDelegateHandle> OnFriendsChangeDelegateHandle;
        TMap<int32, FDelegateHandle> OnOutgoingInviteSentDelegateHandle;
        TMap<int32, FDelegateHandle> OnBlockListChangeDelegateHandle;
        FDelegateHandle OnFriendRemovedDelegateHandle;
        FDelegateHandle OnInviteAcceptedDelegateHandle;
        FDelegateHandle OnInviteRejectedDelegateHandle;
        FDelegateHandle OnInviteReceivedDelegateHandle;
        FDelegateHandle OnRecentPlayersAddedDelegateHandle;
        TMap<int32, TMap<ERelationshipType, ::Redpoint::EOS::Core::Id::TUniqueNetIdMap<FExternalUserInfoPtr>>>
            CachedRelationships;
    };
    TPlatformStorage<FPlatformStorage> PlatformStorage;

public:
    FOSSv1RuntimePlatformRelationshipService(
        const EOS_EExternalAccountType &InExternalAccountTypeToHandle,
        const FName &InSubsystemName,
        const FResolveUserIdToExternalAccountIdInfo &InExternalInfoResolver,
        const FString &InFriendsListName,
        const FString &InRecentPlayersNamespace);
    UE_NONCOPYABLE(FOSSv1RuntimePlatformRelationshipService);
    virtual ~FOSSv1RuntimePlatformRelationshipService() override = default;

    virtual FString GetCacheKey() const override;
    virtual bool AllowRelationshipCaching() const override;

private:
    FExternalUserInfoRef CreateExternalUserInfo(const FOnlineUser &OnlineUser) const;
    void OnChangesFromOSS(const FName &InstanceName, const FUniqueNetId &LocalPlatformUserId);
    void OnChangesFromOSS(const FName &InstanceName, int32 LocalUserNum);
    void OnChangesFromOSS(
        const FPlatformHandle &PlatformHandle,
        const TSharedRef<IOnlineIdentity> &IdentityInterface,
        const TSharedRef<IOnlineFriends> &FriendsInterface,
        const FUniqueNetId &LocalPlatformUserId,
        int32 LocalUserNum);

public:
    virtual void GetRelationships(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        ERelationshipType RelationshipTypes,
        const FOnGetRelationshipComplete &OnComplete) override;

    virtual FOnRelationshipsChanged &OnRelationshipsChanged(
        const FRuntimePlatformServiceCallContextRef &CallContext) override;

    void RegisterEvents(const FRuntimePlatformServiceCallContextRef &CallContext, const FSimpleDelegate &OnComplete);

    void UnregisterEvents(const FRuntimePlatformServiceCallContextRef &CallContext);

protected:
    virtual bool CanQueryRecentPlayers() const;

private:
    void OnOSSv1FriendsChange(FName InstanceName, int32 LocalUserNum);
    void OnOSSv1OutgoingInviteSent(FName InstanceName, int32 LocalUserNum);
    void OnOSSv1BlockListChange(int32, const FString &, FName InstanceName, int32 LocalUserNum);
};

}; // namespace REDPOINT_EOS_FILE_NS_ID(2555362757,Redpoint::EOS::Platform::Integration::OSSv1::Services)

namespace Redpoint::EOS::Platform::Integration::OSSv1::Services
{
REDPOINT_EOS_FILE_NS_EXPORT(
    2555362757,
    Redpoint::EOS::Platform::Integration::OSSv1::Services,
    FOSSv1RuntimePlatformRelationshipService)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()