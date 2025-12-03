// Copyright June Rhodes. All Rights Reserved.

#pragma once

#if REDPOINT_EOS_IS_FREE_EDITION

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSIdentity/IdentityHook.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3130854758, Redpoint::EOS::LicenseValidation)
{
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::API;

class FFreeEditionLicenseVerificationIdentityHook : public IIdentityHook,
                                                    public TSharedFromThis<FFreeEditionLicenseVerificationIdentityHook>
{
public:
    FFreeEditionLicenseVerificationIdentityHook() = default;
    UE_NONCOPYABLE(FFreeEditionLicenseVerificationIdentityHook);
    virtual ~FFreeEditionLicenseVerificationIdentityHook() override = default;

    virtual FString GetHookDebugName() const override
    {
        return TEXT("FreeEditionLicenseVerificationIdentityHook");
    }

    virtual void OnPreLogin(
        const FIdentityHookContext &InContext,
        const FIdentityUserSlot &InUserSlot,
        const FOnPreLoginComplete &InOnComplete) override;
};

}

namespace Redpoint::EOS::LicenseValidation
{
REDPOINT_EOS_FILE_NS_EXPORT(3130854758, Redpoint::EOS::LicenseValidation, FFreeEditionLicenseVerificationIdentityHook)
}

REDPOINT_EOS_CODE_GUARD_END()

#endif