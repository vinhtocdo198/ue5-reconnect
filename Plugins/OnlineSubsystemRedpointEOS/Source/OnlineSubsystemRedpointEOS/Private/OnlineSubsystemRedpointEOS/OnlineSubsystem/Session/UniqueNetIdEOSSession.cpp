// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Session/UniqueNetIdEOSSession.h"

#include "Containers/UnrealString.h"
#include "OnlineSubsystemRedpointEOS/Shared/EOSCommon.h"
#include "RedpointEOSCore/Legacy/EOSString.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3267991839, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Session)
{

FUniqueNetIdEOSSession::FUniqueNetIdEOSSession(const FString &InSessionId)
    : SessionId(InSessionId)
    , DataBytes(nullptr)
    , DataBytesSize(0)
{
    verify(
        EOSString_SessionModification_SessionId::AllocateToCharBuffer(
            InSessionId,
            this->DataBytes,
            this->DataBytesSize) == EOS_EResult::EOS_Success);
}

FUniqueNetIdEOSSession::FUniqueNetIdEOSSession(const char *InSessionId)
    : FUniqueNetIdEOSSession(FString(ANSI_TO_TCHAR(InSessionId)))
{
}

bool FUniqueNetIdEOSSession::Compare(const FUniqueNetId &Other) const
{
    if (Other.GetType() != GetType())
    {
        return false;
    }

    return (GetSize() == Other.GetSize()) &&
           (FMemory::Memcmp(GetBytes(), Other.GetBytes(), static_cast<SIZE_T>(GetSize())) == 0);
}

FUniqueNetIdEOSSession::~FUniqueNetIdEOSSession()
{
    EOSString_SessionModification_SessionId::FreeFromCharBuffer(this->DataBytes);
}

FString FUniqueNetIdEOSSession::GetSessionId() const
{
    return this->SessionId;
}

FName FUniqueNetIdEOSSession::GetType() const
{
    return REDPOINT_EOS_SUBSYSTEM_SESSION;
}

const uint8 *FUniqueNetIdEOSSession::GetBytes() const
{
    return reinterpret_cast<const uint8 *>(this->DataBytes);
}

int32 FUniqueNetIdEOSSession::GetSize() const
{
    return this->DataBytesSize;
}

bool FUniqueNetIdEOSSession::IsValid() const
{
    return true;
}

FString FUniqueNetIdEOSSession::ToString() const
{
    return this->SessionId;
}

FString FUniqueNetIdEOSSession::ToDebugString() const
{
    return this->SessionId;
}

uint32 GetTypeHash(const FUniqueNetIdEOSSession &A)
{
    return GetTypeHashHelper(A.ToString());
}

FArchive &operator<<(FArchive &Ar, FUniqueNetIdEOSSession &OtherId)
{
    auto IdSer = OtherId.ToString();
    return Ar << IdSer;
}

}

REDPOINT_EOS_CODE_GUARD_END()