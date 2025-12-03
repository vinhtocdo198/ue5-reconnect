// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Online/CoreOnline.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3267991839, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Session)
{

class ONLINESUBSYSTEMREDPOINTEOS_API FUniqueNetIdEOSSession : public FUniqueNetId
{
private:
    FString SessionId;
    const char *DataBytes;
    int32 DataBytesSize;

public:
    FUniqueNetIdEOSSession() = delete;
    FUniqueNetIdEOSSession(const FString &InSessionId);
    FUniqueNetIdEOSSession(const char *InSessionId);
    // This operation isn't safe, because the copy won't have been created through TSharedPtr, and thus
    // you won't be able to call AsShared on it. Prevent any accidental copies by removing the copy constructor.
    UE_NONCOPYABLE(FUniqueNetIdEOSSession);
    virtual ~FUniqueNetIdEOSSession() override;

    virtual bool Compare(const FUniqueNetId &Other) const override;
    virtual FString GetSessionId() const;
    virtual FName GetType() const override;
    virtual const uint8 *GetBytes() const override;
    virtual int32 GetSize() const override;
    virtual bool IsValid() const override;
    virtual FString ToString() const override;
    virtual FString ToDebugString() const override;

    friend uint32 GetTypeHash(const FUniqueNetIdEOSSession &A);
    friend FArchive &operator<<(FArchive &Ar, FUniqueNetIdEOSSession &OtherId);
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Session
{
REDPOINT_EOS_FILE_NS_EXPORT(
    3267991839,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Session,
    FUniqueNetIdEOSSession)
}

REDPOINT_EOS_CODE_GUARD_END()