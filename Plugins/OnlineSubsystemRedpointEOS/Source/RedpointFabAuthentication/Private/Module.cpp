// Copyright June Rhodes. All Rights Reserved.

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "RedpointFabAuthentication/FabRedpointSessionProvider.h"

class REDPOINTFABAUTHENTICATION_API FRedpointFabAuthenticationModule : public IModuleInterface
{
private:
    TSharedPtr<::Redpoint::Fab::Authentication::FFabRedpointSessionProvider> SessionProvider;

public:
    virtual void StartupModule() override
    {
        this->SessionProvider = MakeShared<::Redpoint::Fab::Authentication::FFabRedpointSessionProvider>();
        IModularFeatures::Get().RegisterModularFeature(FName("FabSessionProvider"), this->SessionProvider.Get());
    }

    virtual void ShutdownModule() override
    {
        if (this->SessionProvider.IsValid())
        {
            IModularFeatures::Get().UnregisterModularFeature(FName("FabSessionProvider"), this->SessionProvider.Get());
            this->SessionProvider.Reset();
        }
    }
};

IMPLEMENT_MODULE(FRedpointFabAuthenticationModule, RedpointFabAuthentication)