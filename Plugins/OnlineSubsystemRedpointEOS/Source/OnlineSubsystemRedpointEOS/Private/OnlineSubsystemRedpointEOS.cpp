// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Misc/CoreDelegates.h"
#include "Misc/MessageDialog.h"
#include "Misc/Parse.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Avatar/OnlineAvatarInterfaceSynthetic.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/ExternalUI/OnlineExternalUIInterfaceEOS.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Identity/OnlineIdentityInterfaceEOSImpl.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Lobby/OnlineLobbyInterfaceEOS.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Party/OnlinePartySystemRedpointEOS.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Session/OnlineSessionInterfaceEOS.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Social/OnlineSocialInterfaceEOS.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/TitleFile/OnlineTitleFileInterfaceEOS.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/User/OnlineUserInterfaceEOS.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/UserCloud/OnlineUserCloudInterfaceEOS.h"
#include "OnlineSubsystemRedpointEOS/Private/OnlineVoiceAdminInterfaceEOS.h"
#include "OnlineSubsystemRedpointEOS/Shared/EOSCommon.h"
#include "OnlineSubsystemRedpointEOS/Shared/EpicGames/OnlineIdentityInterfaceEAS.h"
#include "OnlineSubsystemRedpointEOS/Shared/EpicGames/OnlineSubsystemRedpointEAS.h"
#include "OnlineSubsystemRedpointEOS/Shared/OnlineAchievementsInterfaceEOS.h"
#include "OnlineSubsystemRedpointEOS/Shared/OnlineEntitlementsInterfaceSynthetic.h"
#include "OnlineSubsystemRedpointEOS/Shared/OnlineLeaderboardsInterfaceEOS.h"
#include "OnlineSubsystemRedpointEOS/Shared/OnlinePurchaseInterfaceSynthetic.h"
#include "OnlineSubsystemRedpointEOS/Shared/OnlineStatsInterfaceEOS.h"
#include "OnlineSubsystemRedpointEOS/Shared/OnlineStoreInterfaceV2Synthetic.h"
#include "OnlineSubsystemRedpointEOS/Shared/OnlineUserEOS.h"
#include "OnlineSubsystemRedpointEOS/Shared/OnlineVoiceInterfaceEOS.h"
#include "OnlineSubsystemRedpointEOS/VoiceChat/EOSVoiceManager.h"
#include "RedpointEOSAntiCheat/AntiCheatSystem.h"
#include "RedpointEOSAuth/CrossPlatform/CrossPlatformAccountId.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGames/TryDeveloperAuthenticationEASCredentialsNode.h"
#include "RedpointEOSConfig/Config.h"
#include "RedpointEOSCore/InstancePool.h"
#include "RedpointEOSCore/Legacy/EOSString.h"
#include "RedpointEOSCore/Legacy/WeakPtrHelpers.h"
#include "RedpointEOSCore/Utils/MakeSharedWithEvents.h"
#include "RedpointEOSCore/Utils/MultiOperation.h"
#include "RedpointEOSCore/Utils/WorldResolution.h"
#include "RedpointEOSPlatform/RuntimePlatform.h"
#include "RedpointEOSRooms/RoomSystem.h"
#if EOS_HAS_AUTHENTICATION
#include "RedpointEOSAuth/CrossPlatform/EpicGames/TryDeveloperAuthenticationEASCredentialsNode.h"
#endif // #if EOS_HAS_AUTHENTICATION

EOS_ENABLE_STRICT_WARNINGS

#define LOCTEXT_NAMESPACE "FOnlineSubsystemRedpointEOSModule"

FOnlineSubsystemEOS::FOnlineSubsystemEOS(
    FName InInstanceName,
    FOnlineSubsystemRedpointEOSModule *InModule,
    const TSharedRef<Redpoint::EOS::Config::IConfig> &InConfig)
    : FOnlineSubsystemImplBase(REDPOINT_EOS_SUBSYSTEM, InInstanceName)
    , PlatformHandle()
    , Module(InModule)
    , Config(InConfig)
    , TickerHandle()
    , UserFactory(nullptr)
    , RoomSystem(nullptr)
#if EOS_HAS_AUTHENTICATION
    , VoiceManager(nullptr)
    , VoiceImpl(nullptr)
#endif // #if EOS_HAS_AUTHENTICATION
    , VoiceAdminImpl(nullptr)
#if EOS_HAS_AUTHENTICATION
    , SubsystemEAS(nullptr)
    , LobbyImpl(nullptr)
    , SocialImpl(nullptr)
    , PartyImpl(nullptr)
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    , UserCloudImpl(nullptr)
#endif
    , AvatarImpl(nullptr)
