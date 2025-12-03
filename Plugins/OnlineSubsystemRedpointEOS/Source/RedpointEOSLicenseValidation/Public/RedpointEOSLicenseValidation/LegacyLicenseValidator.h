// Copyright June Rhodes. All Rights Reserved.

#pragma once

#if REDPOINT_EOS_IS_FREE_EDITION

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSConfig/Config.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1254383436, Redpoint::EOS::LicenseValidation)
{
using namespace ::Redpoint::EOS::Config;

typedef TFunction<void()> FLicenseValidatorCallback;

class REDPOINTEOSLICENSEVALIDATION_API FLegacyLicenseValidator
{
public:
    static void ValidateLicense(
        TSharedRef<IConfig> Config,
        FLicenseValidatorCallback OnInvalidLicense,
        FLicenseValidatorCallback OnValidLicense);
};

}

namespace Redpoint::EOS::LicenseValidation
{
REDPOINT_EOS_FILE_NS_EXPORT(1254383436, Redpoint::EOS::LicenseValidation, FLicenseValidatorCallback)
REDPOINT_EOS_FILE_NS_EXPORT(1254383436, Redpoint::EOS::LicenseValidation, FLegacyLicenseValidator)
}

REDPOINT_EOS_CODE_GUARD_END()

#endif