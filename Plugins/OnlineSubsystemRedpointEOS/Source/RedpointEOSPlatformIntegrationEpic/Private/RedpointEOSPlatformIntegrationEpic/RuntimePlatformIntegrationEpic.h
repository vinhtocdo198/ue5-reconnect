// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSPlatform/RuntimePlatformIntegration.h"
#include "RedpointEOSPlatform/Services/RuntimePlatformPresenceService.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3219386061, Redpoint::EOS::Platform::Integration::Epic)
{
using namespace ::Redpoint::EOS::Platform;
using namespace ::Redpoint::EOS::Platform::Services;

class FRuntimePlatformIntegrationEpic : public IRuntimePlatformIntegration
{
private:
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    TSharedRef<IRuntimePlatformPresenceService> PresenceService;
    TSharedRef<IRuntimePlatformRelationshipService> RelationshipService;
#endif

public:
    FRuntimePlatformIntegrationEpic();
    UE_NONCOPYABLE(FRuntimePlatformIntegrationEpic);
    virtual ~FRuntimePlatformIntegrationEpic() override = default;

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    virtual TSharedRef<IRuntimePlatformPresenceService> GetPresenceService() const override;
    virtual TSharedRef<IRuntimePlatformRelationshipService> GetRelationshipService() const override;

    virtual void OnLocalUserSignInBeforeSystemStart(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FSimpleDelegate &OnComplete) override;
    virtual void OnLocalUserSignOutAfterSystemStop(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FSimpleDelegate &OnComplete) override;
#endif
};

}

namespace Redpoint::EOS::Platform::Integration::Epic
{
REDPOINT_EOS_FILE_NS_EXPORT(3219386061, Redpoint::EOS::Platform::Integration::Epic, FRuntimePlatformIntegrationEpic)
}

REDPOINT_EOS_CODE_GUARD_END()