#endif // #if EOS_HAS_AUTHENTICATION
    , SessionImpl(nullptr)
    , IdentityImpl(nullptr)
    , UserImpl(nullptr)
    , TitleFileImpl(nullptr)
    , AchievementsImpl(nullptr)
    , StatsImpl(nullptr)
    , LeaderboardsImpl(nullptr)
    , EntitlementsImpl(nullptr)
    , StoreV2Impl(nullptr)
    , PurchaseImpl(nullptr)
    , OnPreExitHandle()
    , bConfigCanBeSwitched(true)
    , bDidEarlyDestroyForEditor(false)
{
    Module->SubsystemInstances.Add(this->InstanceName, this);
}

FOnlineSubsystemEOS::~FOnlineSubsystemEOS()
{
    if (!this->bDidEarlyDestroyForEditor)
    {
        Module->SubsystemInstances.Remove(this->InstanceName);
    }
}

bool FOnlineSubsystemEOS::IsEnabled() const
{
    return true;
}

EOS_HPlatform FOnlineSubsystemEOS::GetPlatformInstance() const
{
    return this->PlatformHandle->Instance()->Handle();
}

const Redpoint::EOS::API::FPlatformHandle &FOnlineSubsystemEOS::GetPlatformHandle() const
{
    return this->PlatformHandle->Instance();
}

IOnlineSessionPtr FOnlineSubsystemEOS::GetSessionInterface() const
{
    return this->SessionImpl;
}

IOnlineFriendsPtr FOnlineSubsystemEOS::GetFriendsInterface() const
{
#if EOS_HAS_AUTHENTICATION
    return StaticCastSharedPtr<IOnlineFriends>(this->SocialImpl);
#else
    return nullptr;
#endif // #if EOS_HAS_AUTHENTICATION
}

IOnlineIdentityPtr FOnlineSubsystemEOS::GetIdentityInterface() const
{
    return this->IdentityImpl;
}

IOnlinePresencePtr FOnlineSubsystemEOS::GetPresenceInterface() const
{
#if EOS_HAS_AUTHENTICATION
    return StaticCastSharedPtr<IOnlinePresence>(this->SocialImpl);
#else
    return nullptr;
#endif // #if EOS_HAS_AUTHENTICATION
}

IOnlinePartyPtr FOnlineSubsystemEOS::GetPartyInterface() const
{
#if EOS_HAS_AUTHENTICATION
    return this->PartyImpl;
#else
    return nullptr;
#endif // #if EOS_HAS_AUTHENTICATION
}

IOnlineUserPtr FOnlineSubsystemEOS::GetUserInterface() const
{
    return this->UserImpl;
}

IOnlineUserCloudPtr FOnlineSubsystemEOS::GetUserCloudInterface() const
{
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    return this->UserCloudImpl;
#else
    return nullptr;
#endif
}

IOnlineTitleFilePtr FOnlineSubsystemEOS::GetTitleFileInterface() const
{
    return this->TitleFileImpl;
}

IOnlineLeaderboardsPtr FOnlineSubsystemEOS::GetLeaderboardsInterface() const
{
    return this->LeaderboardsImpl;
}

IOnlineEntitlementsPtr FOnlineSubsystemEOS::GetEntitlementsInterface() const
{
    return this->EntitlementsImpl;
}

IOnlineStoreV2Ptr FOnlineSubsystemEOS::GetStoreV2Interface() const
{
    return this->StoreV2Impl;
}

IOnlinePurchasePtr FOnlineSubsystemEOS::GetPurchaseInterface() const
{
    return this->PurchaseImpl;
}

IOnlineAchievementsPtr FOnlineSubsystemEOS::GetAchievementsInterface() const
{
    return this->AchievementsImpl;
}

IOnlineStatsPtr FOnlineSubsystemEOS::GetStatsInterface() const
{
    return this->StatsImpl;
}

IOnlineExternalUIPtr FOnlineSubsystemEOS::GetExternalUIInterface() const
{
#if EOS_HAS_AUTHENTICATION
    return this->ExternalUIImpl;
#else
    return nullptr;
#endif // #if EOS_HAS_AUTHENTICATION
}

TSharedPtr<Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat::FEOSVoiceManager> FOnlineSubsystemEOS::
    GetVoiceManager() const
{
#if EOS_HAS_AUTHENTICATION
    return this->VoiceManager;
#else
    return nullptr;
#endif // #if EOS_HAS_AUTHENTICATION
}

IOnlineVoicePtr FOnlineSubsystemEOS::GetVoiceInterface() const
{
#if EOS_HAS_AUTHENTICATION
    return this->VoiceImpl;
#else
    return nullptr;
#endif // #if EOS_HAS_AUTHENTICATION
}

class UObject *FOnlineSubsystemEOS::GetNamedInterface(FName InterfaceName)
{
#if EOS_HAS_AUTHENTICATION
    if (InterfaceName.IsEqual(ONLINE_LOBBY_INTERFACE_NAME))
    {
        return (class UObject *)(void *)&this->LobbyImpl;
    }
    if (InterfaceName.IsEqual(ONLINE_AVATAR_INTERFACE_NAME))
    {
        return (class UObject *)(void *)&this->AvatarImpl;
    }
#endif // #if EOS_HAS_AUTHENTICATION
    if (InterfaceName.IsEqual(ONLINE_VOICE_ADMIN_INTERFACE_NAME))
    {
        return (class UObject *)(void *)&this->VoiceAdminImpl;
    }

