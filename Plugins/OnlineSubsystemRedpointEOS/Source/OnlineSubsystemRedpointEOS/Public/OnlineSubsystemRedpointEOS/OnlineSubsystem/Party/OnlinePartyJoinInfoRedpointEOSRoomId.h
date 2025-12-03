// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Party/OnlinePartyJoinInfoRedpointEOS.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/RoomInvite.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2321124487, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party;

class ONLINESUBSYSTEMREDPOINTEOS_API FOnlinePartyJoinInfoRedpointEOSRoomId : public FOnlinePartyJoinInfoRedpointEOS
{
private:
    FRoomIdRef RoomId;
    FString EmptyString;
    bool bIsPrimaryParty;

public:
    FOnlinePartyJoinInfoRedpointEOSRoomId(const FRoomIdRef &InRoomId, bool bInIsPrimaryParty);
    UE_NONCOPYABLE(FOnlinePartyJoinInfoRedpointEOSRoomId);
    virtual ~FOnlinePartyJoinInfoRedpointEOSRoomId() override = default;

    virtual Requests::FJoinRoomRequest GetJoinRoomRequest(const FUniqueNetId &LocalUserId) const override;
    virtual FRoomSearchResultPtr GetRoomSearchResult() const override;
    virtual bool IsKnownToBePrimaryParty() const override;

    virtual bool IsValid() const override;
    virtual TSharedRef<const FOnlinePartyId> GetPartyId() const override;
    virtual FOnlinePartyTypeId GetPartyTypeId() const override;
    virtual FUniqueNetIdRef GetSourceUserId() const override;
    virtual const FString &GetSourceDisplayName() const override;
    virtual const FString &GetSourcePlatform() const override;
    virtual const FString &GetPlatformData() const override;
    virtual bool HasKey() const override;
    virtual bool HasPassword() const override;
    virtual bool IsAcceptingMembers() const override;
    virtual bool IsPartyOfOne() const override;
    virtual int32 GetNotAcceptingReason() const override;
    virtual const FString &GetAppId() const override;
    virtual const FString &GetBuildId() const override;
    virtual bool CanJoin() const override;
    virtual bool CanJoinWithPassword() const override;
    virtual bool CanRequestAnInvite() const override;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party
{
REDPOINT_EOS_FILE_NS_EXPORT(
    2321124487,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party,
    FOnlinePartyJoinInfoRedpointEOSRoomId)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()