// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationSteam/RuntimePlatformIntegrationSteam.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && defined(REDPOINT_EOS_STEAM_ENABLED)
#include "RedpointEOSPlatformIntegrationSteam/Services/SteamRuntimePlatformAuthService.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && defined(REDPOINT_EOS_STEAM_ENABLED)

namespace REDPOINT_EOS_FILE_NS_ID(1893130842, Redpoint::EOS::Platform::Integration::Steam)
{
using namespace ::Redpoint::EOS::Platform::Integration::Steam::Services;
using namespace ::Redpoint::EOS::Platform::Types;

FRuntimePlatformIntegrationSteam::FRuntimePlatformIntegrationSteam()
    : FRuntimePlatformIntegrationOSSv1(
          EOS_EExternalAccountType::EOS_EAT_STEAM,
          FName(TEXT("Steam")),
          [](const FUniqueNetId &InUserId) -> TOptional<FExternalAccountIdInfo> {
              if (!InUserId.GetType().IsEqual(STEAM_SUBSYSTEM))
              {
                  return TOptional<FExternalAccountIdInfo>();
              }

              FExternalAccountIdInfo Info;
              Info.AccountIdType = EOS_EExternalAccountType::EOS_EAT_STEAM;
              Info.AccountId = InUserId.ToString();
              return Info;
          },
          TEXT(""),
          TEXT(""),
          MakeShared<FSteamRuntimePlatformAuthService>())
{
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()