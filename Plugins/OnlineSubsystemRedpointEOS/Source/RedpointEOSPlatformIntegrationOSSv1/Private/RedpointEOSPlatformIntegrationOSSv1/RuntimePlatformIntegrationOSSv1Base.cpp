// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationOSSv1/RuntimePlatformIntegrationOSSv1Base.h"

#include "RedpointEOSCore/Utils/MakeSharedWithEvents.h"
#include "RedpointEOSPlatformIntegrationOSSv1/Services/OSSv1RuntimePlatformAuthService.h"
#include "RedpointEOSPlatformIntegrationOSSv1/Services/OSSv1RuntimePlatformAvatarService.h"
#include "RedpointEOSPlatformIntegrationOSSv1/Services/OSSv1RuntimePlatformPresenceService.h"
#include "RedpointEOSPlatformIntegrationOSSv1/Services/OSSv1RuntimePlatformRelationshipService.h"
#include "RedpointEOSPlatformIntegrationOSSv1/Services/OSSv1RuntimePlatformUserService.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1765837869, Redpoint::EOS::Platform::Integration::OSSv1)
{
using namespace ::Redpoint::EOS::Platform::Integration::OSSv1::Services;
using namespace ::Redpoint::EOS::Core::Utils;

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

TSharedRef<IRuntimePlatformRelationshipService> FRuntimePlatformIntegrationOSSv1Base::GetRelationshipService() const
{
    return this->GetOSSv1RelationshipService();
}

TSharedRef<IRuntimePlatformPresenceService> FRuntimePlatformIntegrationOSSv1Base::GetPresenceService() const
{
    return this->GetOSSv1PresenceService();
}

void FRuntimePlatformIntegrationOSSv1Base::OnLocalUserSignInBeforeSystemStart(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FSimpleDelegate &OnComplete)
{
    this->GetOSSv1RelationshipService()->RegisterEvents(
        CallContext,
        FSimpleDelegate::CreateLambda([PresenceOSSv1 = this->GetOSSv1PresenceService(), CallContext, OnComplete]() {
            PresenceOSSv1->RegisterEventsBeforeLogin(CallContext, OnComplete);
        }));
}

void FRuntimePlatformIntegrationOSSv1Base::OnLocalUserSignInAfterSystemStart(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FSimpleDelegate &OnComplete)
{
    this->GetOSSv1PresenceService()->RestoreInvitesAfterLogin(CallContext, OnComplete);
}

void FRuntimePlatformIntegrationOSSv1Base::OnLocalUserSignOutAfterSystemStop(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FSimpleDelegate &OnComplete)
{
    this->GetOSSv1PresenceService()->UnregisterEvents(CallContext);
    this->GetOSSv1RelationshipService()->UnregisterEvents(CallContext);
    OnComplete.ExecuteIfBound();
}

#endif

}; // namespace REDPOINT_EOS_FILE_NS_ID(1765837869,Redpoint::EOS::Platform::Integration::OSSv1)

REDPOINT_EOS_CODE_GUARD_END()