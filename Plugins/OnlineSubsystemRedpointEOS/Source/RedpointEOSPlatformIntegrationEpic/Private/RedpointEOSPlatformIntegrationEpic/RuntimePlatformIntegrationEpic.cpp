// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationEpic/RuntimePlatformIntegrationEpic.h"
#include "RedpointEOSPlatformIntegrationEpic/Services/EpicRuntimePlatformPresenceService.h"
#include "RedpointEOSPlatformIntegrationEpic/Services/EpicRuntimePlatformRelationshipService.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3219386061, Redpoint::EOS::Platform::Integration::Epic)
{
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
using namespace ::Redpoint::EOS::Platform::Integration::Epic::Services;
#endif

FRuntimePlatformIntegrationEpic::FRuntimePlatformIntegrationEpic()
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    : PresenceService(MakeShared<FEpicRuntimePlatformPresenceService>())
    , RelationshipService(MakeShared<FEpicRuntimePlatformRelationshipService>())
#endif
{
}

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

TSharedRef<IRuntimePlatformPresenceService> FRuntimePlatformIntegrationEpic::GetPresenceService() const
{
    return this->PresenceService;
}

TSharedRef<IRuntimePlatformRelationshipService> FRuntimePlatformIntegrationEpic::GetRelationshipService() const
{
    return this->RelationshipService;
}

void FRuntimePlatformIntegrationEpic::OnLocalUserSignInBeforeSystemStart(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FSimpleDelegate &OnComplete)
{
    auto PresenceImpl = StaticCastSharedRef<FEpicRuntimePlatformPresenceService>(this->PresenceService);
    StaticCastSharedRef<FEpicRuntimePlatformRelationshipService>(this->RelationshipService)
        ->RegisterEvents(CallContext, FSimpleDelegate::CreateLambda([CallContext, OnComplete, PresenceImpl]() {
                             PresenceImpl->RegisterEvents(CallContext, OnComplete);
                         }));
}

void FRuntimePlatformIntegrationEpic::OnLocalUserSignOutAfterSystemStop(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FSimpleDelegate &OnComplete)
{
    StaticCastSharedRef<FEpicRuntimePlatformPresenceService>(this->PresenceService)->UnregisterEvents(CallContext);
    StaticCastSharedRef<FEpicRuntimePlatformRelationshipService>(this->RelationshipService)
        ->UnregisterEvents(CallContext);
    OnComplete.ExecuteIfBound();
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()
