// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSPlatform/RuntimePlatformIntegration.h"
#include "RedpointEOSPlatform/Types/ExternalAccountIdInfo.h"
#include "RedpointEOSPlatformIntegrationOSSv1/ResolveUserIdToExternalAccountIdInfo.h"
#include "RedpointEOSPlatformIntegrationOSSv1/RuntimePlatformIntegrationOSSv1Base.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(764385463, Redpoint::EOS::Platform::Integration::OSSv1)
{
using namespace ::Redpoint::EOS::Platform;
using namespace ::Redpoint::EOS::Platform::Services;
using namespace ::Redpoint::EOS::Platform::Types;
using namespace ::Redpoint::EOS::Platform::Integration::OSSv1;
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
using namespace ::Redpoint::EOS::Platform::Integration::OSSv1::Services;
#endif

class REDPOINTEOSPLATFORMINTEGRATIONOSSV1_API FRuntimePlatformIntegrationOSSv1
    : public FRuntimePlatformIntegrationOSSv1Base
{
private:
    TSharedRef<IRuntimePlatformAvatarService> AvatarService;
    TSharedRef<IRuntimePlatformUserService> UserService;
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    TSharedRef<IRuntimePlatformAuthService> AuthService;
    TSharedRef<FOSSv1RuntimePlatformRelationshipService> RelationshipService;
    TSharedRef<FOSSv1RuntimePlatformPresenceService> PresenceService;
#endif

protected:
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    virtual TSharedRef<FOSSv1RuntimePlatformPresenceService> GetOSSv1PresenceService() const override final;
    virtual TSharedRef<FOSSv1RuntimePlatformRelationshipService> GetOSSv1RelationshipService() const override final;
#endif

public:
    FRuntimePlatformIntegrationOSSv1(
        const EOS_EExternalAccountType &InExternalAccountTypeToHandle,
        const FName &InSubsystemName,
        const FResolveUserIdToExternalAccountIdInfo &InExternalInfoResolver,
        const FString &InFriendsListName,
        const FString &InRecentPlayersNamespace,
        EOS_EExternalCredentialType InObtainedCredentialType,
        const FString &InAuthenticatedWithValue,
        const FString &InTokenAuthAttributeName);
    FRuntimePlatformIntegrationOSSv1(
        const EOS_EExternalAccountType &InExternalAccountTypeToHandle,
        const FName &InSubsystemName,
        const FResolveUserIdToExternalAccountIdInfo &InExternalInfoResolver,
        const FString &InFriendsListName,
        const FString &InRecentPlayersNamespace
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
        ,
        const TSharedRef<IRuntimePlatformAuthService> &InAuthService
#endif
    );
    UE_NONCOPYABLE(FRuntimePlatformIntegrationOSSv1);
    virtual ~FRuntimePlatformIntegrationOSSv1() override = default;

    virtual TSharedRef<IRuntimePlatformAvatarService> GetAvatarService() const override final;
    virtual TSharedRef<IRuntimePlatformUserService> GetUserService() const override final;
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    virtual TSharedRef<IRuntimePlatformAuthService> GetAuthService() const override final;
#endif
};

};

namespace Redpoint::EOS::Platform::Integration::OSSv1
{
REDPOINT_EOS_FILE_NS_EXPORT(764385463, Redpoint::EOS::Platform::Integration::OSSv1, FRuntimePlatformIntegrationOSSv1)
}

REDPOINT_EOS_CODE_GUARD_END()