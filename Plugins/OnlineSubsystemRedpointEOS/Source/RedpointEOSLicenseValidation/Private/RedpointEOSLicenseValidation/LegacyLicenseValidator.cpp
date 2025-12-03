// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSLicenseValidation/LegacyLicenseValidator.h"

#if REDPOINT_EOS_IS_FREE_EDITION

#include "RedpointEOSLicenseValidation/LicenseValidator.h"
#include "RedpointEOSLicenseValidation/Logging.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1254383436, Redpoint::EOS::LicenseValidation)
{
using namespace ::Redpoint::EOS::LicenseValidation;

void FLegacyLicenseValidator::ValidateLicense(
    TSharedRef<IConfig> Config,
    FLicenseValidatorCallback OnInvalidLicense,
    FLicenseValidatorCallback OnValidLicense)
{
    FLicenseValidator::ValidateLicenseKey(
        Config->GetFreeEditionLicenseKey(),
        true,
        FOnLicenseValidatorComplete::CreateLambda([OnInvalidLicense, OnValidLicense](bool bIsValid) {
            if (bIsValid)
            {
                OnValidLicense();
            }
            else
            {
                OnInvalidLicense();
            }
        }));
}

}

REDPOINT_EOS_CODE_GUARD_END()

#endif