    return FOnlineSubsystemImpl::GetNamedInterface(InterfaceName);
}

IOnlineTurnBasedPtr FOnlineSubsystemEOS::GetTurnBasedInterface() const
{
    return nullptr;
}

IOnlineTournamentPtr FOnlineSubsystemEOS::GetTournamentInterface() const
{
    return nullptr;
}

bool FOnlineSubsystemEOS::Init()
{
    using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Session;
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party;
    using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Lobby;
    using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Avatar;
    using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::ExternalUI;
#endif

    EOS_SCOPE_CYCLE_COUNTER(STAT_EOSOnlineSubsystemInit);

    bool bIsDedicatedServer = this->IsTrueDedicatedServer();
    auto RuntimePlatformRegistry = Redpoint::EOS::Core::FModule::GetModuleChecked().GetRuntimePlatformRegistry();
    auto RuntimePlatform = RuntimePlatformRegistry->GetRuntimePlatform();

    this->PlatformHandle = this->Config.IsValid()
                               ? Redpoint::EOS::Core::IInstancePool::Pool().CreateWithConfig(
                                     this->InstanceName,
                                     this->Config.ToSharedRef())
                               : Redpoint::EOS::Core::IInstancePool::Pool().Create(this->InstanceName);
    if (this->PlatformHandle->Instance()->IsShutdown())
    {
        UE_LOG(LogRedpointEOS, Error, TEXT("Unable to initialize EOS platform."));
        if (FParse::Param(FCommandLine::Get(), TEXT("requireeos")))
        {
            checkf(
                !this->PlatformHandle->Instance()->IsShutdown(),
                TEXT("Unable to initialize EOS platform and you have -REQUIREEOS on the command line."));
        }
        return false;
    }

#if !WITH_EDITOR && !REDPOINT_EOS_IS_DEDICATED_SERVER
    if (this->Config->GetRequireEpicGamesLauncher())
    {
        EOS_EResult RestartResult = EOS_Platform_CheckForLauncherAndRestart(this->PlatformHandle->Instance()->Handle());
        if (RestartResult == EOS_EResult::EOS_Success)
        {
            UE_LOG(
                LogRedpointEOS,
                Warning,
                TEXT("Game is restarting because it was not launched through the Epic Games Launcher."));
            FGenericPlatformMisc::RequestExit(false);
            return false;
        }
        else if (RestartResult == EOS_EResult::EOS_NoChange)
        {
            UE_LOG(LogRedpointEOS, Verbose, TEXT("Game was already launched through the Epic Games Launcher."));
        }
        else if (RestartResult == EOS_EResult::EOS_UnexpectedError)
        {
            UE_LOG(
                LogRedpointEOS,
                Warning,
                TEXT("Game is exiting because EOS_Platform_CheckForLauncherAndRestart failed to return an expected "
                     "result."));
            FMessageDialog::Open(
                EAppMsgType::Ok,
                LOCTEXT(
                    "OnlineSubsystemEOS_EGSRestartFailed",
                    "Unable to detect if the game was launched through the Epic Games Launcher. Please reinstall the "
                    "application."));
            FGenericPlatformMisc::RequestExit(false);
            return false;
        }
    }
#endif

    auto AntiCheat = this->PlatformHandle->Instance()->GetSystem<::Redpoint::EOS::AntiCheat::IAntiCheatSystem>();
    if (AntiCheat->ShouldGameExitOnStartup())
    {
        UE_LOG(LogRedpointEOS, Error, TEXT("Anti-Cheat failed to initialize!"));
        FPlatformMisc::RequestExit(true);
        return false;
    }

    this->RoomSystem = this->PlatformHandle->Instance()->GetSystem<Redpoint::EOS::Rooms::IRoomSystem>();

    this->UserFactory = MakeShared<FEOSUserFactory, ESPMode::ThreadSafe>(
        this->GetInstanceName(),
        this->PlatformHandle->Instance(),
        RuntimePlatformRegistry);

#if EOS_HAS_AUTHENTICATION
    if (!bIsDedicatedServer)
    {
        this->SubsystemEAS =
            MakeShared<FOnlineSubsystemRedpointEAS, ESPMode::ThreadSafe>(this->InstanceName, this->AsShared());
        verifyf(this->SubsystemEAS->Init(), TEXT("EAS Subsystem did not Init successfully! This should not happen."));
    }
#endif

    auto IdentityEOS = Redpoint::EOS::Core::Utils::MakeSharedWithEvents<
        Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Identity::FOnlineIdentityInterfaceEOSImpl>(
        this->PlatformHandle->Instance(),
        this->GetInstanceName(),
        this->Config.ToSharedRef()
#if EOS_HAS_AUTHENTICATION
            ,
        this->SubsystemEAS
#endif
    );
    this->IdentityImpl = IdentityEOS;

    this->UserImpl = MakeShared<
        Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::User::FOnlineUserInterfaceEOS,
        ESPMode::ThreadSafe>(IdentityEOS, this->PlatformHandle->Instance());

#if EOS_HAS_AUTHENTICATION
    this->VoiceManager = Redpoint::EOS::Core::Utils::MakeSharedWithEvents<
        Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat::FEOSVoiceManager>(
        this->PlatformHandle->Instance(),
        this->Config.ToSharedRef(),
        this->IdentityImpl.ToSharedRef());
#endif
    EOS_HRTCAdmin EOSRTCAdmin = EOS_Platform_GetRTCAdminInterface(this->PlatformHandle->Instance()->Handle());
    if (EOSRTCAdmin != nullptr)
    {
        this->VoiceAdminImpl = MakeShared<FOnlineVoiceAdminInterfaceEOS, ESPMode::ThreadSafe>(EOSRTCAdmin);
    }
#if EOS_HAS_AUTHENTICATION
    this->VoiceImpl = MakeShared<FOnlineVoiceInterfaceEOS, ESPMode::ThreadSafe>(
        this->VoiceManager.ToSharedRef(),
        this->IdentityImpl.ToSharedRef());

    TSharedPtr<FOnlinePartySystemRedpointEOS, ESPMode::ThreadSafe> EOSPartyImpl;
    if (!bIsDedicatedServer)
    {
        this->UserCloudImpl = MakeShared<
            ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::UserCloud::FOnlineUserCloudInterfaceEOS>(
            this->PlatformHandle->Instance());

        this->SocialImpl = ::Redpoint::EOS::Core::Utils::MakeSharedWithEvents<
            ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Social::FOnlineSocialInterfaceEOS>(
            this->PlatformHandle->Instance());

        this->AvatarImpl = MakeShared<FOnlineAvatarInterfaceSynthetic, ESPMode::ThreadSafe>(
            this->PlatformHandle->Instance(),
            this->IdentityImpl.ToSharedRef(),
            this->SocialImpl.ToSharedRef());

        EOSPartyImpl = ::Redpoint::EOS::Core::Utils::MakeSharedWithEvents<FOnlinePartySystemRedpointEOS>(
            this->PlatformHandle->Instance(),
            this->RoomSystem.ToSharedRef(),
            this->UserFactory.ToSharedRef(),
            this->VoiceManager.ToSharedRef(),
            this->SocialImpl.ToSharedRef());
        this->PartyImpl = EOSPartyImpl;

        this->LobbyImpl = MakeShared<FOnlineLobbyInterfaceEOS, ESPMode::ThreadSafe>(
            this->RoomSystem.ToSharedRef(),
            this->AsShared(),
            this->VoiceManager.ToSharedRef());
        this->LobbyImpl->RegisterEvents();
    }
#endif // #if EOS_HAS_AUTHENTICATION

    auto EOSSessionImpl = ::Redpoint::EOS::Core::Utils::MakeSharedWithEvents<FOnlineSessionInterfaceEOS>(
        this->PlatformHandle->Instance(),
        IdentityEOS,
#if EOS_HAS_AUTHENTICATION
        bIsDedicatedServer ? nullptr : SocialImpl,
#else
        nullptr,
#endif
        this->Config.ToSharedRef());
    this->SessionImpl = EOSSessionImpl;
    this->TitleFileImpl = MakeShared<
        ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::TitleFile::FOnlineTitleFileInterfaceEOS>(
        this->PlatformHandle->Instance());
    this->StatsImpl = MakeShared<FOnlineStatsInterfaceEOS, ESPMode::ThreadSafe>(
        this->PlatformHandle->Instance()->Handle(),
        this->Config.ToSharedRef(),
        this->GetInstanceName());
    auto EOSAchievementsImpl = MakeShared<FOnlineAchievementsInterfaceEOS, ESPMode::ThreadSafe>(
        this->PlatformHandle->Instance()->Handle(),
        this->StatsImpl,
        this->Config.ToSharedRef());
    EOSAchievementsImpl->RegisterEvents();
    this->AchievementsImpl = EOSAchievementsImpl;
    this->LeaderboardsImpl = MakeShared<FOnlineLeaderboardsInterfaceEOS, ESPMode::ThreadSafe>(
        this->PlatformHandle->Instance()->Handle(),
        this->StatsImpl,
        this->IdentityImpl
#if EOS_HAS_AUTHENTICATION
        ,
        bIsDedicatedServer ? nullptr : this->SocialImpl
#endif // #if EOS_HAS_AUTHENTICATION
    );
    this->EntitlementsImpl =
        MakeShared<FOnlineEntitlementsInterfaceSynthetic, ESPMode::ThreadSafe>(this->IdentityImpl.ToSharedRef());
    this->StoreV2Impl =
        MakeShared<FOnlineStoreInterfaceV2Synthetic, ESPMode::ThreadSafe>(this->IdentityImpl.ToSharedRef());
    this->PurchaseImpl =
        MakeShared<FOnlinePurchaseInterfaceSynthetic, ESPMode::ThreadSafe>(this->IdentityImpl.ToSharedRef());
    this->PurchaseImpl->RegisterEvents();

#if EOS_HAS_AUTHENTICATION
    if (!bIsDedicatedServer)
    {
        this->ExternalUIImpl = ::Redpoint::EOS::Core::Utils::MakeSharedWithEvents<FOnlineExternalUIInterfaceEOS>(
            this->PlatformHandle->Instance(),
            this->IdentityImpl.ToSharedRef(),
            this->SessionImpl.ToSharedRef());
        this->ExternalUIImpl->RegisterEvents();
    }
#endif // #if EOS_HAS_AUTHENTICATION

    // If we are a dedicated server, call AutoLogin on the identity interface so that we have a valid
    // user ID in LocalUserNum 0 for all dedicated server operations.
    if (bIsDedicatedServer)
    {
        UE_LOG(
            LogRedpointEOS,
            Verbose,
            TEXT("Automatically calling AutoLogin for LocalUserNum 0 because the game is running as a dedicated "
                 "server"));
        this->IdentityImpl->AutoLogin(0);
    }

#if EOS_HAS_AUTHENTICATION
    if (!bIsDedicatedServer)
    {
                using namespace ::Redpoint::EOS::Presence;

        // Broadcast OnConnectionStatusChanged event when the presence system tells us Internet connectivity has
        // changed.
        auto PresenceSystem = this->PlatformHandle->Instance()->GetSystem<IPresenceSystem>();
        PresenceSystem->OnInternetConnectivityChanged().Add(
            IPresenceSystem::FOnInternetConnectivityChanged::FDelegate::CreateSPLambda(
                this,
                [this](IPresenceSystem::EInternetConnectivityStatus ConnectionStatus) {
                    if (ConnectionStatus == IPresenceSystem::EInternetConnectivityStatus::Connected)
                    {
                        UE_LOG(
                            LogRedpointEOS,
                            Verbose,
                            TEXT("Broadcasting OnInternetConnectivityChanged event from OSSv1 with new connection "
                                 "status: Connected"));
                        this->TriggerOnConnectionStatusChangedDelegates(
                            TEXT("RedpointEOS"),
                            EOnlineServerConnectionStatus::NotConnected,
                            EOnlineServerConnectionStatus::Connected);
                    }
                    else
                    {
                        UE_LOG(
                            LogRedpointEOS,
                            Verbose,
                            TEXT("Broadcasting OnInternetConnectivityChanged event from OSSv1 with new connection "
                                 "status: NotConnected"));
                        this->TriggerOnConnectionStatusChangedDelegates(
                            TEXT("RedpointEOS"),
                            EOnlineServerConnectionStatus::Connected,
                            EOnlineServerConnectionStatus::NotConnected);
                    }
                }));
    }
#endif

    // We want to shutdown ahead of other subsystems, because we will have hooks and events registered
    // to them. If those events are still registered when other subsystems try to shutdown, we'll get
    // check() fails because the shared pointers will still have references.
    this->OnPreExitHandle = FCoreDelegates::OnPreExit.AddLambda([WeakThis = GetWeakThis(this)]() {
        if (auto This = PinWeakThis(WeakThis))
        {
            This->Shutdown();
        }
    });

#if WITH_EDITOR
    // We need to listen for FWorldDelegates::OnStartGameInstance. When initializing a play-in-editor window,
    // the UpdateUniqueNetIdForPlayerController logic from post-login runs before the UWorld* or ULocalPlayer*
    // is created, so we can't set the unique net ID that way. However, Unreal doesn't assign the unique net ID
    // it got from logging in to the new ULocalPlayer*, so it'll be stuck with an invalid net ID.
    //
    // We listen to FWorldDelegates::OnStartGameInstance so we can fix up any ULocalPlayer* instances to have
    // the unique net ID that was signed in from "login before PIE".
    FWorldDelegates::OnStartGameInstance.AddLambda([WeakThis = GetWeakThis(this)](UGameInstance *NewGameInstance) {
        if (auto This = PinWeakThis(WeakThis))
        {
            TSoftObjectPtr<UWorld> OurWorld =
                Redpoint::EOS::Core::Utils::FWorldResolution::GetWorld(This->GetInstanceName(), true);
            checkf(
                IsValid(NewGameInstance->GetWorld()),
                TEXT("World must be valid for new game instance in OnStartGameInstance handler."));
            if (!OurWorld.IsValid() || OurWorld.Get() != NewGameInstance->GetWorld())
            {
                return;
            }

            for (ULocalPlayer *Player : NewGameInstance->GetLocalPlayers())
            {
                if (This->IdentityImpl->GetLoginStatus(Player->GetControllerId()) == ELoginStatus::LoggedIn)
                {
                    auto ThisUniqueId = This->IdentityImpl->GetUniquePlayerId(Player->GetControllerId());
                    Player->SetCachedUniqueNetId(FUniqueNetIdRepl(ThisUniqueId));
                    UE_LOG(
                        LogRedpointEOS,
                        Verbose,
                        TEXT("Updated unique net ID of ULocalPlayer for local player num %d (via "
                             "OnStartGameInstance): %s"),
                        Player->GetControllerId(),
                        *Player->GetCachedUniqueNetId()->ToString());
                }
            }
        }
    });
#endif

    this->TickerHandle = Redpoint::EOS::Core::Utils::FRegulatedTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateThreadSafeSP(this, &FOnlineSubsystemEOS::RegulatedTick));

    return true;
}

