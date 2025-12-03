// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSPlatform/Services/RuntimePlatformAuthService.h"
#include "RedpointEOSPlatform/Services/RuntimePlatformAvatarService.h"
#include "RedpointEOSPlatform/Services/RuntimePlatformPresenceService.h"
#include "RedpointEOSPlatform/Services/RuntimePlatformRelationshipService.h"
#include "RedpointEOSPlatform/Services/RuntimePlatformServiceCallContext.h"
#include "RedpointEOSPlatform/Services/RuntimePlatformUserService.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(131982838, Redpoint::EOS::Platform)
{
using namespace ::Redpoint::EOS::Platform;

class REDPOINTEOSPLATFORM_API IRuntimePlatformIntegration
{
public:
    IRuntimePlatformIntegration() = default;
    UE_NONCOPYABLE(IRuntimePlatformIntegration);
    virtual ~IRuntimePlatformIntegration() = default;

    virtual TSharedRef<Services::IRuntimePlatformAvatarService> GetAvatarService() const;
    virtual TSharedRef<Services::IRuntimePlatformUserService> GetUserService() const;

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    virtual TSharedRef<Services::IRuntimePlatformAuthService> GetAuthService() const;
    virtual TSharedRef<Services::IRuntimePlatformRelationshipService> GetRelationshipService() const;
    virtual TSharedRef<Services::IRuntimePlatformPresenceService> GetPresenceService() const;

    virtual void OnLocalUserSignInBeforeSystemStart(
        const Services::FRuntimePlatformServiceCallContextRef &CallContext,
        const FSimpleDelegate &OnComplete);
    virtual void OnLocalUserSignInAfterSystemStart(
        const Services::FRuntimePlatformServiceCallContextRef &CallContext,
        const FSimpleDelegate &OnComplete);

    virtual void OnLocalUserSignOutBeforeSystemStop(
        const Services::FRuntimePlatformServiceCallContextRef &CallContext,
        const FSimpleDelegate &OnComplete);
    virtual void OnLocalUserSignOutAfterSystemStop(
        const Services::FRuntimePlatformServiceCallContextRef &CallContext,
        const FSimpleDelegate &OnComplete);
#endif
};

}

namespace Redpoint::EOS::Platform
{
REDPOINT_EOS_FILE_NS_EXPORT(131982838, Redpoint::EOS::Platform, IRuntimePlatformIntegration)
}

REDPOINT_EOS_CODE_GUARD_END()