// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Party/OnlinePartyIdRedpointBase.h"
#include "OnlineSubsystemRedpointEOS/VoiceChat/EOSVoiceManagerDeviceList.h"
#include "OnlineSubsystemRedpointEOS/VoiceChat/EOSVoiceManagerLocalUser.h"
#include "RedpointEOSAPI/Lobby/NotifyLobbyMemberStatusReceived.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSConfig/Config.h"
#include "RedpointEOSCore/Id/UniqueNetIdEOS.h"
#include "RedpointEOSCore/Id/UserIdMap.h"
#include "RedpointEOSCore/Utils/HasEventRegistration.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(
    2877257062,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat,
    FEOSVoiceManagerLocalUser)
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(
    1057666991,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat,
    FEOSVoiceManagerLocalUserJoinedChannel)

namespace REDPOINT_EOS_FILE_NS_ID(3738789065, Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::API::Lobby;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party;
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat;

class ONLINESUBSYSTEMREDPOINTEOS_API FEOSVoiceManager : public TSharedFromThis<FEOSVoiceManager>,
                                                        public IHasEventRegistration
{
    friend FEOSVoiceManagerLocalUser;
    friend FEOSVoiceManagerLocalUserJoinedChannel;

private:
    FPlatformHandle PlatformHandle;
    TSharedRef<Redpoint::EOS::Config::IConfig> Config;
    TSharedRef<IOnlineIdentity, ESPMode::ThreadSafe> Identity;
    TUserIdMap<TSharedPtr<FEOSVoiceManagerLocalUser>> LocalUsers;
    TArray<TPair<TSharedRef<const FUniqueNetIdEOS>, TSharedRef<const FOnlinePartyId>>> QueuedEchoEnabledLobbies;
    TSharedRef<FEOSVoiceManagerDeviceList> DeviceList;

    FEventHandlePtr OnNativeLobbyMemberStatusReceived;

    void Internal_UserJoinedLobby(
        const TSharedRef<const FUniqueNetIdEOS> &UserId,
        const TSharedRef<const FOnlinePartyIdRedpointBase> &LobbyId);
    void Internal_UserLeftLobby(
        const TSharedRef<const FUniqueNetIdEOS> &UserId,
        const TSharedRef<const FOnlinePartyIdRedpointBase> &LobbyId);

    void OnLobbyMemberStatusReceived(const FNotifyLobbyMemberStatusReceived::Result &Result);

public:
    FEOSVoiceManager(
        FPlatformHandle InPlatformHandle,
        const TSharedRef<Redpoint::EOS::Config::IConfig> &InConfig,
        const TSharedRef<IOnlineIdentity, ESPMode::ThreadSafe> &InIdentity);
    virtual void RegisterEvents() override;
    UE_NONCOPYABLE(FEOSVoiceManager);
    virtual ~FEOSVoiceManager() override = default;

    void ScheduleUserSynchronisationIfNeeded();

    void MarkPartyOrLobbyAsEchoEnabled(
        const TSharedRef<const FUniqueNetIdEOS> &InUserId,
        const TSharedRef<const FOnlinePartyId> &InPartyOrLobbyId);

    void UserManuallyJoinedLobby(const FUniqueNetIdEOS &UserId, const FOnlinePartyIdRedpointBase &LobbyId);
    void UserManuallyLeftLobby(const FUniqueNetIdEOS &UserId, const FOnlinePartyIdRedpointBase &LobbyId);

    void AddLocalUser(const FUniqueNetIdEOS &UserId);
    void RemoveLocalUser(const FUniqueNetIdEOS &UserId);
    void RemoveAllLocalUsers();
    TSharedPtr<FEOSVoiceManagerLocalUser> GetLocalUser(const FUniqueNetIdEOS &UserId);
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat
{
REDPOINT_EOS_FILE_NS_EXPORT(3738789065, Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat, FEOSVoiceManager)
}

#endif // #if !REDPOINT_EOS_IS_DEDICATED_SERVER

REDPOINT_EOS_CODE_GUARD_END()