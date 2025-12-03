// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Interfaces/OnlineExternalUIInterface.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Identity/OnlineIdentityInterfaceEOS.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Session/OnlineSessionInterfaceEOS.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Utils/HasEventRegistration.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3002889082, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::ExternalUI)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Identity;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Session;

class ONLINESUBSYSTEMREDPOINTEOS_API FOnlineExternalUIInterfaceEOS
    : public IOnlineExternalUI,
      public IHasEventRegistration,
      public TSharedFromThis<FOnlineExternalUIInterfaceEOS>
{
private:
    FPlatformHandle PlatformHandle;
    FOnlineIdentityInterfaceEOSRef IdentityEOS;
    TSharedRef<FOnlineSessionInterfaceEOS> SessionEOS;
    TSharedPtr<EOSEventHandle<EOS_UI_OnDisplaySettingsUpdatedCallbackInfo>> Unregister_DisplaySettingsUpdated;

public:
    FOnlineExternalUIInterfaceEOS(
        const FPlatformHandle &PlatformHandle,
        const FOnlineIdentityInterfaceEOSRef &IdentityEOS,
        const TSharedRef<FOnlineSessionInterfaceEOS> &SessionEOS);
    UE_NONCOPYABLE(FOnlineExternalUIInterfaceEOS);
    virtual ~FOnlineExternalUIInterfaceEOS() override = default;

    virtual void RegisterEvents() override;

    virtual bool ShowLoginUI(
        const int ControllerIndex,
        bool bShowOnlineOnly,
        bool bShowSkipButton,
        const FOnLoginUIClosedDelegate &Delegate = FOnLoginUIClosedDelegate()) override;
    virtual bool ShowAccountCreationUI(
        const int ControllerIndex,
        const FOnAccountCreationUIClosedDelegate &Delegate = FOnAccountCreationUIClosedDelegate()) override;
    virtual bool ShowFriendsUI(int32 LocalUserNum) override;
    virtual bool ShowInviteUI(int32 LocalUserNum, FName SessionName = NAME_GameSession) override;
    virtual bool ShowAchievementsUI(int32 LocalUserNum) override;
    virtual bool ShowLeaderboardUI(const FString &LeaderboardName) override;
    virtual bool ShowWebURL(
        const FString &Url,
        const FShowWebUrlParams &ShowParams,
        const FOnShowWebUrlClosedDelegate &Delegate = FOnShowWebUrlClosedDelegate()) override;
    virtual bool CloseWebURL() override;
    virtual bool ShowProfileUI(
        const FUniqueNetId &Requestor,
        const FUniqueNetId &Requestee,
        const FOnProfileUIClosedDelegate &Delegate = FOnProfileUIClosedDelegate()) override;
    virtual bool ShowAccountUpgradeUI(const FUniqueNetId &UniqueId) override;
    virtual bool ShowStoreUI(
        int32 LocalUserNum,
        const FShowStoreParams &ShowParams,
        const FOnShowStoreUIClosedDelegate &Delegate = FOnShowStoreUIClosedDelegate()) override;
    virtual bool ShowSendMessageUI(
        int32 LocalUserNum,
        const FShowSendMessageParams &ShowParams,
        const FOnShowSendMessageUIClosedDelegate &Delegate = FOnShowSendMessageUIClosedDelegate()) override;
    virtual bool ShowSendMessageToUserUI(
        int32 LocalUserNum,
        const FUniqueNetId &Recipient,
        const FShowSendMessageParams &ShowParams,
        const FOnShowSendMessageUIClosedDelegate &Delegate = FOnShowSendMessageUIClosedDelegate()) override;
    virtual bool ShowPlatformMessageBox(const FUniqueNetId &UserId, EPlatformMessageType MessageType) override;
    virtual void ReportEnterInGameStoreUI() override;
    virtual void ReportExitInGameStoreUI() override;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::ExternalUI
{
REDPOINT_EOS_FILE_NS_EXPORT(
    3002889082,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::ExternalUI,
    FOnlineExternalUIInterfaceEOS)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()