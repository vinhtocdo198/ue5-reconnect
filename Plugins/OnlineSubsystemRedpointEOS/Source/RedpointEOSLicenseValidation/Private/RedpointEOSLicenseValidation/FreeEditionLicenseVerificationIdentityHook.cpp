// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSLicenseValidation/FreeEditionLicenseVerificationIdentityHook.h"

#if REDPOINT_EOS_IS_FREE_EDITION

#include "RedpointEOSConfig/Config.h"
#include "RedpointEOSLicenseValidation/LicenseValidator.h"
#include "RedpointEOSLicenseValidation/Logging.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3130854758, Redpoint::EOS::LicenseValidation)
{
using namespace ::Redpoint::EOS::LicenseValidation;
using namespace ::Redpoint::EOS::API;

void FFreeEditionLicenseVerificationIdentityHook::OnPreLogin(
    const FIdentityHookContext &InContext,
    const FIdentityUserSlot &InUserSlot,
    const FOnPreLoginComplete &InOnComplete)
{
    FLicenseValidator::ValidateLicenseKey(
        InContext.Environment->Config->GetFreeEditionLicenseKey(),
        true,
        FOnLicenseValidatorComplete::CreateSPLambda(this, [InOnComplete](bool bIsLicenseValid) {
            if (bIsLicenseValid)
            {
                InOnComplete.ExecuteIfBound(Errors::Success());
            }
            else
            {
                InOnComplete.ExecuteIfBound(Errors::AccessDenied(
                    ANSI_TO_TCHAR(__FUNCTION__),
                    TEXT("Invalid license key for EOS. Please report this issue to the game developer.")));
            }
        }));
}

}

REDPOINT_EOS_CODE_GUARD_END()

#endif