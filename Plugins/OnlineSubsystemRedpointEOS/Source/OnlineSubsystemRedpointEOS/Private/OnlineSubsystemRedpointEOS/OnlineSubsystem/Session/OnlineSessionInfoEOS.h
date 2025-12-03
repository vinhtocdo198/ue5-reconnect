// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Session/UniqueNetIdEOSSession.h"
#include "OnlineSubsystemTypes.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1903314576, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Session)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Session;

class FOnlineSessionInfoEOS : public FOnlineSessionInfo
{
private:
    TSharedPtr<FPlatformInstance> PlatformHandle;
    TSharedPtr<FUniqueNetIdEOSSession> Id;
    EOS_HSessionDetails Handle;
    EOS_SessionDetails_Info *Info;

public:
    FOnlineSessionInfoEOS(
        const FPlatformHandle &InPlatformHandle,
        EOS_HSessionDetails InHandle,
        EOS_SessionDetails_Info *InInfo);
    FOnlineSessionInfoEOS(const TSharedRef<FUniqueNetIdEOSSession> &InId);
    UE_NONCOPYABLE(FOnlineSessionInfoEOS);
    virtual ~FOnlineSessionInfoEOS() override;

    EOS_HSessionDetails GetHandle() const;
    bool GetResolvedConnectString(FString &OutConnectionString, FName PortType) const;

    virtual const FUniqueNetId &GetSessionId() const override;
    virtual const uint8 *GetBytes() const override;
    virtual int32 GetSize() const override;
    virtual bool IsValid() const override;
    virtual FString ToString() const override;
    virtual FString ToDebugString() const override;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Session
{
REDPOINT_EOS_FILE_NS_EXPORT(
    1903314576,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Session,
    FOnlineSessionInfoEOS)
}

REDPOINT_EOS_CODE_GUARD_END()