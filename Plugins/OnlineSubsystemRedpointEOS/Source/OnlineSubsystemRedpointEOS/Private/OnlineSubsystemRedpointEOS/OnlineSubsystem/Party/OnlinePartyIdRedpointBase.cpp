// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Party/OnlinePartyIdRedpointBase.h"

#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Party/OnlinePartyIdRedpointEOS.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2707984723, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party)
{
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party;

FOnlinePartyIdRedpointBase::FOnlinePartyIdRedpointBase(const FRoomIdPtr &InRoomId)
    : RoomId(InRoomId)
    , AnsiData(nullptr)
    , AnsiDataLen(0)
{
    if (InRoomId.IsValid())
    {
        auto AnsiPtr = StringCast<ANSICHAR>(*InRoomId->GetRoomId());
        this->AnsiDataLen = AnsiPtr.Length() + 1;
        this->AnsiData = (uint8 *)FMemory::MallocZeroed(static_cast<SIZE_T>(AnsiDataLen));
        FMemory::Memcpy((void *)AnsiData, (void *)AnsiPtr.Get(), static_cast<SIZE_T>(AnsiPtr.Length()));
    }
    else
    {
        this->AnsiDataLen = 0;
        this->AnsiData = nullptr;
    }
}

FOnlinePartyIdRedpointBase::~FOnlinePartyIdRedpointBase()
{
    if (AnsiData != nullptr)
    {
        FMemory::Free((void *)AnsiData);
        AnsiData = nullptr;
    }
}

TSharedRef<const FOnlinePartyIdRedpointBase> FOnlinePartyIdRedpointBase::Create(const FRoomIdPtr &InRoomId)
{
    if (InRoomId.IsValid())
    {
        return MakeShared<FOnlinePartyIdRedpointEOS>(InRoomId.ToSharedRef());
    }
    else
    {
        return MakeShared<FOnlinePartyIdRedpointEOS>();
    }
}

const uint8 *FOnlinePartyIdRedpointBase::GetBytes() const
{
    return this->AnsiData;
}

int32 FOnlinePartyIdRedpointBase::GetSize() const
{
    return this->AnsiDataLen;
}

bool FOnlinePartyIdRedpointBase::IsValid() const
{
    return this->RoomId.IsValid();
}

FString FOnlinePartyIdRedpointBase::ToString() const
{
    if (this->RoomId.IsValid())
    {
        // @note: External code relies on 'ToString()' being only the lobby ID without the "Lobby:" prefix.
        return this->RoomId->GetRoomId();
    }
    else
    {
        return TEXT("");
    }
}

FString FOnlinePartyIdRedpointBase::ToDebugString() const
{
    if (this->RoomId.IsValid())
    {
        return this->RoomId->ToString();
    }
    else
    {
        return TEXT("(invalid)");
    }
}

FRoomIdPtr FOnlinePartyIdRedpointBase::GetRoomId() const
{
    return this->RoomId;
}

FRoomIdRef FOnlinePartyIdRedpointBase::GetRoomIdChecked() const
{
    checkf(
        this->RoomId.IsValid(),
        TEXT("%s is not valid to call on an invalid party ID."),
        ANSI_TO_TCHAR(__FUNCTION__));
    return this->RoomId.ToSharedRef();
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()