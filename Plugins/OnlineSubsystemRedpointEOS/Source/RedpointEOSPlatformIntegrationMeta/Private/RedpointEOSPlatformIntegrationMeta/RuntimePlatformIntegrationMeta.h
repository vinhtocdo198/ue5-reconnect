// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_META_ENABLED
#include "RedpointEOSPlatform/RuntimePlatformIntegration.h"
#include "RedpointEOSPlatformIntegrationMeta/Services/MetaRuntimePlatformPresenceService.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_META_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(1706371436, Redpoint::EOS::Platform::Integration::Meta)
{
using namespace ::Redpoint::EOS::Platform;
using namespace ::Redpoint::EOS::Platform::Services;
using namespace ::Redpoint::EOS::Platform::Integration::Meta::Services;

class FRuntimePlatformIntegrationMeta : public IRuntimePlatformIntegration
{
private:
    TSharedRef<IRuntimePlatformAvatarService> AvatarService;
    TSharedRef<IRuntimePlatformAuthService> AuthService;
    TSharedRef<IRuntimePlatformRelationshipService> RelationshipService;
    TSharedRef<FMetaRuntimePlatformPresenceService> PresenceService;

public:
    FRuntimePlatformIntegrationMeta();
    UE_NONCOPYABLE(FRuntimePlatformIntegrationMeta);
    virtual ~FRuntimePlatformIntegrationMeta() override = default;

    virtual TSharedRef<IRuntimePlatformAvatarService> GetAvatarService() const override;
    virtual TSharedRef<IRuntimePlatformAuthService> GetAuthService() const override;
    virtual TSharedRef<IRuntimePlatformRelationshipService> GetRelationshipService() const override;
    virtual TSharedRef<IRuntimePlatformPresenceService> GetPresenceService() const override;

    virtual void OnLocalUserSignInBeforeSystemStart(
        const Services::FRuntimePlatformServiceCallContextRef &CallContext,
        const FSimpleDelegate &OnComplete) override;
    virtual void OnLocalUserSignInAfterSystemStart(
        const Services::FRuntimePlatformServiceCallContextRef &CallContext,
        const FSimpleDelegate &OnComplete) override;
};

}

namespace Redpoint::EOS::Platform::Integration::Meta
{
REDPOINT_EOS_FILE_NS_EXPORT(1706371436, Redpoint::EOS::Platform::Integration::Meta, FRuntimePlatformIntegrationMeta)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()