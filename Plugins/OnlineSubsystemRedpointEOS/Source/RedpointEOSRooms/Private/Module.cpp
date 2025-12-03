// Copyright June Rhodes. All Rights Reserved.

#include "Modules/ModuleManager.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSIdentity/IdentityHookRegistry.h"
#include "RedpointEOSRooms/Hooks/RoomRuntimePlatformIdentityHook.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

class REDPOINTEOSROOMS_API FRedpointEOSRoomsModule : public FDefaultModuleImpl
{
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
private:
    TSharedPtr<Redpoint::EOS::Identity::IIdentityHook> RoomRuntimePlatformIdentityHook;

public:
    virtual void StartupModule() override
    {
        using namespace ::Redpoint::EOS::Identity;
        using namespace ::Redpoint::EOS::Rooms::Hooks;

        this->RoomRuntimePlatformIdentityHook = MakeShared<FRoomRuntimePlatformIdentityHook>();
        FIdentityHookRegistry::Register(this->RoomRuntimePlatformIdentityHook.ToSharedRef(), 30000);
    };

    virtual void ShutdownModule() override
    {
        using namespace ::Redpoint::EOS::Identity;

        if (this->RoomRuntimePlatformIdentityHook.IsValid())
        {
            FIdentityHookRegistry::Unregister(this->RoomRuntimePlatformIdentityHook.ToSharedRef());
        }
        this->RoomRuntimePlatformIdentityHook.Reset();
    }
#endif
};

REDPOINT_EOS_CODE_GUARD_END()

IMPLEMENT_MODULE(FRedpointEOSRoomsModule, RedpointEOSRooms);
