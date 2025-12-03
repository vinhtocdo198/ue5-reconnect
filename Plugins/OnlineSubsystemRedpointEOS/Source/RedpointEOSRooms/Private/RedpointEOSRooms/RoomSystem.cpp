// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/RoomSystem.h"

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSCore/Exec/ExecSystem.h"
#include "RedpointEOSCore/Utils/MakeSharedWithEvents.h"
#include "RedpointEOSRooms/RoomSystemImpl.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(195853503, Redpoint::EOS::Rooms)
{
using namespace ::Redpoint::EOS::Core::Exec;
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::Rooms;

FName IRoomSystem::GetSystemName()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    return FName(TEXT("IRoomSystem"));
}

TSharedRef<IRoomSystem> IRoomSystem::ConstructSystem(
    FPlatformHandle InPlatformHandle,
    FPlatformInstanceEnvironmentRef InPlatformInstanceEnvironment)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    TSharedRef<FRoomSystemImpl> RoomSystem = MakeSharedWithEvents<FRoomSystemImpl>(InPlatformHandle);
    InPlatformHandle->GetSystem<IExecSystem>()->Register(TEXT("Rooms"), RoomSystem);

    TSharedRef<FDelegateHandle> OnAfterTickHandle = MakeShared<FDelegateHandle>();
    TSharedRef<FDelegateHandle> OnShutdownHandle = MakeShared<FDelegateHandle>();
    *OnAfterTickHandle = InPlatformHandle->OnAfterTick().Add(
        FOnPlatformInstanceEvent::FDelegate::CreateSPLambda(RoomSystem, [RoomSystem](FPlatformInstance &Handle) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms);

            RoomSystem->SynchroniseRoomsAfterPlatformTick();
        }));
    *OnShutdownHandle = InPlatformHandle->OnShutdown().Add(FOnPlatformInstanceEvent::FDelegate::CreateSPLambda(
        RoomSystem,
        [OnAfterTickHandle, OnShutdownHandle](FPlatformInstance &Handle) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms);

            Handle.OnAfterTick().Remove(*OnAfterTickHandle);
            Handle.OnShutdown().Remove(*OnShutdownHandle);
        }));
    return RoomSystem;
}

}

REDPOINT_EOS_CODE_GUARD_END()