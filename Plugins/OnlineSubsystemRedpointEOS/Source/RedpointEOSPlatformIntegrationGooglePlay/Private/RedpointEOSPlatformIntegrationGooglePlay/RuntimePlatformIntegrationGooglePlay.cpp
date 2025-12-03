// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationGooglePlay/RuntimePlatformIntegrationGooglePlay.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && EOS_GOOGLE_PLAY_ENABLED
#include "RedpointEOSPlatformIntegrationGooglePlay/Services/GPRuntimePlatformAuthService.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && EOS_GOOGLE_PLAY_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(4079654914, Redpoint::EOS::Platform::Integration::GooglePlay)
{
using namespace ::Redpoint::EOS::Platform::Integration::GooglePlay::Services;
using namespace ::Redpoint::EOS::Platform::Types;

FRuntimePlatformIntegrationGooglePlay::FRuntimePlatformIntegrationGooglePlay()
    : FRuntimePlatformIntegrationOSSv1(
          EOS_EExternalAccountType::EOS_EAT_GOOGLE,
          FName(TEXT("RedpointGoogle")),
          [](const FUniqueNetId &InUserId) -> TOptional<FExternalAccountIdInfo> {
              if (!InUserId.GetType().IsEqual(FName(TEXT("RedpointGoogle"))))
              {
                  return TOptional<FExternalAccountIdInfo>();
              }

              FExternalAccountIdInfo Info;
              Info.AccountIdType = EOS_EExternalAccountType::EOS_EAT_GOOGLE;
              Info.AccountId = InUserId.ToString();
              return Info;
          },
          TEXT(""),
          TEXT(""),
          MakeShared<FGooglePlayRuntimePlatformAuthService>(
              FName(TEXT("RedpointGoogle")),
              EOS_EExternalCredentialType::EOS_ECT_GOOGLE_ID_TOKEN,
              TEXT("google"),
              TEXT("google.idToken")))
{
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()