// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Interfaces/OnlinePartyInterface.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/RoomId.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2707984723, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party)
{
using namespace ::Redpoint::EOS::Rooms;

class ONLINESUBSYSTEMREDPOINTEOS_API FOnlinePartyIdRedpointBase : public FOnlinePartyId
{
private:
    FRoomIdPtr RoomId;
    const uint8 *AnsiData;
    int32 AnsiDataLen;

protected:
    FOnlinePartyIdRedpointBase(const FRoomIdPtr &InRoomId);
    UE_NONCOPYABLE(FOnlinePartyIdRedpointBase);
    virtual ~FOnlinePartyIdRedpointBase() override;

public:
    static TSharedRef<const FOnlinePartyIdRedpointBase> Create(const FRoomIdPtr &InRoomId);

    virtual const uint8 *GetBytes() const override;
    virtual int32 GetSize() const override;
    virtual bool IsValid() const override;
    virtual FString ToString() const override;
    virtual FString ToDebugString() const override;

    FRoomIdPtr GetRoomId() const;
    FRoomIdRef GetRoomIdChecked() const;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party
{
REDPOINT_EOS_FILE_NS_EXPORT(
    2707984723,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party,
    FOnlinePartyIdRedpointBase)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()