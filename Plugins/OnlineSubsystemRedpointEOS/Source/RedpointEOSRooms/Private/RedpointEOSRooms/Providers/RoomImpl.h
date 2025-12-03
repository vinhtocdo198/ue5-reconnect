// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/RoomFeature.h"
#include "RedpointEOSRooms/Providers/RoomMemberImpl.h"
#include "RedpointEOSRooms/Room.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(3254663568, Redpoint::EOS::Rooms::Providers, FRoomProvider)

namespace REDPOINT_EOS_FILE_NS_ID(3293798267, Redpoint::EOS::Rooms::Providers)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Providers;
using namespace ::Redpoint::EOS::Rooms::Features;

class REDPOINTEOSROOMS_API FRoomImpl : public IRoom
{
    friend FRoomProvider;

protected:
    FRoomUserId BoundForLocalUser;
    FRoomIdRef RoomId;
    FRoomOwnerPtr RoomOwner;
    FName RoomNamespace;
    uint32 RoomMaxMembers;
    TMap<FName, TSharedRef<IRoomFeature>> RoomFeatures;
    TMap<FString, FRoomAttribute> RoomAttributes;
    TArray<FRoomMemberRef> RoomMembers;
    TMap<FRoomUserId, FDateTime> RoomUnreadyMembersFirstSeenTimestamp;
    TSet<FRoomUserId> RoomMembersBeingAutomaticallyKicked;
    FOnRoomMemberJoined OnRoomMemberJoinedValue;
    FOnRoomMemberLeft OnRoomMemberLeftValue;
    FOnRoomMemberUpdated OnRoomMemberUpdatedValue;
    FOnRoomMemberPromoted OnRoomMemberPromotedValue;

public:
    FRoomImpl(const FRoomUserId &InBoundForLocalUser, const FRoomIdRef &InRoomId);
    UE_NONCOPYABLE(FRoomImpl);
    virtual ~FRoomImpl() override = default;

    virtual const FRoomIdRef &GetRoomId() const override;
    virtual FName GetRoomNamespace() const override;
    virtual FRoomOwnerRef GetRoomOwner() const override;
    virtual uint32 GetRoomMaxMembers() const override;
    virtual const TMap<FString, FRoomAttribute> &GetRoomAttributes() const override;
    virtual TSet<FName> GetRoomFeatureNames() const override;
    virtual TSharedPtr<Features::IRoomFeature> GetRoomFeatureBase(const FName &FeatureName) const override;
    virtual const TArray<FRoomMemberRef> &GetMembers() const override;
    virtual FOnRoomMemberJoined &OnRoomMemberJoined() override;
    virtual FOnRoomMemberLeft &OnRoomMemberLeft() override;
    virtual FOnRoomMemberUpdated &OnRoomMemberUpdated() override;
    virtual FOnRoomMemberPromoted &OnRoomMemberPromoted() override;

    virtual FRoomUserId GetBoundForLocalUser() const override;

    /**
     * Called by the room system when it's time to remove a room. By removing all
     * the room features from the room, this causes the shared pointers to features
     * to drop to 0 so they are destroyed, which in turn releases the shared pointers
     * that features store to point back to the room.
     */
    void RemoveAllRoomFeatures();
};

}

namespace Redpoint::EOS::Rooms::Providers
{
REDPOINT_EOS_FILE_NS_EXPORT(3293798267, Redpoint::EOS::Rooms::Providers, FRoomImpl)
}

REDPOINT_EOS_CODE_GUARD_END()