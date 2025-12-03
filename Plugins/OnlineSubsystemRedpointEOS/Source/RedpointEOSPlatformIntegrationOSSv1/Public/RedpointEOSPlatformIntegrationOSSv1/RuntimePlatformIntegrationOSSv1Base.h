// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSPlatform/RuntimePlatformIntegration.h"
#include "RedpointEOSPlatform/Types/ExternalAccountIdInfo.h"
#include "RedpointEOSPlatformIntegrationOSSv1/ResolveUserIdToExternalAccountIdInfo.h"
#include "RedpointEOSPlatformIntegrationOSSv1/Services/OSSv1RuntimePlatformPresenceService.h"
#include "RedpointEOSPlatformIntegrationOSSv1/Services/OSSv1RuntimePlatformRelationshipService.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1765837869, Redpoint::EOS::Platform::Integration::OSSv1)
{
using namespace ::Redpoint::EOS::Platform;
using namespace ::Redpoint::EOS::Platform::Services;
using namespace ::Redpoint::EOS::Platform::Types;
using namespace ::Redpoint::EOS::Platform::Integration::OSSv1;
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
using namespace ::Redpoint::EOS::Platform::Integration::OSSv1::Services;
#endif

class REDPOINTEOSPLATFORMINTEGRATIONOSSV1_API FRuntimePlatformIntegrationOSSv1Base : public IRuntimePlatformIntegration
{
protected:
    FRuntimePlatformIntegrationOSSv1Base() = default;

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    virtual TSharedRef<FOSSv1RuntimePlatformPresenceService> GetOSSv1PresenceService() const = 0;
    virtual TSharedRef<FOSSv1RuntimePlatformRelationshipService> GetOSSv1RelationshipService() const = 0;
#endif

public:
    UE_NONCOPYABLE(FRuntimePlatformIntegrationOSSv1Base);
    virtual ~FRuntimePlatformIntegrationOSSv1Base() override = default;

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    virtual TSharedRef<IRuntimePlatformRelationshipService> GetRelationshipService() const override final;
    virtual TSharedRef<IRuntimePlatformPresenceService> GetPresenceService() const override final;

    virtual void OnLocalUserSignInBeforeSystemStart(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FSimpleDelegate &OnComplete) override final;
    virtual void OnLocalUserSignInAfterSystemStart(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FSimpleDelegate &OnComplete) override final;
    virtual void OnLocalUserSignOutAfterSystemStop(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FSimpleDelegate &OnComplete) override final;
#endif
};

};

namespace Redpoint::EOS::Platform::Integration::OSSv1
{
REDPOINT_EOS_FILE_NS_EXPORT(
    1765837869,
    Redpoint::EOS::Platform::Integration::OSSv1,
    FRuntimePlatformIntegrationOSSv1Base)
}

REDPOINT_EOS_CODE_GUARD_END()