// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSPlatform/Services/RuntimePlatformAuthService.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1384177521, Redpoint::EOS::Platform::Services)
{
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::Platform::Services;

class FNullRuntimePlatformAuthService : public IRuntimePlatformAuthService
{
public:
    FNullRuntimePlatformAuthService() = default;
    UE_NONCOPYABLE(FNullRuntimePlatformAuthService);
    virtual ~FNullRuntimePlatformAuthService() override = default;

    virtual FName GetPlatformName() const override;
    virtual int32 GetSelectionScore(const FRuntimePlatformServiceCallContextRef &CallContext) const override;
    virtual void EnsureSignedInOnLocalPlatform(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FOnEnsureSignedInOnLocalPlatform &OnComplete) override;
    virtual void IsOnlinePlayPermitted(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FOnIsOnlinePlayPermitted &OnComplete) override;
    virtual void GetCredentialsFromLocalPlatformDuringAuthenticationGraph(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const TSharedRef<FAuthenticationGraphState> &State,
        const FOnGetCredentialsFromLocalPlatformDuringAuthenticationGraph &OnComplete) override;
};

extern TSharedRef<IRuntimePlatformAuthService> NullAuthService;

}

namespace Redpoint::EOS::Platform::Services
{
REDPOINT_EOS_FILE_NS_EXPORT(1384177521, Redpoint::EOS::Platform::Services, FNullRuntimePlatformAuthService)
REDPOINT_EOS_FILE_NS_EXPORT(1384177521, Redpoint::EOS::Platform::Services, NullAuthService)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()