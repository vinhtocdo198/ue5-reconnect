// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/RoomAttribute.h"
#include "RedpointEOSRooms/RoomId.h"
#include "RedpointEOSRooms/RoomLeaveReason.h"
#include "RedpointEOSRooms/RoomMember.h"
#include "RedpointEOSRooms/RoomMemberUpdateInfo.h"
#include "RedpointEOSRooms/RoomOwner.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(1648834184, Redpoint::EOS::Rooms::Features, IRoomFeature);
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_STRUCT(43897010, Redpoint::EOS::Rooms, FRoomEventContext);

namespace REDPOINT_EOS_FILE_NS_ID(3320738808, Redpoint::EOS::Rooms)
{
using namespace ::Redpoint::EOS::Rooms;

class REDPOINTEOSROOMS_API IRoom
{
public:
    IRoom() = default;
    UE_NONCOPYABLE(IRoom);
    virtual ~IRoom() = default;

    /**
     * Returns the ID of the room.
     */
    virtual const FRoomIdRef &GetRoomId() const = 0;

    /**
     * Returns the namespace of the room.
     */
    virtual FName GetRoomNamespace() const = 0;

    /**
     * Returns the owner of the room.
     */
    virtual FRoomOwnerRef GetRoomOwner() const = 0;

    /**
     * Returns the maximum number of members in the room.
     */
    virtual uint32 GetRoomMaxMembers() const = 0;

    /**
     * Returns the room attributes.
     */
    virtual const TMap<FString, FRoomAttribute> &GetRoomAttributes() const = 0;

    /**
     * Returns a list of feature names that this room has.
     */
    virtual TSet<FName> GetRoomFeatureNames() const = 0;

    /**
     * Returns the base feature if the room has the feature.
     */
    virtual TSharedPtr<Features::IRoomFeature> GetRoomFeatureBase(const FName &FeatureName) const = 0;

    /**
     * Returns whether the room has the specified feature.
     */
    template <typename TRoomFeature> bool HasRoomFeature() const
    {
        static_assert(
            std::is_convertible_v<TRoomFeature *, Features::IRoomFeature *>,
            "TRoomFeature must inherit from IRoomFeature.");
        return this->GetRoomFeatureBase(TRoomFeature::GetFeatureName()).IsValid();
    }

    /**
     * Returns the feature request if the room has the feature.
     */
    template <typename TRoomFeature> TSharedPtr<TRoomFeature> GetRoomFeature() const
    {
        static_assert(
            std::is_convertible_v<TRoomFeature *, Features::IRoomFeature *>,
            "TRoomFeature must inherit from IRoomFeature.");
        return StaticCastSharedPtr<TRoomFeature>(this->GetRoomFeatureBase(TRoomFeature::GetFeatureName()));
    }

    /**
     * Returns a feature request that the room has, or asserts if the room doesn't have the feature.
     */
    template <typename TRoomFeature> TSharedRef<TRoomFeature> GetRoomFeatureChecked() const
    {
        static_assert(
            std::is_convertible_v<TRoomFeature *, Features::IRoomFeature *>,
            "TRoomFeature must inherit from IRoomFeature.");
        return GetRoomFeature<TRoomFeature>().ToSharedRef();
    }

    typedef TMulticastDelegate<
        void(const FRoomEventContext &EventContext, const TSharedRef<IRoom> &Room, FRoomUserId TargetUser)>
        FOnRoomMemberJoined;
    typedef TMulticastDelegate<void(
        const FRoomEventContext &EventContext,
        const TSharedRef<IRoom> &Room,
        FRoomUserId TargetUser,
        ERoomLeaveReason Reason)>
        FOnRoomMemberLeft;
    typedef TMulticastDelegate<void(
        const FRoomEventContext &EventContext,
        const TSharedRef<IRoom> &Room,
        FRoomUserId TargetUser,
        const FRoomMemberUpdateInfo &RoomMemberUpdateInfo)>
        FOnRoomMemberUpdated;
    typedef TMulticastDelegate<
        void(const FRoomEventContext &EventContext, const TSharedRef<IRoom> &Room, FRoomUserId TargetUser)>
        FOnRoomMemberPromoted;

    /**
     * Returns the current list of members in the room.
     */
    virtual const TArray<FRoomMemberRef> &GetMembers() const = 0;

    /**
     * The event which fires when a user (including the local user) joins the room.
     */
    virtual FOnRoomMemberJoined &OnRoomMemberJoined() = 0;

    /**
     * The event which fires when a user (including the local user) leaves the room.
     */
    virtual FOnRoomMemberLeft &OnRoomMemberLeft() = 0;

    /**
     * The event which fires when the data associated with a member of the room is updated.
     */
    virtual FOnRoomMemberUpdated &OnRoomMemberUpdated() = 0;

    /**
     * The event which fires when a member of the room is promoted to own the room.
     */
    virtual FOnRoomMemberPromoted &OnRoomMemberPromoted() = 0;

    /**
     * Returns the local user that this room object is bound for. That is, the actor that joined or created this room
     * and the one that performs actions when this room is updated or changed.
     *
     * The room system can track the same EOS SDK lobby/session in different room objects when you have multiple local
     * users who have both joined the same room.
     */
    virtual FRoomUserId GetBoundForLocalUser() const = 0;
};

typedef TSharedRef<IRoom> FRoomRef;
typedef TSharedPtr<IRoom> FRoomPtr;

}

namespace Redpoint::EOS::Rooms
{
REDPOINT_EOS_FILE_NS_EXPORT(3320738808, Redpoint::EOS::Rooms, IRoom)
REDPOINT_EOS_FILE_NS_EXPORT(3320738808, Redpoint::EOS::Rooms, FRoomRef)
REDPOINT_EOS_FILE_NS_EXPORT(3320738808, Redpoint::EOS::Rooms, FRoomPtr)
}

REDPOINT_EOS_CODE_GUARD_END()