bool FOnlineSubsystemEOS::Tick(float DeltaTime)
{
    // We don't need an unregulated tick.
    return false;
}

bool FOnlineSubsystemEOS::RegulatedTick(float DeltaTime)
{
    EOS_SCOPE_CYCLE_COUNTER(STAT_EOSOnlineSubsystemTick);

    if (this->PlatformHandle.IsValid())
    {
#if EOS_HAS_AUTHENTICATION
        if (this->VoiceManager.IsValid())
        {
            this->VoiceManager->ScheduleUserSynchronisationIfNeeded();
        }
#endif
    }

    return FOnlineSubsystemImpl::Tick(DeltaTime);
}

bool FOnlineSubsystemEOS::Exec(UWorld *InWorld, const TCHAR *Cmd, FOutputDevice &Ar)
{
    bool bWasHandled = false;

    if (FParse::Command(&Cmd, TEXT("IDENTITY")))
    {
        bWasHandled = this->IdentityImpl->Exec(InWorld, Cmd, Ar);
    }

    if (!bWasHandled)
    {
        bWasHandled = FOnlineSubsystemImpl::Exec(InWorld, Cmd, Ar);
    }

    return bWasHandled;
}

template <typename T, ESPMode Mode> void DestructInterface(TSharedPtr<T, Mode> &Ref, const TCHAR *Name)
{
    if (Ref.IsValid())
    {
        ensureMsgf(
            Ref.IsUnique(),
            TEXT(
                "Interface is not unique during shutdown of EOS Online Framework: %s. "
                "This indicates you have a TSharedPtr<> or IOnline... in your code that is holding a reference open to "
                "the interface longer than the lifetime of the online subsystem. You should use TWeakPtr<> "
                "to hold references to interfaces in class fields to prevent lifetime issues"),
            Name);
        Ref = nullptr;
    }
}

