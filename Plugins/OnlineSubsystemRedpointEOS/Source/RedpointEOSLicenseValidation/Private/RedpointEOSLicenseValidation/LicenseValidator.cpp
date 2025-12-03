// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSLicenseValidation/LicenseValidator.h"

#if REDPOINT_EOS_IS_FREE_EDITION

#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Misc/Paths.h"
#include "RedpointEOSLicenseValidation/Logging.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(213993558, Redpoint::EOS::LicenseValidation)
{
using namespace ::Redpoint::EOS::LicenseValidation;

TSharedRef<FExclusiveAsyncOperationQueue> FLicenseValidator::OperationQueue =
    MakeShared<FExclusiveAsyncOperationQueue>();
bool FLicenseValidator::bHasChecked = false;
bool FLicenseValidator::bHasValidLicense = false;
FString FLicenseValidator::LastCheckedLicenseKey = TEXT("");

void FLicenseValidator::OnExecuteOperation(
    const FExclusiveAsyncOperationQueue::FOnDone &InOnDone,
    FString InLicenseKey,
    bool bInPerformEditorAgreementCheck,
    FOnLicenseValidatorComplete InOnLicenseValidatorComplete)
{
    auto OnComplete = [InOnDone, InOnLicenseValidatorComplete](bool bIsValid) {
        InOnLicenseValidatorComplete.ExecuteIfBound(bIsValid);
        InOnDone.ExecuteIfBound();
    };

    if (bInPerformEditorAgreementCheck)
    {
#if WITH_EDITOR
#if PLATFORM_WINDOWS
        FString FlagPath =
            FString(FPlatformMisc::GetEnvironmentVariable(TEXT("USERPROFILE"))) / ".eos-free-edition-agreed";
        FString VersionPath =
            FString(FPlatformMisc::GetEnvironmentVariable(TEXT("USERPROFILE"))) / ".eos-free-edition-latest-version";
#else
        FString FlagPath = FString(FPlatformMisc::GetEnvironmentVariable(TEXT("HOME"))) / ".eos-free-edition-agreed";
        FString VersionPath =
            FString(FPlatformMisc::GetEnvironmentVariable(TEXT("HOME"))) / ".eos-free-edition-latest-version";
#endif

        if (!FPaths::FileExists(FlagPath))
        {
            // Have not yet accepted license agreement in editor.
            UE_LOG(
                LogRedpointEOSLicenseValidation,
                Error,
                TEXT("You need to accept the license agreement in the editor before this operation will succeed."))
            OnComplete(false);
            return;
        }

#if defined(REDPOINT_EOS_BUILD_VERSION_NAME)
        if (FPaths::FileExists(VersionPath))
        {
            FString AllowedVersionString;
            if (FFileHelper::LoadFileToString(AllowedVersionString, *VersionPath))
            {
                TArray<FString> AllowedVersions;
                AllowedVersionString.ParseIntoArrayLines(AllowedVersions, true);
                if (AllowedVersions.Num() > 0)
                {
                    bool bAllowedVersionFound = false;
                    for (auto AllowedVersion : AllowedVersions)
                    {
                        if (AllowedVersion == FString(REDPOINT_EOS_BUILD_VERSION_NAME))
                        {
                            bAllowedVersionFound = true;
                            break;
                        }
                    }

                    if (!bAllowedVersionFound)
                    {
                        // Version too old.
                        UE_LOG(
                            LogRedpointEOSLicenseValidation,
                            Error,
                            TEXT("Your version of Redpoint EOS Online Framework Free Edition is too old. Please upgrade to the "
                                 "latest version at https://licensing.redpoint.games/get/eos-online-subsystem-free. "
                                 "This version check only applies in the editor, and does not apply to packaged games."))
                        OnInvalidLicense();
                        return;
                    }
                }
            }
        }
#endif
#endif
    }

    // If we have already checked in a previous request, don't do it again.
    if (FLicenseValidator::bHasChecked && FLicenseValidator::LastCheckedLicenseKey == InLicenseKey)
    {
        OnComplete(FLicenseValidator::bHasValidLicense);
        return;
    }

    // Test that a license key was provided at all.
    if (InLicenseKey.IsEmpty())
    {
        UE_LOG(
            LogRedpointEOSLicenseValidation,
            Error,
            TEXT("Missing license key for Online Subsystem EOS Free Edition. Set the FreeEditionLicenseKey= "
                 "value in DefaultEngine.ini. You can get a license key from "
                 "https://licensing.redpoint.games/get/eos-online-subsystem-free."))
        FLicenseValidator::bHasChecked = true;
        FLicenseValidator::bHasValidLicense = false;
        FLicenseValidator::LastCheckedLicenseKey = InLicenseKey;
        OnComplete(FLicenseValidator::bHasValidLicense);
        return;
    }

    // Send a HTTP request to the endpoint.
    UE_LOG(LogRedpointEOSLicenseValidation, Verbose, TEXT("Sending request to validation server."));
    auto HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetURL(TEXT("https://licensing.redpoint.games/api/validate"));
    HttpRequest->SetHeader("Authorization", FString::Printf(TEXT("Bearer %s"), *InLicenseKey));
    HttpRequest->SetHeader("Audience", TEXT("epic-online-subsystem-free"));
    HttpRequest->SetVerb(TEXT("GET"));
    HttpRequest->SetHeader(TEXT("Content-Length"), TEXT("0"));
    HttpRequest->OnProcessRequestComplete().BindLambda(
        [OnComplete, InLicenseKey](FHttpRequestPtr, FHttpResponsePtr HttpResponse, bool bSucceeded) {
            // If we fail to get a response from the licensing server, assume the license key is valid. This
            // prevents outages from impacting games.
            if (!HttpResponse.IsValid() || HttpResponse->GetResponseCode() != 200)
            {
                UE_LOG(
                    LogRedpointEOSLicenseValidation,
                    Warning,
                    TEXT("Could not get valid response from licensing server. Assuming license is valid."));
                FLicenseValidator::bHasValidLicense = true;
                FLicenseValidator::bHasChecked = true;
                FLicenseValidator::LastCheckedLicenseKey = InLicenseKey;
                OnComplete(FLicenseValidator::bHasValidLicense);
                return;
            }

            FString Result = HttpResponse->GetContentAsString();
            if (Result == TEXT("missing_authorization_header") || Result == TEXT("expected_bearer_token") ||
                Result == TEXT("missing_audience_header") || Result == TEXT("expected_audience") ||
                Result == TEXT("token_validation_failed"))
            {
                // Developer has not configured the product correctly (e.g. no license key provided or it wasn't
                // cryptographically valid).
                UE_LOG(
                    LogRedpointEOSLicenseValidation,
                    Error,
                    TEXT("License key validation failed with error code '%s'. Check that you have set the "
                         "license key in the configuration. You can get a license key from "
                         "https://licensing.redpoint.games/get/eos-online-subsystem-free."),
                    *Result);
                FLicenseValidator::bHasValidLicense = false;
                FLicenseValidator::bHasChecked = true;
                FLicenseValidator::LastCheckedLicenseKey = InLicenseKey;
                OnComplete(FLicenseValidator::bHasValidLicense);
                return;
            }

            if (Result == TEXT("wrong_product_for_license_key"))
            {
                // Developer has used the wrong type of license key for this product.
                UE_LOG(
                    LogRedpointEOSLicenseValidation,
                    Error,
                    TEXT("License key validation failed because the license key is not for this type of "
                         "product. Go to https://licensing.redpoint.games/get/eos-online-subsystem-free to "
                         "obtain a license key for Redpoint EOS Online Framework (Free Edition)."));
                FLicenseValidator::bHasValidLicense = false;
                FLicenseValidator::bHasChecked = true;
                FLicenseValidator::LastCheckedLicenseKey = InLicenseKey;
                OnComplete(FLicenseValidator::bHasValidLicense);
                return;
            }

            if (Result == TEXT("license_key_revoked"))
            {
                // The license key has been revoked.
                UE_LOG(
                    LogRedpointEOSLicenseValidation,
                    Error,
                    TEXT("License key validation failed because the license key has been revoked. If you feel "
                         "this is in error, please contact support."));
                FLicenseValidator::bHasValidLicense = false;
                FLicenseValidator::bHasChecked = true;
                FLicenseValidator::LastCheckedLicenseKey = InLicenseKey;
                OnComplete(FLicenseValidator::bHasValidLicense);
                return;
            }

            if (Result == TEXT("license_ok"))
            {
                // The license key is valid.
                UE_LOG(LogRedpointEOSLicenseValidation, Verbose, TEXT("License key validation succeeded."));
                FLicenseValidator::bHasValidLicense = true;
                FLicenseValidator::bHasChecked = true;
                FLicenseValidator::LastCheckedLicenseKey = InLicenseKey;
                OnComplete(FLicenseValidator::bHasValidLicense);
                return;
            }

            // For everything else, including "internal_error", we assume the license key is valid. This
            // prevents validation server bugs from impacting games.
            UE_LOG(
                LogRedpointEOSLicenseValidation,
                Warning,
                TEXT("Could not get valid response from licensing server. Assuming license is valid."));
            FLicenseValidator::bHasValidLicense = true;
            FLicenseValidator::bHasChecked = true;
            FLicenseValidator::LastCheckedLicenseKey = InLicenseKey;
            OnComplete(FLicenseValidator::bHasValidLicense);
        });
    HttpRequest->ProcessRequest();
}

void FLicenseValidator::ValidateLicenseKey(
    const FString &InLicenseKey,
    bool bInPerformEditorAgreementCheck,
    const FOnLicenseValidatorComplete &InOnLicenseValidatorComplete)
{
    FLicenseValidator::OperationQueue->AddToQueue(FExclusiveAsyncOperationQueue::FOperation::CreateStatic(
        &FLicenseValidator::OnExecuteOperation,
        InLicenseKey,
        bInPerformEditorAgreementCheck,
        InOnLicenseValidatorComplete));
}

}

REDPOINT_EOS_CODE_GUARD_END()

#endif