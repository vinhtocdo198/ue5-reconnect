// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationMeta/RuntimePlatformIntegrationMeta.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_META_ENABLED
#include "RedpointEOSCore/Utils/MakeSharedWithEvents.h"
#include "RedpointEOSPlatformIntegrationMeta/Services/MetaRuntimePlatformAuthService.h"
#include "RedpointEOSPlatformIntegrationMeta/Services/MetaRuntimePlatformAvatarService.h"
#include "RedpointEOSPlatformIntegrationMeta/Services/MetaRuntimePlatformRelationshipService.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_META_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(1706371436, Redpoint::EOS::Platform::Integration::Meta)
{
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::Platform::Services;
using namespace ::Redpoint::EOS::Platform::Integration::Meta::Services;

FRuntimePlatformIntegrationMeta::FRuntimePlatformIntegrationMeta()
    : AvatarService(MakeShared<FMetaRuntimePlatformAvatarService>())
    , AuthService(MakeShared<FMetaRuntimePlatformAuthService>())
    , RelationshipService(MakeShared<FMetaRuntimePlatformRelationshipService>())
    , PresenceService(MakeSharedWithEvents<FMetaRuntimePlatformPresenceService>())
{
}

TSharedRef<IRuntimePlatformAvatarService> FRuntimePlatformIntegrationMeta::GetAvatarService() const
{
    return this->AvatarService;
}

TSharedRef<IRuntimePlatformAuthService> FRuntimePlatformIntegrationMeta::GetAuthService() const
{
    return this->AuthService;
}

TSharedRef<IRuntimePlatformRelationshipService> FRuntimePlatformIntegrationMeta::GetRelationshipService() const
{
    return this->RelationshipService;
}

TSharedRef<IRuntimePlatformPresenceService> FRuntimePlatformIntegrationMeta::GetPresenceService() const
{
    return this->PresenceService;
}

void FRuntimePlatformIntegrationMeta::OnLocalUserSignInBeforeSystemStart(
    const Services::FRuntimePlatformServiceCallContextRef &CallContext,
    const FSimpleDelegate &OnComplete)
{
    this->PresenceService->SetPlatformHandle(CallContext->PlatformHandle);
    OnComplete.ExecuteIfBound();
}

void FRuntimePlatformIntegrationMeta::OnLocalUserSignInAfterSystemStart(
    const Services::FRuntimePlatformServiceCallContextRef &CallContext,
    const FSimpleDelegate &OnComplete)
{
    this->PresenceService->FlushDeeplinkEventsAfterLogin();
    OnComplete.ExecuteIfBound();
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()