class FCleanShutdown
{
private:
    TSharedPtr<FOnlineSubsystemEOS, ESPMode::ThreadSafe> EOS;
    TSharedPtr<
        ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Session::FOnlineSessionInterfaceEOS,
        ESPMode::ThreadSafe>
        SessionImpl;

public:
    FCleanShutdown(
        TSharedPtr<FOnlineSubsystemEOS, ESPMode::ThreadSafe> InEOS,
        TSharedPtr<
            ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Session::FOnlineSessionInterfaceEOS,
            ESPMode::ThreadSafe> InSession)
        : EOS(MoveTemp(InEOS))
        , SessionImpl(MoveTemp(InSession)) {};

    void Shutdown();
};

void FCleanShutdown::Shutdown()
{
    using namespace ::Redpoint::EOS::Core::Utils;

#if EOS_HAS_AUTHENTICATION
    Redpoint::EOS::Auth::CrossPlatform::EpicGames::FTryDeveloperAuthenticationEASCredentialsNode::
        ForceLCTDeveloperInProgressMutexReset();
#endif // #if EOS_HAS_AUTHENTICATION

    // Set up a ticker so we can continue ticking (since the engine will no longer call the EOS's subsystems Tick
    // event).
    FTSTicker::FDelegateHandle TickerHandle = FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateLambda([EOS = this->EOS](float DeltaSeconds) {
            EOS->Tick(DeltaSeconds);
            return true;
        }),
        0.0f);

    TArray<FName> SessionNames;
    for (const auto &Session : this->SessionImpl->Sessions)
    {
        SessionNames.Add(Session.SessionName);
    }
    this->SessionImpl.Reset();

    FMultiOperation<FName, bool>::Run(
        SessionNames,
        FMultiOperation<FName, bool>::FInitiate::CreateLambda([this](
                                                                  const FName &SessionName,
                                                                  const TDelegate<void(bool)> &OnDone) {
            UE_LOG(
                LogRedpointEOS,
                Verbose,
                TEXT(
                    "Automatically destroying up session %s for you, since you're running in the editor. Sessions will "
                    "not be automatically destroyed if you're running in standalone."),
                *SessionName.ToString());
            auto SessionNameAnsi = EOSString_SessionModification_SessionName::ToAnsiString(SessionName.ToString());
            EOS_Sessions_DestroySessionOptions Opts = {};
            Opts.ApiVersion = EOS_SESSIONS_ENDSESSION_API_LATEST;
            Opts.SessionName = SessionNameAnsi.Ptr.Get();
            EOSRunOperation<EOS_HSessions, EOS_Sessions_DestroySessionOptions, EOS_Sessions_DestroySessionCallbackInfo>(
                this->EOS->PlatformHandle->Instance()->Get<EOS_HSessions>(),
                &Opts,
                EOS_Sessions_DestroySession,
                [SessionName, OnDone](const EOS_Sessions_DestroySessionCallbackInfo *Info) {
                    if (Info->ResultCode == EOS_EResult::EOS_Success)
                    {
                        UE_LOG(
                            LogRedpointEOS,
                            Verbose,
                            TEXT("Session %s was automatically destroyed successfully."),
                            *SessionName.ToString());
                    }
                    else
                    {
                        UE_LOG(
                            LogRedpointEOS,
                            Error,
                            TEXT("Session %s could not be automatically destroyed. It may continue to exist on the "
                                 "EOS backend for a few minutes."),
                            *SessionName.ToString());
                    }
                    OnDone.ExecuteIfBound(true);
                });
            return true;
        }),
        FMultiOperation<FName, bool>::FOnDone::CreateLambda([this, TickerHandle](const TArray<bool> &Results) {
            // Do the real shutdown. We have to be super careful with pointers here for uniqueness checks!
            FTSTicker::GetCoreTicker().AddTicker(
                FTickerDelegate::CreateLambda([this, TickerHandle](float DeltaSeconds) {
                    FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
                    FName ShutdownName =
                        FName(FString::Printf(TEXT("%s_ShuttingDown"), *this->EOS->InstanceName.ToString()));
                    this->EOS->RealShutdown();
                    this->EOS->Module->SubsystemInstances.Remove(ShutdownName);
                    this->EOS.Reset();
                    delete this;
                    return false;
                }),
                0.0f);
        }));
}

