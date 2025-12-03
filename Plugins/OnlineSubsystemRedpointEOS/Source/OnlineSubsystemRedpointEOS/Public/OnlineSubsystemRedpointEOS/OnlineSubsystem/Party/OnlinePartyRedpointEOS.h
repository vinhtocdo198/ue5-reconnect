// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Interfaces/OnlinePartyInterface.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Utils/HasEventRegistration.h"
#include "RedpointEOSRooms/Room.h"
#include "RedpointEOSRooms/RoomSystem.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3473859598, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party)
{
using namespace ::Redpoint::EOS::Rooms;

class ONLINESUBSYSTEMREDPOINTEOS_API FOnlinePartyRedpointEOS : public FOnlineParty,
                                                               public Redpoint::EOS::Core::Utils::IHasEventRegistration
{
private:
    TSharedRef<IRoomSystem> RoomSystem;
    FRoomRef Room;
    FDelegateHandle RoomMemberPromotedHandle;
    FDelegateHandle RoomUpdatedHandle;

    void SyncRoomState();

public:
    FOnlinePartyRedpointEOS(const TSharedRef<IRoomSystem> &InRoomSystem, const FRoomRef &InRoom);
    virtual void RegisterEvents() override;
    UE_NONCOPYABLE(FOnlinePartyRedpointEOS);
    virtual ~FOnlinePartyRedpointEOS() override;

    virtual bool CanLocalUserInvite(const FUniqueNetId &LocalUserId) const override;
    virtual bool IsJoinable() const override;
    virtual void SetState(EPartyState InState) override;
    virtual TSharedRef<const FPartyConfiguration> GetConfiguration() const override;

    FOnlinePartyDataConstPtr GetPartyData(const FName &InNamespace) const;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party
{
REDPOINT_EOS_FILE_NS_EXPORT(
    3473859598,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party,
    FOnlinePartyRedpointEOS)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()