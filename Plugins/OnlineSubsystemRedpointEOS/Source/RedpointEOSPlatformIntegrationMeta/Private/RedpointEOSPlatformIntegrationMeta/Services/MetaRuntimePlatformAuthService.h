// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_META_ENABLED
#include "RedpointEOSAuth/AuthenticationGraph.h"
#include "RedpointEOSPlatform/Services/RuntimePlatformAuthService.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_META_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(540198365, Redpoint::EOS::Platform::Integration::Meta::Services)
{
using namespace ::Redpoint::EOS::Platform::Services;
using namespace ::Redpoint::EOS::Auth;

class FMetaRuntimePlatformAuthService : public IRuntimePlatformAuthService,
                                        public TSharedFromThis<FMetaRuntimePlatformAuthService>
{
public:
    FMetaRuntimePlatformAuthService() = default;
    UE_NONCOPYABLE(FMetaRuntimePlatformAuthService);
    virtual ~FMetaRuntimePlatformAuthService() override = default;

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

}

namespace Redpoint::EOS::Platform::Integration::Meta::Services
{
REDPOINT_EOS_FILE_NS_EXPORT(
    540198365,
    Redpoint::EOS::Platform::Integration::Meta::Services,
    FMetaRuntimePlatformAuthService)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()