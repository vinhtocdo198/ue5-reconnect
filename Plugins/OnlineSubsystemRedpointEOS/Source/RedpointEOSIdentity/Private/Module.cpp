// Copyright June Rhodes. All Rights Reserved.

#include "Modules/ModuleManager.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSIdentity/Hooks/EditorCheckReadyForLoginCompletionIdentityHook.h"
#include "RedpointEOSIdentity/Hooks/EditorDefaultInstanceCheckIdentityHook.h"
#include "RedpointEOSIdentity/Hooks/LocalPlayerIdentityHook.h"
#include "RedpointEOSIdentity/IdentityHook.h"
#include "RedpointEOSIdentity/IdentityHookRegistry.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

class REDPOINTEOSIDENTITY_API FRedpointEOSIdentityModule : public FDefaultModuleImpl
{
private:
    TArray<TSharedRef<Redpoint::EOS::Identity::IIdentityHook>> IdentityHooks;

public:
    virtual void StartupModule() override
    {
        using namespace ::Redpoint::EOS::Identity;

        this->IdentityHooks.Add(MakeShared<Hooks::FEditorDefaultInstanceCheckIdentityHook>());
        this->IdentityHooks.Add(MakeShared<Hooks::FEditorCheckReadyForLoginCompletionIdentityHook>());
        this->IdentityHooks.Add(MakeShared<Hooks::FLocalPlayerIdentityHook>());
        for (const auto &Hook : this->IdentityHooks)
        {
            FIdentityHookRegistry::Register(Hook, 100000);
        }
    };

    virtual void ShutdownModule() override
    {
        using namespace ::Redpoint::EOS::Identity;

        for (const auto &Hook : this->IdentityHooks)
        {
            FIdentityHookRegistry::Unregister(Hook);
        }
        this->IdentityHooks.Empty();
    }
};

REDPOINT_EOS_CODE_GUARD_END()

IMPLEMENT_MODULE(FRedpointEOSIdentityModule, RedpointEOSIdentity);
