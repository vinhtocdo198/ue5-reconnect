// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Delegates/DelegateCombinations.h"
#include "Misc/Optional.h"
#include "RedpointEOSAntiCheat/AntiCheatSystem.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSConfig/Config.h"
#include "RedpointEOSControlChannel.h"
#include "RedpointEOSCore/Utils/WorldResolution.h"
#include "RedpointEOSNetDriver.h"
#include "RedpointEOSNetworking/Auth/AuthPhaseFailureCode.h"
#include "RedpointEOSNetworking/Auth/NetworkHelpers.h"
#include "RedpointEOSNetworking/Logging.h"
#include "RedpointEOSNetworking/NetDriverImpl.h"
#include "RedpointEOSNetworking/NetDriverResolution.h"
#include "RedpointEOSNetworking/NetDriverRole.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

class URedpointEOSControlChannel;

namespace REDPOINT_EOS_FILE_NS_ID(3952495614, Redpoint::EOS::Networking::Auth)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::AntiCheat;
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::Config;
using namespace ::Redpoint::EOS::Networking;
using namespace ::Redpoint::EOS::Networking::Auth;

typedef TMulticastDelegate<void(EAuthPhaseFailureCode FailureCode, const FString &ErrorMessage)> FAuthPhasesCompleted;
typedef TDelegate<void(EAuthPhaseFailureCode FailureCode, const FString &ErrorMessage)>
    FOnClientMustDisconnectFromServer;

struct FAuthPhaseContextInfo
{
public:
    URedpointEOSControlChannel *const ControlChannel;
    UNetConnection *const Connection;
    const TSharedPtr<FPlatformInstance> PlatformHandle;
    const TSharedPtr<IConfig> Config;
    const TOptional<ENetDriverRole> Role;
    ISocketSubsystem *const SocketSubsystem;
    const TSharedPtr<FAntiCheatSession> AntiCheatSession;
    const bool bIsBeacon;
    const TOptional<bool> bIsTrustedOnClient;
};

class IAuthPhaseContext
{
public:
    virtual ~IAuthPhaseContext() = default;

    [[nodiscard]] virtual FAuthPhaseContextInfo GetInfo() const = 0;

    [[nodiscard]] virtual bool GetControlChannel(URedpointEOSControlChannel *&OutControlChannel) const = 0;
    [[nodiscard]] virtual bool GetConnection(UNetConnection *&OutConnection) const = 0;
    [[nodiscard]] virtual bool GetPlatformHandle(TSharedPtr<FPlatformInstance> &OutPlatformHandle) const = 0;
    [[nodiscard]] virtual bool GetConfig(const Redpoint::EOS::Config::IConfig *&OutConfig) = 0;
    [[nodiscard]] virtual bool GetRole(ENetDriverRole &OutRole) = 0;
    [[nodiscard]] virtual bool GetSocketSubsystem(ISocketSubsystem *&OutSocketSubsystem) = 0;
    virtual void Finish(EAuthPhaseFailureCode ErrorCode) = 0;
};