bool FOnlineSubsystemEOS::Shutdown()
{
#if WITH_EDITOR
    // bIsPerformingFullShutdown is true in *most* cases when the editor is exiting (from pressing X in the top-right
    // corner of the window). It's not perfect, but should eliminate some cases of delayed cleanup running on shutdown.
    bool bIsPerformingFullShutdown = IsEngineExitRequested();
    // WITH_EDITOR is true if you're playing "Standalone Game" from the editor, but bIsEditor will be false. We don't
    // want to do delayed cleanup for standalone games since the process will exit soon after calling ::Shutdown().
    bool bIsEditor = GIsEditor;
    if (this->PlatformHandle.IsValid() && this->SessionImpl.IsValid() && bIsEditor && !bIsPerformingFullShutdown)
    {
        // When we're running in the editor, try to destroy all sessions that we're a part of before we do the real
        // shutdown logic.
        FName ShutdownName = FName(FString::Printf(TEXT("%s_ShuttingDown"), *this->InstanceName.ToString()));
        if (Module->SubsystemInstances.Contains(ShutdownName) ||
            !Module->SubsystemInstances.Contains(this->InstanceName))
        {
            return true;
        }
        Module->SubsystemInstances.Add(ShutdownName, this);
        Module->SubsystemInstances.Remove(this->InstanceName);
        this->bDidEarlyDestroyForEditor = true;
        auto Impl = new FCleanShutdown(this->AsShared(), this->SessionImpl);
        Impl->Shutdown();
        return true;
    }
    else
    {
        // EOS didn't initialize the editor, so we don't need to clean up sessions.
        this->RealShutdown();
        return true;
    }
#else
    this->RealShutdown();
    return true;
#endif
}

