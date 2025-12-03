// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSCore/Module.h"

#include "Online/CoreOnline.h"
#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSConfig/DependentModuleLoader.h"
#include "RedpointEOSCore/Id/OnlineAccountIdRegistryRedpointEOSImpl.h"
#include "RedpointEOSCore/InstancePool.h"
#include "RedpointEOSCore/InstancePoolImpl.h"
#include "RedpointEOSCore/SDKGlobal.h"
#include "RedpointEOSCore/SDKLogging.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3273657706, Redpoint::EOS::Core)
{
using namespace UE::Online;
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Core;
using namespace ::Redpoint::EOS::Platform;

FModule::FModule()
    : RuntimePlatformRegistry(MakeShared<FRuntimePlatformRegistry>())
{
}

void FModule::StartupModule()
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    FOnlineIdRegistryRegistry::Get().RegisterAccountIdRegistry(
        EOnlineServices::Epic,
        &FOnlineAccountIdRegistryRedpointEOSImpl::Instance(),
        1000);
}

void FModule::ShutdownModule()
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    FOnlineIdRegistryRegistry::Get().UnregisterAccountIdRegistry(EOnlineServices::Epic, 1000);

    if (this->RuntimePlatformRegistry->IsRuntimePlatformValid())
    {
        // @note: Forcibly shuts down all handles and marks
        // them as destroyed so further API calls in Redpoint::EOS::API
        // will fail with an error code instead of trying to call
        // unloaded methods.
        ((FInstancePoolImpl &)IInstancePool::Pool()).ShutdownAll();

        this->RuntimePlatformRegistry->UnloadRuntimePlatform();
    }

#if WITH_EDITOR
    FSDKLogging::EditorLogHandler.Unbind();
#endif

    FSDKGlobal::Shutdown();
}

TSharedRef<IRuntimePlatform> FModule::GetRuntimePlatform() const
{
    return this->RuntimePlatformRegistry->GetRuntimePlatform();
}

void FModule::SetRuntimePlatform(const TSharedRef<IRuntimePlatform> &InRuntimePlatform)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    this->RuntimePlatformRegistry->SetRuntimePlatform(InRuntimePlatform);
}

TSharedRef<FRuntimePlatformRegistry> FModule::GetRuntimePlatformRegistry() const
{
    return this->RuntimePlatformRegistry;
}

#if WITH_EDITOR
void FModule::SetLogHandler(const FOnLogForwardedForEditor &InEditorLogForwardingHandler)
{
    FSDKLogging::EditorLogHandler = InEditorLogForwardingHandler;
}
#endif

}

REDPOINT_EOS_CODE_GUARD_END()

IMPLEMENT_MODULE(REDPOINT_EOS_FILE_NS_ID(3273657706, Redpoint::EOS::Core)::FModule, RedpointEOSCore);