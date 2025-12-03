// Copyright June Rhodes. All Rights Reserved.

#include "VerifyFreeEditionLicenseCommandlet.h"
#include "Containers/Ticker.h"
#include "Engine/Engine.h"
#include "Misc/App.h"
#include "Misc/Parse.h"
#include "RedpointEOSLicenseValidation/LicenseValidator.h"
#include "RedpointEOSLicenseValidation/Logging.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UVerifyFreeEditionLicenseCommandlet::UVerifyFreeEditionLicenseCommandlet(const FObjectInitializer &ObjectInitializer)
    : Super(ObjectInitializer)
{
    IsClient = false;
    IsServer = false;
    IsEditor = true;

    LogToConsole = true;
    ShowErrorCount = true;
    ShowProgress = true;
}

int32 UVerifyFreeEditionLicenseCommandlet::Main(const FString &Params)
{
    using namespace ::Redpoint::EOS::LicenseValidation;

#if !REDPOINT_EOS_IS_FREE_EDITION
    UE_LOG(
        LogRedpointEOSLicenseValidation,
        Error,
        TEXT("This build of Redpoint EOS Online Framework is not the Free Edition, so license keys can not be validated."));
    return 1;
#else
    FString LicenseKey;
    if (!FParse::Value(*Params, TEXT("-LICENSEKEY="), LicenseKey))
    {
        UE_LOG(LogRedpointEOSLicenseValidation, Error, TEXT("Missing -LICENSEKEY= parameter."));
        return 1;
    }

    bool bSuccessful = false;

    FLicenseValidator::ValidateLicenseKey(
        LicenseKey,
        false,
        FOnLicenseValidatorComplete::CreateLambda([&bSuccessful](bool bIsLicenseValid) {
            bSuccessful = bIsLicenseValid;
            RequestEngineExit(bSuccessful ? TEXT("License verified successfully!") : TEXT("License failed to verify!"));
        }));

    if (!IsEngineExitRequested())
    {
        GIsRunning = true;
        while (GIsRunning && !IsEngineExitRequested())
        {
            GEngine->UpdateTimeAndHandleMaxTickRate();
            GEngine->Tick((float)FApp::GetDeltaTime(), false);
            GFrameCounter++;
            FTSTicker::GetCoreTicker().Tick((float)FApp::GetDeltaTime());
            FPlatformProcess::Sleep(0);
        }
        GIsRunning = false;
    }
    return bSuccessful ? 0 : 1;
#endif
}

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()