void FOnlineSubsystemEOS::RealShutdown()
{
    EOS_SCOPE_CYCLE_COUNTER(STAT_EOSOnlineSubsystemShutdown);

    FCoreDelegates::OnPreExit.Remove(this->OnPreExitHandle);

#if EOS_HAS_AUTHENTICATION
    DestructInterface(this->ExternalUIImpl, TEXT("IOnlineExternalUI"));
#endif // #if EOS_HAS_AUTHENTICATION
    DestructInterface(this->PurchaseImpl, TEXT("IOnlinePurchase"));
    DestructInterface(this->StoreV2Impl, TEXT("IOnlineStoreV2"));
    DestructInterface(this->EntitlementsImpl, TEXT("IOnlineEntitlements"));
    DestructInterface(this->LeaderboardsImpl, TEXT("IOnlineLeaderboards"));
    DestructInterface(this->AchievementsImpl, TEXT("IOnlineAchievements"));
    DestructInterface(this->StatsImpl, TEXT("IOnlineStats"));
    DestructInterface(this->TitleFileImpl, TEXT("IOnlineTitleFile"));
    DestructInterface(this->SessionImpl, TEXT("IOnlineSession"));
#if EOS_HAS_AUTHENTICATION
    DestructInterface(this->PartyImpl, TEXT("IOnlinePartySystem"));
    DestructInterface(this->LobbyImpl, TEXT("IOnlineLobby"));
    DestructInterface(this->AvatarImpl, TEXT("IOnlineAvatar"));
    DestructInterface(this->SocialImpl, TEXT("IOnlinePresence/IOnlineFriends"));
    DestructInterface(this->UserCloudImpl, TEXT("IOnlineUserCloud"));
    DestructInterface(this->VoiceImpl, TEXT("IOnlineVoice"));
#endif // #if EOS_HAS_AUTHENTICATION
    DestructInterface(this->VoiceAdminImpl, TEXT("IOnlineVoiceAdmin"));
#if EOS_HAS_AUTHENTICATION
    if (this->VoiceManager.IsValid())
    {
        this->VoiceManager->RemoveAllLocalUsers();
    }
    DestructInterface(this->VoiceManager, TEXT("(internal) VoiceManager"));
#endif // #if EOS_HAS_AUTHENTICATION
    DestructInterface(this->UserImpl, TEXT("IOnlineUser"));
    DestructInterface(this->IdentityImpl, TEXT("IOnlineIdentity"));
#if EOS_HAS_AUTHENTICATION
    if (this->SubsystemEAS.IsValid())
    {
        verifyf(this->SubsystemEAS->Shutdown(), TEXT("EAS Subsystem did not shutdown successfully!"));
    }
    DestructInterface(this->SubsystemEAS, TEXT("(internal) SubsystemEAS"));
#endif // #if EOS_HAS_AUTHENTICATION
    DestructInterface(this->UserFactory, TEXT("(internal) UserFactory"));
    if (this->RoomSystem.IsValid())
    {
        // @note: This will not be a unique interface because FRoomSystemPoolImpl
        // tracks instances of room systems.
        this->RoomSystem.Reset();
    }

    // Shutdown the platform.
    if (this->PlatformHandle.IsValid())
    {
        EOS_HPlatform OldHandleForLog = this->PlatformHandle->Instance()->Handle();
        this->PlatformHandle.Reset();

        UE_LOG(LogRedpointEOS, Verbose, TEXT("EOS platform %p has been released."), OldHandleForLog);
    }

    Redpoint::EOS::Core::Utils::FRegulatedTicker::GetCoreTicker().RemoveTicker(this->TickerHandle);
}

