// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Interfaces/OnlinePartyInterface.h"
#include "OnlineSubsystemRedpointEOS/Shared/OnlineUserEOS.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Utils/HasEventRegistration.h"
#include "RedpointEOSRooms/Room.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(621763951, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party)
{
using namespace ::Redpoint::EOS::Rooms;

class ONLINESUBSYSTEMREDPOINTEOS_API FOnlinePartyMemberRedpointEOS
    : public FOnlinePartyMember,
      public Redpoint::EOS::Core::Utils::IHasEventRegistration
{
private:
    FRoomRef Room;
    FRoomMemberRef RoomMember;
    FRoomUserId MemberId;
    TSharedRef<FOnlineUserEOS> UserRedpointEOS;

    static TSharedRef<FOnlineUserEOS> GetOnlineUserEOSInstance(
        TSharedRef<FEOSUserFactory> InUserFactory,
        TSharedRef<const FUniqueNetIdEOS> InLocalUserId,
        FRoomUserId InMemberId);

public:
    FOnlinePartyMemberRedpointEOS(
        TSharedRef<FEOSUserFactory> InUserFactory,
        TSharedRef<const FUniqueNetIdEOS> InLocalUserId,
        FRoomRef InRoom,
        FRoomMemberRef InRoomMember,
        FRoomUserId InMemberId);
    virtual void RegisterEvents() override;
    UE_NONCOPYABLE(FOnlinePartyMemberRedpointEOS);
    virtual ~FOnlinePartyMemberRedpointEOS() override;

    virtual FUniqueNetIdRef GetUserId() const override;
    virtual FString GetRealName() const override;
    virtual FString GetDisplayName(const FString &Platform = FString()) const override;
    virtual bool GetUserAttribute(const FString &AttrName, FString &OutAttrValue) const override;
    virtual bool SetUserLocalAttribute(const FString &AttrName, const FString &InAttrValue) override;

    FOnlinePartyDataConstPtr GetPartyMemberData(const FName &InNamespace) const;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party
{
REDPOINT_EOS_FILE_NS_EXPORT(
    621763951,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party,
    FOnlinePartyMemberRedpointEOS)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()