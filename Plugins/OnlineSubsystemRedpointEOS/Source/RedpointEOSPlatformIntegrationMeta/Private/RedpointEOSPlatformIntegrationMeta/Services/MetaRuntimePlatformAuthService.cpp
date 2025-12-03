// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationMeta/Services/MetaRuntimePlatformAuthService.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_META_ENABLED
#include "OVR_Platform.h"
#include "OVR_PlatformVersion.h"
#include "RedpointEOSAuth/AuthenticationConstants.h"
#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "RedpointEOSPlatform/Android/AndroidEnvironment.h"
#include "RedpointEOSPlatformIntegrationMeta/Auth/MetaExternalCredentials.h"
#include "RedpointEOSPlatformIntegrationMeta/Logging.h"
#include "RedpointEOSPlatformIntegrationMeta/MetaSDK.h"
#include "RedpointEOSPlatformIntegrationMeta/PlatformName.h"
#if PLATFORM_ANDROID
#include "Android/AndroidApplication.h"
#include "Android/AndroidJNI.h"
#include "RedpointEOSPlatform/Android/AndroidEnvironment.h"
#endif
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_META_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(540198365, Redpoint::EOS::Platform::Integration::Meta::Services)
{
using namespace ::Redpoint::EOS::API;
#if PLATFORM_ANDROID
using namespace ::Redpoint::EOS::Platform::Android;
#endif
using namespace ::Redpoint::EOS::Platform::Integration::Meta;
using namespace ::Redpoint::EOS::Platform::Integration::Meta::Auth;

FName FMetaRuntimePlatformAuthService::GetPlatformName() const
{
    return PlatformNameMeta;
}

int32 FMetaRuntimePlatformAuthService::GetSelectionScore(const FRuntimePlatformServiceCallContextRef &CallContext) const
{
#if PLATFORM_ANDROID
    if (FAndroidEnvironment::IsMetaQuest())
    {
        // If we're running on the Quest itself, then we always want to use Meta Quest authentication.
        return 1000;
    }
#elif PLATFORM_WINDOWS
    if (FMetaSDK::IsMetaQuestLinkOnDesktop())
    {
        if (ovr_Entitlement_GetIsViewerEntitled())
        {
            // We're able to initialize OVR platform, and it tells us that the user is entitled to the current app,
            // so we should be able to perform sign-in.
            return 200;
        }
        else
        {
            // Emit a warning in case developers are expecting it to authenticate with Meta and we skip it due to a lack
            // of entitlement.
            UE_LOG(
                LogRedpointEOSPlatformIntegrationMeta,
                Warning,
                TEXT("OVR API was able to initialize, but the current user is not entitled to the app, so we will not "
                     "attempt Meta authentication."));
            return 0;
        }
    }
#endif

    // We're not running on the Meta Quest, and we're not running on PC with an entitlement to the app.
    return 0;
}

void FMetaRuntimePlatformAuthService::EnsureSignedInOnLocalPlatform(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FOnEnsureSignedInOnLocalPlatform &OnComplete)
{
    FMetaSDK::GetLoggedInUser(FMetaSDK::FOnGetLoggedInUserComplete::CreateSPLambda(
        this,
        [OnComplete](const FError &ErrorCode, const TOptional<ovrUserHandle> &) {
            OnComplete.ExecuteIfBound(ErrorCode);
        }));
}

void FMetaRuntimePlatformAuthService::IsOnlinePlayPermitted(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FOnIsOnlinePlayPermitted &OnComplete)
{
    // Meta Quest always permits online play.
    OnComplete.ExecuteIfBound(Errors::Success());
}

void FMetaRuntimePlatformAuthService::GetCredentialsFromLocalPlatformDuringAuthenticationGraph(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const TSharedRef<FAuthenticationGraphState> &State,
    const FOnGetCredentialsFromLocalPlatformDuringAuthenticationGraph &OnComplete)
{
    UE_LOG(LogRedpointEOSPlatformIntegrationMeta, Verbose, TEXT("Obtaining logged in user ID..."));
    FMetaSDK::GetLoggedInUser(FMetaSDK::FOnGetLoggedInUserComplete::CreateSPLambda(
        this,
        [this, State, OnComplete](const FError &UserErrorCode, const TOptional<ovrUserHandle> &UserHandle) {
            if (!UserErrorCode.WasSuccessful())
            {
                UE_LOG(LogRedpointEOSPlatformIntegrationMeta, Error, TEXT("Call to get logged in user ID failed."));
                OnComplete.ExecuteIfBound(UserErrorCode);
                return;
            }

#if REDPOINT_EOS_META_HAS_USER_GET_DISPLAY_NAME
            FString DisplayName = UTF8_TO_TCHAR(ovr_User_GetDisplayName(UserHandle.GetValue()));
            if (DisplayName.IsEmpty())
            {
                // The display name can be empty, even on newer SDKs. Fallback to the Oculus ID if the display name is
                // blank.
                DisplayName = UTF8_TO_TCHAR(ovr_User_GetOculusID(UserHandle.GetValue()));
            }
#else
            FString DisplayName = UTF8_TO_TCHAR(ovr_User_GetOculusID(UserHandle.GetValue()));
#endif
            auto UserId = ovr_User_GetID(UserHandle.GetValue());

            UE_LOG(
                LogRedpointEOSPlatformIntegrationMeta,
                Verbose,
                TEXT("Checking that the current user is entitled to the Meta Quest app..."));
            FMetaSDK::GetIsViewerEntitled(FMetaSDK::FOnGetIsViewerEntitledComplete::CreateSPLambda(
                this,
                [this, DisplayName, UserId, State, OnComplete](const FError &EntitlementErrorCode) {
                    if (!EntitlementErrorCode.WasSuccessful())
                    {
                        UE_LOG(
                            LogRedpointEOSPlatformIntegrationMeta,
                            Error,
                            TEXT("The current user is not entitled to use this Meta Quest app: %s"),
                            *EntitlementErrorCode.ToLogString());
                        OnComplete.ExecuteIfBound(EntitlementErrorCode);
                        return;
                    }

                    UE_LOG(
                        LogRedpointEOSPlatformIntegrationMeta,
                        Verbose,
                        TEXT("Obtaining user proof to send to EOS backend..."));
                    FMetaSDK::GetUserProof(FMetaSDK::FOnGetUserProofComplete::CreateSPLambda(
                        this,
                        [UserId, DisplayName, State, OnComplete](
                            const FError &ProofErrorCode,
                            const TOptional<FString> &ProofNonce) {
                            if (!ProofErrorCode.WasSuccessful())
                            {
                                UE_LOG(
                                    LogRedpointEOSPlatformIntegrationMeta,
                                    Error,
                                    TEXT("Call to get logged in user proof failed."));
                                OnComplete.ExecuteIfBound(ProofErrorCode);
                                return;
                            }

                            FString MetaToken = FString::Printf(TEXT("%llu|%s"), UserId, *ProofNonce.GetValue());

                            TMap<FString, FString> MetaAttrs;
                            MetaAttrs.Add(EOS_AUTH_ATTRIBUTE_AUTHENTICATEDWITH, TEXT("meta"));
                            MetaAttrs.Add(TEXT("meta.userId"), FString::Printf(TEXT("%llu"), UserId));
                            MetaAttrs.Add(TEXT("meta.proofNonce"), ProofNonce.GetValue());
                            // deprecated
                            MetaAttrs.Add(TEXT("oculus.userId"), FString::Printf(TEXT("%llu"), UserId));
                            MetaAttrs.Add(TEXT("oculus.proofNonce"), ProofNonce.GetValue());

                            UE_LOG(
                                LogRedpointEOSPlatformIntegrationMeta,
                                Verbose,
                                TEXT("Authenticating with Meta user proof token: %s"),
                                *MetaToken);

                            State->AvailableExternalCredentials.Add(MakeShared<FMetaExternalCredentials>(
                                MetaToken,
                                DisplayName,
                                MetaAttrs,
                                FString::Printf(TEXT("%llu"), UserId)));
                            OnComplete.ExecuteIfBound(ProofErrorCode);
                        }));
                }));
        }));
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()