FString FOnlineSubsystemEOS::GetAppId() const
{
    return this->Config->GetClientId();
}

FText FOnlineSubsystemEOS::GetOnlineServiceName(void) const
{
    return NSLOCTEXT("EOS", "EOSPlatformName", "Epic Online Services");
}

bool FOnlineSubsystemEOS::IsTrueDedicatedServer() const
{
    // Neither IsServer nor IsDedicated work correctly in play-in-editor. Both listen servers
    // and dedicated servers return true from IsServer, but *neither* return true from IsDedicated
    // (in fact it looks like IsDedicated just doesn't do the right thing at all for single process).
    //
    // So instead implement our own version here which does the detection correctly.

    if (GIsAutomationTesting && this->InstanceName.ToString().EndsWith(TEXT("_AutomationDedicatedServer")))
    {
        // Allow automation tests to force creation of dedicated server instance.
        return true;
    }

#if WITH_EDITOR
    // Running multiple worlds in the editor; we need to see if this world is specifically a dedicated server.
    FName WorldContextHandle =
        (this->InstanceName != NAME_None && this->InstanceName != DefaultInstanceName) ? this->InstanceName : NAME_None;
    if (WorldContextHandle.IsNone())
    {
        // The default OSS instance is equal to IsRunningDedicatedServer(), in case the editor
        // is being run with -server for multi-process.
        return IsRunningDedicatedServer();
    }
    if (GEngine == nullptr)
    {
        UE_LOG(
            LogRedpointEOS,
            Error,
            TEXT("GEngine is not available, but EOS requires it in order to detect if it is running as a dedicated "
                 "server inside the editor! Please report this error in the EOS Online Framework Discord!"));
        return false;
    }
    FWorldContext *WorldContext = GEngine->GetWorldContextFromHandle(WorldContextHandle);
    if (WorldContext == nullptr)
    {
        // World context is invalid. This will occur during unit tests.
        return false;
    }
    return WorldContext->RunAsDedicated;
#else
    // Just use IsRunningDedicatedServer, since our process is either a server or it's not.
    return IsRunningDedicatedServer();
#endif
}

#undef LOCTEXT_NAMESPACE

EOS_DISABLE_STRICT_WARNINGS
