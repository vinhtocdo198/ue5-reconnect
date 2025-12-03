// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSIdentity/IdentityHook.h"
#include "RedpointEOSPlatform/RuntimePlatformIntegration.h"
#include "RedpointEOSRooms/RoomSystemImpl.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1859153880, Redpoint::EOS::Rooms::Hooks)
{
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Platform;

class FRoomRuntimePlatformIdentityHook : public IIdentityHook, public TSharedFromThis<FRoomRuntimePlatformIdentityHook>
{
private:
    TMap<TTuple<TSharedRef<FRoomSystemImpl>, TSharedRef<IRuntimePlatformIntegration>>, FDelegateHandle>
        OnRoomAdvertisementInviteReceivedHandles;
    TMap<TTuple<TSharedRef<FRoomSystemImpl>, TSharedRef<IRuntimePlatformIntegration>>, FDelegateHandle>
        OnRoomAdvertisementInviteAcceptedHandles;

public:
    FRoomRuntimePlatformIdentityHook() = default;
    UE_NONCOPYABLE(FRoomRuntimePlatformIdentityHook);
    virtual ~FRoomRuntimePlatformIdentityHook() override = default;

    virtual FString GetHookDebugName() const override
    {
        return TEXT("RoomRuntimePlatformIdentityHook");
    }

    virtual void OnStartSystem(
        const FIdentityHookContext &InContext,
        const FIdentityUserRef &InUser,
        const FOnStartSystemComplete &InOnComplete) override;
    virtual void OnStopSystem(
        const FIdentityHookContext &InContext,
        const FIdentityUserRef &InUser,
        const FOnStopSystemComplete &InOnComplete) override;
};

}

namespace Redpoint::EOS::Rooms::Hooks
{
REDPOINT_EOS_FILE_NS_EXPORT(1859153880, Redpoint::EOS::Rooms::Hooks, FRoomRuntimePlatformIdentityHook)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()