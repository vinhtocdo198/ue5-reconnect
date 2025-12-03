// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPresence/PresenceSystem.h"

#include "RedpointEOSCore/Exec/ExecSystem.h"
#include "RedpointEOSCore/Utils/MakeSharedWithEvents.h"
#include "RedpointEOSPresence/PresenceSystemImpl.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2615688832, Redpoint::EOS::Presence)
{
using namespace ::Redpoint::EOS::Presence;
using namespace ::Redpoint::EOS::Core::Exec;
using namespace ::Redpoint::EOS::Core::Utils;

FName IPresenceSystem::GetSystemName()
{
    return FName(TEXT("IPresenceSystem"));
}

TSharedRef<IPresenceSystem> IPresenceSystem::ConstructSystem(
    FPlatformHandle InPlatformHandle,
    FPlatformInstanceEnvironmentRef InPlatformInstanceEnvironment)
{
    TSharedRef<FPresenceSystemImpl> PresenceSystem = MakeSharedWithEvents<FPresenceSystemImpl>(InPlatformHandle);
    InPlatformHandle->GetSystem<IExecSystem>()->Register(TEXT("Presence"), PresenceSystem);

    TSharedRef<FDelegateHandle> OnShutdownHandle = MakeShared<FDelegateHandle>();
    *OnShutdownHandle =
        InPlatformHandle->OnShutdown().Add(FOnPlatformInstanceEvent::FDelegate::CreateSPLambda(
            PresenceSystem,
            [OnShutdownHandle, PresenceSystem](FPlatformInstance &Handle) {
                PresenceSystem->UnregisterEvents();
                Handle.OnShutdown().Remove(*OnShutdownHandle);
            }));
    return PresenceSystem;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()