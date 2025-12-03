// Copyright June Rhodes. All Rights Reserved.

#include "Modules/ModuleManager.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSIdentity/IdentityHookRegistry.h"
#include "RedpointEOSPresence/PresenceIdentityHook.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

class REDPOINTEOSPRESENCE_API FRedpointEOSPresenceModule : public FDefaultModuleImpl
{
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
private:
    TSharedPtr<Redpoint::EOS::Identity::IIdentityHook> PresenceIdentityHook;

public:
    virtual void StartupModule() override
    {
        using namespace ::Redpoint::EOS::Identity;
        using namespace ::Redpoint::EOS::Presence;

        this->PresenceIdentityHook = MakeShared<FPresenceIdentityHook>();
        FIdentityHookRegistry::Register(this->PresenceIdentityHook.ToSharedRef(), 70000);
    };

    virtual void ShutdownModule() override
    {
        using namespace ::Redpoint::EOS::Identity;

        if (this->PresenceIdentityHook.IsValid())
        {
            FIdentityHookRegistry::Unregister(this->PresenceIdentityHook.ToSharedRef());
        }
        this->PresenceIdentityHook.Reset();
    }
#endif
};

REDPOINT_EOS_CODE_GUARD_END()

IMPLEMENT_MODULE(FRedpointEOSPresenceModule, RedpointEOSPresence);
