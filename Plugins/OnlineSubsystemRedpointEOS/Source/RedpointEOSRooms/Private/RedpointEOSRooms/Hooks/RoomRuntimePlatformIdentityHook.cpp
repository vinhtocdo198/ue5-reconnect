// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Hooks/RoomRuntimePlatformIdentityHook.h"

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSCore/Module.h"
#include "RedpointEOSCore/Utils/CallContextFactory.h"
#include "RedpointEOSCore/Utils/MultiOperation.h"
#include "RedpointEOSRooms/Logging.h"
#include "RedpointEOSUserCache/ExternalUserInfo.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1859153880, Redpoint::EOS::Rooms::Hooks)
{
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::Platform::Services;
using namespace ::Redpoint::EOS::Core;
using namespace ::Redpoint::EOS::UserCache;

void FRoomRuntimePlatformIdentityHook::OnStartSystem(
    const FIdentityHookContext &InContext,
    const FIdentityUserRef &InUser,
    const FOnStartSystemComplete &InOnComplete)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Hooks);

    // Get the room system and create the call context.
    auto RoomSystem = StaticCastSharedRef<FRoomSystemImpl>(InContext.PlatformHandle->GetSystem<IRoomSystem>());
    auto CallContext = ConstructRuntimePlatformServiceCallContext(InContext.PlatformHandle, InUser->GetUserSlot());

    // Perform event registration across all runtime platform integrations.
    FMultiOperation<TSharedRef<IRuntimePlatformIntegration>, bool>::RunSP(
        this,
        FModule::GetModuleChecked().GetRuntimePlatformRegistry()->GetRuntimePlatformIntegrations(),
        [this, InContext, RoomSystem, CallContext](
            TSharedRef<IRuntimePlatformIntegration> RuntimeIntegration,
            TFunction<void(bool OutValue)> OnDone) -> bool {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Hooks);

            // Configure key for unregistering later.
            auto Key = TTuple<TSharedRef<FRoomSystemImpl>, TSharedRef<IRuntimePlatformIntegration>>(
                RoomSystem,
                RuntimeIntegration);

            // Wire up events on the runtime platform and forward to the room system.
            this->OnRoomAdvertisementInviteReceivedHandles.Add(
                Key,
                RuntimeIntegration->GetPresenceService()
                    ->OnRoomAdvertisementInviteReceived(CallContext)
                    .Add(IRuntimePlatformPresenceService::FOnRoomAdvertisementInviteReceived::FDelegate::CreateSPLambda(
                        this,
                        [RoomSystem](
                            const UE::Online::FAccountId &LocalUserId,
                            const FUserInfoPtr &SenderUser,
                            const FRoomIdRef &RoomId,
                            const FName &RoomNamespace) {
                            REDPOINT_EOS_USE_LLM_TAG(Rooms_Hooks);

                            RoomSystem->OnRoomInviteReceivedFromRuntimePlatform(
                                LocalUserId,
                                SenderUser,
                                RoomId,
                                RoomNamespace);
                        })));
            this->OnRoomAdvertisementInviteAcceptedHandles.Add(
                Key,
                RuntimeIntegration->GetPresenceService()
                    ->OnRoomAdvertisementInviteAccepted(CallContext)
                    .Add(IRuntimePlatformPresenceService::FOnRoomAdvertisementInviteAccepted::FDelegate::CreateSPLambda(
                        this,
                        [RoomSystem](
                            const UE::Online::FAccountId &LocalUserId,
                            const FUserInfoPtr &SenderUser,
                            const FRoomIdRef &RoomId,
                            const FName &RoomNamespace) {
                            REDPOINT_EOS_USE_LLM_TAG(Rooms_Hooks);

                            RoomSystem->OnRoomInviteAcceptedFromRuntimePlatform(
                                LocalUserId,
                                SenderUser,
                                RoomId,
                                RoomNamespace);
                        })));

            OnDone(true);
            return true;
        },
        [InOnComplete](TArray<bool>) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Hooks);

            InOnComplete.ExecuteIfBound(Errors::Success());
        },
        false);
}

void FRoomRuntimePlatformIdentityHook::OnStopSystem(
    const FIdentityHookContext &InContext,
    const FIdentityUserRef &InUser,
    const FOnStopSystemComplete &InOnComplete)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Hooks);

    // Get the room system and create the call context.
    auto RoomSystem = StaticCastSharedRef<FRoomSystemImpl>(InContext.PlatformHandle->GetSystem<IRoomSystem>());
    auto CallContext = ConstructRuntimePlatformServiceCallContext(InContext.PlatformHandle, InUser->GetUserSlot());

    // Perform event unregistration across all runtime platform integrations.
    FMultiOperation<TSharedRef<IRuntimePlatformIntegration>, bool>::RunSP(
        this,
        FModule::GetModuleChecked().GetRuntimePlatformRegistry()->GetRuntimePlatformIntegrations(),
        [this, InContext, InUser, RoomSystem, CallContext](
            TSharedRef<IRuntimePlatformIntegration> RuntimeIntegration,
            TFunction<void(bool OutValue)> OnDone) -> bool {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Hooks);

            // Configure key for unregistering.
            auto Key = TTuple<TSharedRef<FRoomSystemImpl>, TSharedRef<IRuntimePlatformIntegration>>(
                RoomSystem,
                RuntimeIntegration);

            // Unregister events from the runtime platform.
            auto *ReceivedEntry = this->OnRoomAdvertisementInviteReceivedHandles.Find(Key);
            auto *AcceptedEntry = this->OnRoomAdvertisementInviteAcceptedHandles.Find(Key);
            if (ReceivedEntry != nullptr)
            {
                RuntimeIntegration->GetPresenceService()
                    ->OnRoomAdvertisementInviteReceived(CallContext)
                    .Remove(*ReceivedEntry);
                this->OnRoomAdvertisementInviteReceivedHandles.Remove(Key);
            }
            if (AcceptedEntry != nullptr)
            {
                RuntimeIntegration->GetPresenceService()
                    ->OnRoomAdvertisementInviteAccepted(CallContext)
                    .Remove(*AcceptedEntry);
                this->OnRoomAdvertisementInviteAcceptedHandles.Remove(Key);
            }

            // We're immediately done.
            OnDone(true);
            return true;
        },
        [InOnComplete](TArray<bool>) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Hooks);

            InOnComplete.ExecuteIfBound(Errors::Success());
        },
        false);
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()