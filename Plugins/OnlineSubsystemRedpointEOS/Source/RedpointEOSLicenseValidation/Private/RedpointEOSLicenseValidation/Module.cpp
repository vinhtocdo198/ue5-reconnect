// Copyright June Rhodes. All Rights Reserved.

#include "Modules/ModuleManager.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSIdentity/IdentityHookRegistry.h"
#include "RedpointEOSLicenseValidation/FreeEditionLicenseVerificationIdentityHook.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2779844191, Redpoint::EOS::LicenseValidation)
{
#if REDPOINT_EOS_IS_FREE_EDITION
using namespace ::Redpoint::EOS::LicenseValidation;
#endif
using namespace ::Redpoint::EOS::Identity;

class FRedpointEOSLicenseValidationModule : public FDefaultModuleImpl
{
private:
    TSharedPtr<IIdentityHook> LicenseValidationIdentityHook;

public:
    virtual void StartupModule() override
    {
        using namespace ::Redpoint::EOS::Identity;

#if REDPOINT_EOS_IS_FREE_EDITION
        this->LicenseValidationIdentityHook = MakeShared<FFreeEditionLicenseVerificationIdentityHook>();
        FIdentityHookRegistry::Register(this->LicenseValidationIdentityHook.ToSharedRef(), 200000);
#endif
    };

    virtual void ShutdownModule() override
    {
        using namespace ::Redpoint::EOS::Identity;

        if (this->LicenseValidationIdentityHook.IsValid())
        {
            FIdentityHookRegistry::Unregister(this->LicenseValidationIdentityHook.ToSharedRef());
        }
    }
};

}

namespace Redpoint::EOS::LicenseValidation
{
REDPOINT_EOS_FILE_NS_EXPORT(2779844191, Redpoint::EOS::LicenseValidation, FRedpointEOSLicenseValidationModule)
}

REDPOINT_EOS_CODE_GUARD_END()

IMPLEMENT_MODULE(Redpoint::EOS::LicenseValidation::FRedpointEOSLicenseValidationModule, RedpointEOSLicenseValidation);