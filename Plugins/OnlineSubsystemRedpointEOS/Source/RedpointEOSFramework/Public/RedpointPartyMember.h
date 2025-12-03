// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Room.h"
#include "RedpointEOSRooms/RoomEventContext.h"
#include "RedpointEOSRooms/RoomJoinReason.h"
#include "RedpointEOSRooms/RoomLeaveReason.h"
#include "RedpointEOSRooms/RoomUpdateInfo.h"
#include "RedpointPartyMemberInfo.h"

#include "RedpointPartyMember.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

/**
 * This actor looks at the party members of the primary party that the local user is a member of, and lets you
 * render each party member based on their configuration. You should subclass this actor, add the components you
 * need for rendering a party member (such as a skeletal mesh), and handle for the PartyMemberChanged event.
 */
UCLASS(BlueprintType, meta = (Category = "Redpoint EOS Online Framework", DisplayName = "Redpoint Party Member"))
class REDPOINTEOSFRAMEWORK_API ARedpointPartyMember : public AActor
{
    GENERATED_BODY()

private:
    FDelegateHandle OnRoomJoinedHandle;
    FDelegateHandle OnRoomLeftHandle;
    FDelegateHandle OnRoomMemberJoinedHandle;
    FDelegateHandle OnRoomMemberUpdatedHandle;
    FDelegateHandle OnRoomMemberPromotedHandle;
    FDelegateHandle OnRoomMemberLeftHandle;
    ::Redpoint::EOS::Rooms::FRoomPtr CurrentPartyRoom;

public:
    /**
     * The "slot" in the party that this actor should render for. The first member in the party will be in slot 0. You
     * should add multiple copies of this actor to your map, each with a different slot to represent the different party
     * members.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Redpoint EOS Online Framework", meta = (ClampMin = 0))
    int32 PartyMemberSlot = 0;

    /**
     * This event is fired by the Redpoint EOS plugin when the party member changes for the party slot this actor is
     * configured for.
     *
     * This event may be called even if there are no actual changes.
     */
    UFUNCTION(BlueprintNativeEvent, Category = "Redpoint EOS Online Framework")
    void PartyMemberChanged(const FRedpointPartyMemberInfo &PartyMember);

    /**
     * The native implementation of PartyMemberChanged if you want to override it in C++.
     */
    virtual void NativePartyMemberChanged(const FRedpointPartyMemberInfo &PartyMember);

    virtual void BeginPlay() override;
    virtual void BeginDestroy() override;

private:
    void OnRoomJoined(
        const ::Redpoint::EOS::Rooms::FRoomEventContext &EventContext,
        const ::Redpoint::EOS::Rooms::FRoomRef &Room,
        ::Redpoint::EOS::Rooms::ERoomJoinReason Reason);
    void OnRoomLeft(
        const ::Redpoint::EOS::Rooms::FRoomEventContext &EventContext,
        const ::Redpoint::EOS::Rooms::FRoomRef &Room,
        ::Redpoint::EOS::Rooms::ERoomLeaveReason Reason);

    void OnRoomMemberJoined(
        const ::Redpoint::EOS::Rooms::FRoomEventContext &EventContext,
        const TSharedRef<::Redpoint::EOS::Rooms::IRoom> &Room,
        ::Redpoint::EOS::Rooms::FRoomUserId TargetUser);
    void OnRoomMemberUpdated(
        const ::Redpoint::EOS::Rooms::FRoomEventContext &EventContext,
        const TSharedRef<::Redpoint::EOS::Rooms::IRoom> &Room,
        ::Redpoint::EOS::Rooms::FRoomUserId TargetUser,
        const ::Redpoint::EOS::Rooms::FRoomMemberUpdateInfo &RoomMemberUpdateInfo);
    void OnRoomMemberPromoted(
        const ::Redpoint::EOS::Rooms::FRoomEventContext &EventContext,
        const TSharedRef<::Redpoint::EOS::Rooms::IRoom> &Room,
        ::Redpoint::EOS::Rooms::FRoomUserId TargetUser);
    void OnRoomMemberLeft(
        const ::Redpoint::EOS::Rooms::FRoomEventContext &EventContext,
        const TSharedRef<::Redpoint::EOS::Rooms::IRoom> &Room,
        ::Redpoint::EOS::Rooms::FRoomUserId TargetUser,
        ::Redpoint::EOS::Rooms::ERoomLeaveReason Reason);

    void FirePartyMemberChanged();
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()