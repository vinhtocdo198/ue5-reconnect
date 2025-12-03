// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatform/RuntimePlatformIntegration.h"

#include "RedpointEOSPlatform/Services/NullRuntimePlatformAuthService.h"
#include "RedpointEOSPlatform/Services/NullRuntimePlatformAvatarService.h"
#include "RedpointEOSPlatform/Services/NullRuntimePlatformPresenceService.h"
#include "RedpointEOSPlatform/Services/NullRuntimePlatformRelationshipService.h"
#include "RedpointEOSPlatform/Services/NullRuntimePlatformUserService.h"

namespace REDPOINT_EOS_FILE_NS_ID(131982838, Redpoint::EOS::Platform)
{
using namespace ::Redpoint::EOS::Platform;

TSharedRef<Services::IRuntimePlatformAvatarService> IRuntimePlatformIntegration::GetAvatarService() const
{
    return Services::NullAvatarService;
}

TSharedRef<Services::IRuntimePlatformUserService> IRuntimePlatformIntegration::GetUserService() const
{
    return Services::NullUserService;
}

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

TSharedRef<Services::IRuntimePlatformAuthService> IRuntimePlatformIntegration::GetAuthService() const
{
    return Services::NullAuthService;
}

TSharedRef<Services::IRuntimePlatformRelationshipService> IRuntimePlatformIntegration::GetRelationshipService() const
{
    return Services::NullRelationshipService;
}

TSharedRef<Services::IRuntimePlatformPresenceService> IRuntimePlatformIntegration::GetPresenceService() const
{
    return Services::NullPresenceService;
}

void IRuntimePlatformIntegration::OnLocalUserSignInBeforeSystemStart(
    const Services::FRuntimePlatformServiceCallContextRef &CallContext,
    const FSimpleDelegate &OnComplete)
{
    OnComplete.ExecuteIfBound();
}

void IRuntimePlatformIntegration::OnLocalUserSignInAfterSystemStart(
    const Services::FRuntimePlatformServiceCallContextRef &CallContext,
    const FSimpleDelegate &OnComplete)
{
    OnComplete.ExecuteIfBound();
}

void IRuntimePlatformIntegration::OnLocalUserSignOutBeforeSystemStop(
    const Services::FRuntimePlatformServiceCallContextRef &CallContext,
    const FSimpleDelegate &OnComplete)
{
    OnComplete.ExecuteIfBound();
}

void IRuntimePlatformIntegration::OnLocalUserSignOutAfterSystemStop(
    const Services::FRuntimePlatformServiceCallContextRef &CallContext,
    const FSimpleDelegate &OnComplete)
{
    OnComplete.ExecuteIfBound();
}

#endif

}