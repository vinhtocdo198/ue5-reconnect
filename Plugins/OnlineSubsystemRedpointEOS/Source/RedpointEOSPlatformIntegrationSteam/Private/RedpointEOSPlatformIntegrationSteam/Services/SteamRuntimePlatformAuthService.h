// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER && defined(REDPOINT_EOS_STEAM_ENABLED)
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSPlatform/Services/RuntimePlatformAuthService.h"
#include "RedpointEOSPlatformIntegrationOSSv1/Services/OSSv1RuntimePlatformAuthService.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && defined(REDPOINT_EOS_STEAM_ENABLED)

namespace REDPOINT_EOS_FILE_NS_ID(3144435893, Redpoint::EOS::Platform::Integration::Steam::Services)
{
using namespace ::Redpoint::EOS::Platform::Services;
using namespace ::Redpoint::EOS::Platform::Integration::OSSv1::Services;
using namespace ::Redpoint::EOS::Auth;

class FSteamRuntimePlatformAuthService : public FOSSv1RuntimePlatformAuthService
{
public:
    FSteamRuntimePlatformAuthService();
    UE_NONCOPYABLE(FSteamRuntimePlatformAuthService);
    virtual ~FSteamRuntimePlatformAuthService() override = default;

    virtual int32 GetSelectionScore(const FRuntimePlatformServiceCallContextRef &CallContext) const override;

    virtual void GetCredentialsFromLocalPlatformDuringAuthenticationGraph(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const TSharedRef<FAuthenticationGraphState> &State,
        const FOnGetCredentialsFromLocalPlatformDuringAuthenticationGraph &OnComplete) override;
};

}; // namespace REDPOINT_EOS_FILE_NS_ID(3144435893,Redpoint::EOS::Platform::Integration::OSSv1::Services)

namespace Redpoint::EOS::Platform::Integration::Steam::Services
{
REDPOINT_EOS_FILE_NS_EXPORT(
    3144435893,
    Redpoint::EOS::Platform::Integration::Steam::Services,
    FSteamRuntimePlatformAuthService)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()