// @hack: This intentionally does not inherit from TSharedFromThis, because Clang makes bad tail call
// optimizations that result in AsShared() being unusable anyway.
template <typename TPhase, typename TDerivedContext> class FAuthPhaseContext : public IAuthPhaseContext
{
    friend URedpointEOSControlChannel;

private:
    TSharedPtr<TDerivedContext> SelfRef;
    TSoftObjectPtr<URedpointEOSControlChannel> ControlChannel;
    FAuthPhasesCompleted OnCompletedInternal;
    TArray<TSharedRef<TPhase>> Phases;
    FOnClientMustDisconnectFromServer OnClientMustDisconnectFromServer;

protected:
    FAuthPhaseContext(URedpointEOSControlChannel *InControlChannel)
        : SelfRef(nullptr)
        , ControlChannel(InControlChannel)
        , OnCompletedInternal()
        , Phases()
    {
    }

    virtual FString GetIdentifier() const = 0;
    virtual FString GetPhaseGroup() const = 0;

public:
    UE_NONCOPYABLE(FAuthPhaseContext);
    virtual ~FAuthPhaseContext() override = default;

    [[nodiscard]] virtual FAuthPhaseContextInfo GetInfo() const override
    {
        URedpointEOSControlChannel *_ControlChannel = nullptr;
        UNetConnection *_Connection = nullptr;
        TSharedPtr<FPlatformInstance> _PlatformHandle;
        TSharedPtr<IConfig> _Config;
        TOptional<ENetDriverRole> _Role;
        ISocketSubsystem *_SocketSubsystem = nullptr;
        TSharedPtr<FAntiCheatSession> _AntiCheatSession;
        bool _bIsBeacon = false;
        bool _bIsTrustedOnClient = false;

        if (this->ControlChannel.IsValid())
        {
            _ControlChannel = this->ControlChannel.Get();
            _Connection = this->ControlChannel->Connection;

            if (IsValid(_Connection))
            {
                auto LocalNetDriver = Cast<URedpointEOSNetDriver>(_Connection->Driver);
                if (IsValid(LocalNetDriver))
                {
                    _PlatformHandle = FNetDriverResolution::TryGetPlatformHandle(LocalNetDriver);
                    if (_PlatformHandle.IsValid())
                    {
                        _Config = _PlatformHandle->GetEnvironment()->Config;
                    }
                    _Role = LocalNetDriver->GetImpl()->GetEOSRole();
                    _SocketSubsystem = LocalNetDriver->GetSocketSubsystem();
                    _AntiCheatSession = LocalNetDriver->GetImpl()->AntiCheatSession;
                    _bIsBeacon = LocalNetDriver->GetImpl()->bIsOwnedByBeacon;
                    _bIsTrustedOnClient = this->ControlChannel->bClientTrustsServer;
                }
            }
        }

        return FAuthPhaseContextInfo{
            _ControlChannel,
            _Connection,
            _PlatformHandle,
            _Config,
            _Role,
            _SocketSubsystem,
            _AntiCheatSession,
            _bIsBeacon,
            _bIsTrustedOnClient};
    }

    [[nodiscard]] virtual bool GetControlChannel(URedpointEOSControlChannel *&OutControlChannel) const override
    {
        if (this->ControlChannel.IsValid())
        {
            OutControlChannel = this->ControlChannel.Get();
            return true;
        }
        return false;
    }

    [[nodiscard]] virtual bool GetConnection(UNetConnection *&OutConnection) const override
    {
        if (this->ControlChannel.IsValid() && IsValid(this->ControlChannel->Connection))
        {
            OutConnection = this->ControlChannel->Connection;
            return true;
        }
        return false;
    }

    [[nodiscard]] virtual bool GetPlatformHandle(TSharedPtr<FPlatformInstance> &OutPlatformHandle) const override
    {
        UNetConnection *Connection;
        if (GetConnection(Connection))
        {
            FNetworkHelpers::GetPlatformHandle(Connection, OutPlatformHandle);
            return true;
        }
        return false;
    }

    [[nodiscard]] virtual bool GetConfig(const IConfig *&OutConfig) override
    {
        UNetConnection *Connection;
        if (GetConnection(Connection))
        {
            OutConfig = FNetworkHelpers::GetConfig(Connection);
            return true;
        }
        return false;
    }

    [[nodiscard]] virtual bool GetRole(ENetDriverRole &OutRole) override
    {
        UNetConnection *Connection;
        if (GetConnection(Connection))
        {
            OutRole = FNetworkHelpers::GetRole(Connection);
            return true;
        }
        return false;
    }

    [[nodiscard]] virtual bool GetSocketSubsystem(ISocketSubsystem *&OutSocketSubsystem) override
    {
        UNetConnection *Connection;
        if (GetConnection(Connection))
        {
            OutSocketSubsystem = FNetworkHelpers::GetSocketSubsystem(Connection);
            return true;
        }
        return false;
    }

    [[nodiscard]] bool GetAntiCheat(
        TSharedPtr<FPlatformInstance> &OutPlatformHandle,
        TSharedPtr<FAntiCheatSession> &OutAntiCheatSession,
        bool &OutIsBeacon) const
    {
        UNetConnection *Connection;
        if (GetConnection(Connection))
        {
            FNetworkHelpers::GetAntiCheat(Connection, OutPlatformHandle, OutAntiCheatSession, OutIsBeacon);
            return true;
        }
        return false;
    }

    template <typename TDerivedPhase> TSharedPtr<TDerivedPhase> GetPhase(const FName &InPhaseName)
    {
        for (const auto &Entry : this->Phases)
        {
            if (Entry->GetName().IsEqual(InPhaseName))
            {
                return StaticCastSharedRef<TDerivedPhase>(Entry);
            }
        }
        return nullptr;
    }

    FAuthPhasesCompleted &OnCompleted()
    {
        return this->OnCompletedInternal;
    }

    /** Log a verbose message from the current phase. */
    void Log(const FString &Message)
    {
        if (this->Phases.Num() == 0)
        {
            UE_LOG(
                LogRedpointEOSNetworkAuth,
                Verbose,
                TEXT("%s: %s: %s"),
                *GetIdentifier(),
                *GetPhaseGroup(),
                *Message);
        }
        else
        {
            UE_LOG(
                LogRedpointEOSNetworkAuth,
                Verbose,
                TEXT("%s: %s: %s: %s"),
                *GetIdentifier(),
                *GetPhaseGroup(),
                *this->Phases[0]->GetName().ToString(),
                *Message);
        }
    }

    /** Register phases for client routing. Does not execute phases. */
    void RegisterPhasesForClientRouting(
        const TArray<TSharedRef<TPhase>> &InPhases,
        const FOnClientMustDisconnectFromServer &InOnClientMustDisconnectFromServer)
    {
#if DO_CHECK
        UNetConnection *Connection;
        verifyf(
            this->GetConnection(Connection),
            TEXT("RegisterPhasesForClientRouting must only be called on code paths that have already checked that the "
                 "connection is still valid."));
        ENetDriverRole Role = FNetworkHelpers::GetRole(Connection);
        checkf(
            Role == ENetDriverRole::ClientConnectedToDedicatedServer ||
                Role == ENetDriverRole::ClientConnectedToListenServer,
            TEXT("RegisterPhasesForClientRouting can only be called on clients!"));
#endif
        this->Phases = InPhases;
        this->OnClientMustDisconnectFromServer = InOnClientMustDisconnectFromServer;
    }

    /** Start the first authentication phase. This must be called from the server. */
    void Start(
        // @hack: For some reason, Clang makes a bad TCO that results in AsShared() being unset, so
        // we have to pass the value that would be in AsShared() directly into Start() and store it
        // ourselves. I tried many alternatives to get it to keep the value in AsShared() properly,
        // but it refused to do so. :/
        TSharedRef<FAuthPhaseContext<TPhase, TDerivedContext>> ThisRef,
        const TArray<TSharedRef<TPhase>> &InPhases)
    {
#if DO_CHECK
        {
            ENetDriverRole Role;
            if (this->GetRole(Role))
            {
                checkf(
                    Role == ENetDriverRole::DedicatedServer || Role == ENetDriverRole::ListenServer,
                    TEXT("IAuthPhaseContext::Start must only be called on the server. This is a bug in the Redpoint "
                         "EOS Online Framework plugin."));
            }
        }
#endif

        checkf(!this->SelfRef.IsValid(), TEXT("Start() called multiple times on same context!"));
        TSharedRef<TDerivedContext> DowncastedThisRef = StaticCastSharedRef<TDerivedContext>(ThisRef);
        this->SelfRef = DowncastedThisRef;

        ENetDriverRole Role;
        if (!this->GetRole(Role))
        {
            this->Finish(EAuthPhaseFailureCode::All_ConnectionHasGoneAway);
            return;
        }
        checkf(
            Role == ENetDriverRole::DedicatedServer || Role == ENetDriverRole::ListenServer,
            TEXT("Start can only be called on servers!"));

        this->Phases = InPhases;
        if (this->Phases.Num() == 0)
        {
            UE_LOG(
                LogRedpointEOSNetworkAuth,
                Verbose,
                TEXT("%s: %s: Immediately finishing as there are no phases to run."),
                *GetIdentifier(),
                *GetPhaseGroup());
            this->Finish(EAuthPhaseFailureCode::Success);
        }
        else
        {
            UE_LOG(
                LogRedpointEOSNetworkAuth,
                Verbose,
                TEXT("%s: %s: %s: Starting..."),
                *GetIdentifier(),
                *GetPhaseGroup(),
                *this->Phases[0]->GetName().ToString());
            this->Phases[0]->Start(DowncastedThisRef);
        }
    }

    /** Finish the authentication phase and proceed to the next one. This must be called from the server. */
    virtual void Finish(EAuthPhaseFailureCode ErrorCode) override
    {
        // Handle Finish scenarios on clients, where we need to disconnect from the server.
        {
            ENetDriverRole Role;
            if (this->GetRole(Role))
            {
                if (Role != ENetDriverRole::DedicatedServer && Role != ENetDriverRole::ListenServer)
                {
                    checkf(
                        ErrorCode != EAuthPhaseFailureCode::Success,
                        TEXT("IAuthPhaseContext::Finish can not be provided ErrorCode == Success on clients."));
                    checkf(
                        this->OnClientMustDisconnectFromServer.IsBound(),
                        TEXT("IAuthPhaseContext::Finish expected OnClientMustDisconnectFromServer to be bound on "
                             "client to route failure messages."));
                    OnClientMustDisconnectFromServer.Execute(ErrorCode, GetAuthPhaseFailureCodeString(ErrorCode));
                    return;
                }
            }
        }

        FString CurrentPhaseName = TEXT("");

        // This sometimes gets called because there are no phases to run, so
        // don't try and remove index 0 if we don't actually have phases.
        if (this->Phases.Num() > 0)
        {
            CurrentPhaseName = this->Phases[0]->GetName().ToString();
            this->Phases.RemoveAt(0);
        }

        if (ErrorCode != EAuthPhaseFailureCode::Success)
        {
            if (CurrentPhaseName.IsEmpty())
            {
                UE_LOG(
                    LogRedpointEOSNetworkAuth,
                    Error,
                    TEXT("%s: %s: Finished with failure code (0x%04x), connection will be terminated."),
                    *GetIdentifier(),
                    *GetPhaseGroup(),
                    ErrorCode);
            }
            else
            {
                UE_LOG(
                    LogRedpointEOSNetworkAuth,
                    Error,
                    TEXT("%s: %s: %s: Finished with failure code (0x%04x), connection will be terminated."),
                    *GetIdentifier(),
                    *GetPhaseGroup(),
                    *CurrentPhaseName,
                    ErrorCode);
            }

            OnCompleted().Broadcast(ErrorCode, GetAuthPhaseFailureCodeString(ErrorCode));
            this->SelfRef.Reset();
            return;
        }

        if (CurrentPhaseName.IsEmpty())
        {
            UE_LOG(
                LogRedpointEOSNetworkAuth,
                Verbose,
                TEXT("%s: %s: Finished successfully."),
                *GetIdentifier(),
                *GetPhaseGroup());
        }
        else
        {
            UE_LOG(
                LogRedpointEOSNetworkAuth,
                Verbose,
                TEXT("%s: %s: %s: Finished successfully."),
                *GetIdentifier(),
                *GetPhaseGroup(),
                *CurrentPhaseName);
        }

        if (this->Phases.Num() == 0)
        {
            UE_LOG(
                LogRedpointEOSNetworkAuth,
                Verbose,
                TEXT("%s: %s: All phases finished successfully."),
                *GetIdentifier(),
                *GetPhaseGroup());

            // Finished all phases.
            OnCompleted().Broadcast(ErrorCode, GetAuthPhaseFailureCodeString(ErrorCode));
            this->SelfRef.Reset();
        }
        else
        {
            UE_LOG(
                LogRedpointEOSNetworkAuth,
                Verbose,
                TEXT("%s: %s: %s: Starting..."),
                *GetIdentifier(),
                *GetPhaseGroup(),
                *this->Phases[0]->GetName().ToString());

            // Start next phase.
            this->Phases[0]->Start(this->SelfRef.ToSharedRef());
        }
    }
};

}

namespace Redpoint::EOS::Networking::Auth
{
REDPOINT_EOS_FILE_NS_EXPORT(3952495614, Redpoint::EOS::Networking::Auth, FAuthPhasesCompleted)
REDPOINT_EOS_FILE_NS_EXPORT(3952495614, Redpoint::EOS::Networking::Auth, FOnClientMustDisconnectFromServer)
REDPOINT_EOS_FILE_NS_EXPORT(3952495614, Redpoint::EOS::Networking::Auth, IAuthPhaseContext)
REDPOINT_EOS_FILE_NS_EXPORT(3952495614, Redpoint::EOS::Networking::Auth, FAuthPhaseContext)
}

REDPOINT_EOS_CODE_GUARD_END()