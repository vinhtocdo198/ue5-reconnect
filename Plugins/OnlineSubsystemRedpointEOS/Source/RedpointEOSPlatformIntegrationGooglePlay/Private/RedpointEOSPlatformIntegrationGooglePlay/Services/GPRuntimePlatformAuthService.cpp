// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationGooglePlay/Services/GPRuntimePlatformAuthService.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && EOS_GOOGLE_PLAY_ENABLED
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#if PLATFORM_ANDROID
#include "Android/AndroidApplication.h"
#include "Android/AndroidJNI.h"
#include "RedpointEOSPlatform/Android/AndroidEnvironment.h"
#endif
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && EOS_GOOGLE_PLAY_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(2109748584, Redpoint::EOS::Platform::Integration::GooglePlay::Services)
{
using namespace ::Redpoint::EOS::Platform::Android;

FGooglePlayRuntimePlatformAuthService::FGooglePlayRuntimePlatformAuthService(
    FName InPlatformName,
    EOS_EExternalCredentialType InObtainedCredentialType,
    const FString &InAuthenticatedWithValue,
    const FString &InTokenAuthAttributeName)
    : FOSSv1RuntimePlatformAuthService(
          InPlatformName,
          InObtainedCredentialType,
          InAuthenticatedWithValue,
          InTokenAuthAttributeName)
{
}

int32 FGooglePlayRuntimePlatformAuthService::GetSelectionScore(
    const FRuntimePlatformServiceCallContextRef &CallContext) const
{
#if PLATFORM_ANDROID
    if (!FAndroidEnvironment::IsMetaQuest())
    {
        IOnlineSubsystem *GoogleOSS = Online::GetSubsystem(CallContext->World.Get(), FName(TEXT("RedpointGoogle")));
        if (GoogleOSS != nullptr)
        {
            IOnlineIdentityPtr GoogleIdentity = GoogleOSS->GetIdentityInterface();
            if (GoogleIdentity.IsValid())
            {
                return 1000;
            }
        }
    }
#endif

    return 0;
}

};

#endif

REDPOINT_EOS_CODE_GUARD_END()