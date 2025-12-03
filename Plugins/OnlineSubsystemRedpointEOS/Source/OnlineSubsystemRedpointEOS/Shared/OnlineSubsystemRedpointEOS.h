// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "EOSCommon.h"
#include "IPAddress.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Identity/OnlineIdentityInterfaceEOS.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/User/OnlineUserInterfaceEOS.h"
#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemImplBase.h"
#include "OnlineSubsystemRedpointEOSModule.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSConfig/Config.h"
#include "RedpointEOSCore/Module.h"
#include "RedpointEOSCore/Utils/RegulatedTicker.h"
#include "RedpointEOSPlatform/RuntimePlatform.h"
#include "RedpointEOSRooms/RoomSystem.h"

EOS_ENABLE_STRICT_WARNINGS

class FOnlineSubsystemRedpointEASFactory;
class FCleanShutdown;
class FOnlineSubsystemEOS_Networking_P2PPacketOrderingSDK_Manager;
class FOnlineSubsystemEOS_Networking_P2PPacketOrderingSocket_Manager;
class FOnlineSubsystemEOS_Networking_P2PPacketOrderingRPC_Manager;
class FOnlineSubsystemEOS_MessagingHubTest;
class FGameplayDebuggerCategory_P2PConnections;
class FEOSUserFactory;
class FOnlineVoiceInterfaceEOS;
class FOnlineVoiceAdminInterfaceEOS;
class IServerOrchestrator;
class FOnlineSubsystemRedpointEAS;
class FOnlineFriendsInterfaceSynthetic;
class FOnlineAchievementsInterfaceEOS;
class FOnlineStatsInterfaceEOS;
class FOnlineLeaderboardsInterfaceEOS;
class FOnlineEntitlementsInterfaceSynthetic;
class FOnlineStoreInterfaceV2Synthetic;
class FOnlinePurchaseInterfaceSynthetic;
class UObject;

REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(
    3738789065,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat,
    FEOSVoiceManager)
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(
    757011916,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::User,
    FOnlineUserInterfaceEOS)
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(
    4052375527,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::Identity,
    FEpicGamesOSSIdentityHook)
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(
    1415206665,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::Identity,
    FVoiceManagerIdentityHook)
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(
    2303545192,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Lobby,
    FOnlineLobbyInterfaceEOS)
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(
    809401287,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Session,
    FOnlineSessionInterfaceEOS)
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(
    1278425628,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Avatar,
    FOnlineAvatarInterfaceSynthetic)
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(
    415509281,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party,
    FOnlinePartySystemRedpointEOS)
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(
    4053231989,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Social,
    FOnlineSocialInterfaceEOS)
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(
    496971326,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::TitleFile,
    FOnlineTitleFileInterfaceEOS)
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(
    3002889082,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::ExternalUI,
    FOnlineExternalUIInterfaceEOS)
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(
    513699043,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::UserCloud,
    FOnlineUserCloudInterfaceEOS)
#endif

