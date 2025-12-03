// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Session/UniqueNetIdEOSSession.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSIdentity/IdentityUser.h"
#include "RedpointEOSPlatform/Services/RuntimePlatformPresenceService.h"
#include "RedpointEOSRooms/RoomId.h"
#include "RedpointEOSUserCache/UserInfo.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2530062524, Redpoint::EOS::OnlineSubsystemRedpointEOS::Legacy)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Session;
using namespace ::Redpoint::EOS::Platform::Services;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::UserCache;

class FSyntheticSessionManager : public IPlatformInstanceSystem, public TSharedFromThis<FSyntheticSessionManager>
{
private:
    FPlatformHandle PlatformHandle;
    TMap<int32, TMap<TSharedRef<IRuntimePlatformPresenceService>, FDelegateHandle>>
        OnRoomAdvertisementInviteReceivedDelegateHandles;
    TMap<int32, TMap<TSharedRef<IRuntimePlatformPresenceService>, FDelegateHandle>>
        OnRoomAdvertisementInviteAcceptedDelegateHandles;

public:
    static FName GetSystemName();
    static TSharedRef<FSyntheticSessionManager> ConstructSystem(
        FPlatformHandle InPlatformHandle,
        FPlatformInstanceEnvironmentRef InPlatformInstanceEnvironment);

public:
    FSyntheticSessionManager(const FPlatformHandle &InPlatformHandle);
    UE_NONCOPYABLE(FSyntheticSessionManager);
    virtual ~FSyntheticSessionManager() override = default;

private:
    void OnRoomAdvertisementInviteReceived(
        const UE::Online::FAccountId &LocalUserId,
        const FUserInfoPtr &SenderUser,
        const FRoomIdRef &RoomId,
        const FName &RoomNamespace);
    void OnRoomAdvertisementInviteAccepted(
        const UE::Online::FAccountId &LocalUserId,
        const FUserInfoPtr &SenderUser,
        const FRoomIdRef &RoomId,
        const FName &RoomNamespace);

public:
    typedef TDelegate<void(const FError &Error)> FOnComplete;

    void OnStartSystem(const FIdentityUserRef &User, const FSimpleDelegate &OnComplete);
    void OnStopSystem(const FIdentityUserRef &User, const FSimpleDelegate &OnComplete);

    void UpsertSyntheticSession(
        const TSharedPtr<const FUniqueNetIdEOSSession> &SessionId,
        int32 MaxPlayers,
        const FOnComplete &OnComplete);
    void DeleteSyntheticSession(
        const TSharedPtr<const FUniqueNetIdEOSSession> &SessionId,
        const FOnComplete &OnComplete);
    void SendInvitationToSession(
        int32 LocalUserNum,
        const TSharedPtr<const FUniqueNetIdEOSSession> &SessionId,
        const TSharedPtr<const FUniqueNetId> &RecipientId,
        const FOnComplete &Delegate);
    bool OpenSessionInvitationUI(int32 LocalUserNum, const TSharedPtr<const FUniqueNetIdEOSSession> &SessionId);
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::Legacy
{
REDPOINT_EOS_FILE_NS_EXPORT(2530062524, Redpoint::EOS::OnlineSubsystemRedpointEOS::Legacy, FSyntheticSessionManager)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()