class ONLINESUBSYSTEMREDPOINTEOS_API FOnlineSubsystemEOS
    : public FOnlineSubsystemImplBase,
      public TSharedFromThis<FOnlineSubsystemEOS, ESPMode::ThreadSafe>
{
    friend FOnlineSubsystemRedpointEASFactory;
    friend FCleanShutdown;
    friend FOnlineSubsystemEOS_Networking_P2PPacketOrderingSDK_Manager;
    friend FOnlineSubsystemEOS_Networking_P2PPacketOrderingSocket_Manager;
    friend FOnlineSubsystemEOS_Networking_P2PPacketOrderingRPC_Manager;
    friend FOnlineSubsystemEOS_MessagingHubTest;
    friend FGameplayDebuggerCategory_P2PConnections;
    friend ::Redpoint::EOS::OnlineSubsystemRedpointEOS::Identity::FEpicGamesOSSIdentityHook;
    friend ::Redpoint::EOS::OnlineSubsystemRedpointEOS::Identity::FVoiceManagerIdentityHook;

private:
    ::Redpoint::EOS::API::FPlatformOptionalRefCountedHandle PlatformHandle;
    FOnlineSubsystemRedpointEOSModule *Module;
    TSharedPtr<::Redpoint::EOS::Config::IConfig> Config;
    FDelegateHandle TickerHandle;
    TSharedPtr<FEOSUserFactory, ESPMode::ThreadSafe> UserFactory;
    TSharedPtr<Redpoint::EOS::Rooms::IRoomSystem> RoomSystem;
#if EOS_HAS_AUTHENTICATION
    TSharedPtr<Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat::FEOSVoiceManager> VoiceManager;
    TSharedPtr<FOnlineVoiceInterfaceEOS, ESPMode::ThreadSafe> VoiceImpl;
#endif // #if EOS_HAS_AUTHENTICATION
    TSharedPtr<FOnlineVoiceAdminInterfaceEOS, ESPMode::ThreadSafe> VoiceAdminImpl;
#if EOS_HAS_AUTHENTICATION
    TSharedPtr<FOnlineSubsystemRedpointEAS, ESPMode::ThreadSafe> SubsystemEAS;
    TSharedPtr<
        Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Lobby::FOnlineLobbyInterfaceEOS,
        ESPMode::ThreadSafe>
        LobbyImpl;
    TSharedPtr<
        ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Social::FOnlineSocialInterfaceEOS,
        ESPMode::ThreadSafe>
        SocialImpl;
    TSharedPtr<
        ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party::FOnlinePartySystemRedpointEOS,
        ESPMode::ThreadSafe>
        PartyImpl;
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    TSharedPtr<::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::UserCloud::FOnlineUserCloudInterfaceEOS>
        UserCloudImpl;
#endif
    TSharedPtr<
        Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Avatar::FOnlineAvatarInterfaceSynthetic,
        ESPMode::ThreadSafe>
        AvatarImpl;
    TSharedPtr<
        ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::ExternalUI::FOnlineExternalUIInterfaceEOS,
        ESPMode::ThreadSafe>
        ExternalUIImpl;
#endif // #if EOS_HAS_AUTHENTICATION
    TSharedPtr<
        ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Session::FOnlineSessionInterfaceEOS,
        ESPMode::ThreadSafe>
        SessionImpl;
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Identity::FOnlineIdentityInterfaceEOSPtr IdentityImpl;
    TSharedPtr<
        ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::User::FOnlineUserInterfaceEOS,
        ESPMode::ThreadSafe>
        UserImpl;
    TSharedPtr<::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::TitleFile::FOnlineTitleFileInterfaceEOS>
        TitleFileImpl;
    TSharedPtr<FOnlineAchievementsInterfaceEOS, ESPMode::ThreadSafe> AchievementsImpl;
    TSharedPtr<FOnlineStatsInterfaceEOS, ESPMode::ThreadSafe> StatsImpl;
    TSharedPtr<FOnlineLeaderboardsInterfaceEOS, ESPMode::ThreadSafe> LeaderboardsImpl;
    TSharedPtr<FOnlineEntitlementsInterfaceSynthetic, ESPMode::ThreadSafe> EntitlementsImpl;
    TSharedPtr<FOnlineStoreInterfaceV2Synthetic, ESPMode::ThreadSafe> StoreV2Impl;
    TSharedPtr<FOnlinePurchaseInterfaceSynthetic, ESPMode::ThreadSafe> PurchaseImpl;
    FDelegateHandle OnPreExitHandle;
    bool bConfigCanBeSwitched;
    bool bDidEarlyDestroyForEditor;

    bool RegulatedTick(float DeltaTime);

public:
    UE_NONCOPYABLE(FOnlineSubsystemEOS);
    FOnlineSubsystemEOS() = delete;
    FOnlineSubsystemEOS(
        FName InInstanceName,
        FOnlineSubsystemRedpointEOSModule *InModule,
        const TSharedRef<Redpoint::EOS::Config::IConfig> &InConfig);
    ~FOnlineSubsystemEOS();

    const Redpoint::EOS::Config::IConfig &GetConfig() const
    {
        return *this->Config;
    }
    const Redpoint::EOS::Platform::IRuntimePlatform &GetRuntimePlatform() const
    {
        return *Redpoint::EOS::Core::FModule::GetModuleChecked().GetRuntimePlatform();
    }

    FORCEINLINE bool IsPlatformInstanceValid() const
    {
        return this->PlatformHandle.IsValid();
    }

    virtual EOS_HPlatform GetPlatformInstance() const;
    virtual const Redpoint::EOS::API::FPlatformHandle &GetPlatformHandle() const;

    virtual bool IsEnabled() const override;
    virtual bool Tick(float DeltaTime);
    virtual bool Exec(UWorld *InWorld, const TCHAR *Cmd, FOutputDevice &Ar) override;

    virtual IOnlineSessionPtr GetSessionInterface() const override;
    virtual IOnlineFriendsPtr GetFriendsInterface() const override;
    virtual IOnlineIdentityPtr GetIdentityInterface() const override;
    virtual IOnlinePresencePtr GetPresenceInterface() const override;
    virtual IOnlinePartyPtr GetPartyInterface() const override;
    virtual IOnlineTurnBasedPtr GetTurnBasedInterface() const override;
    virtual IOnlineTournamentPtr GetTournamentInterface() const override;
    virtual IOnlineUserPtr GetUserInterface() const override;
    virtual IOnlineUserCloudPtr GetUserCloudInterface() const override;
    virtual IOnlineTitleFilePtr GetTitleFileInterface() const override;
    virtual IOnlineLeaderboardsPtr GetLeaderboardsInterface() const override;
    virtual IOnlineAchievementsPtr GetAchievementsInterface() const override;
    virtual IOnlineStatsPtr GetStatsInterface() const override;
    virtual IOnlineExternalUIPtr GetExternalUIInterface() const override;
    virtual IOnlineEntitlementsPtr GetEntitlementsInterface() const override;
    virtual IOnlineStoreV2Ptr GetStoreV2Interface() const override;
    virtual IOnlinePurchasePtr GetPurchaseInterface() const override;
    virtual TSharedPtr<Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat::FEOSVoiceManager> GetVoiceManager() const;
    virtual IOnlineVoicePtr GetVoiceInterface() const override;

    // Our custom interfaces. Note that even those these APIs return a UObject*, we return
    // non-UObjects that are TSharedFromThis.
    virtual UObject *GetNamedInterface(FName InterfaceName) override;

    virtual bool Init() override;
    virtual bool Shutdown() override;
    void RealShutdown();

    virtual FString GetAppId() const override;

    virtual FText GetOnlineServiceName(void) const override;

    bool IsTrueDedicatedServer() const;

    // @todo: Determine whether this should remain in the public interface of this class.
    TSharedRef<Redpoint::EOS::Rooms::IRoomSystem> GetRoomSystem() const
    {
        return this->RoomSystem.ToSharedRef();
    }
};

EOS_DISABLE_STRICT_WARNINGS