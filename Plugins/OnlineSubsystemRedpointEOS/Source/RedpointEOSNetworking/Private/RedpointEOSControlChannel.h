// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Engine/ControlChannel.h"
#include "Interfaces/OnlineStatsInterface.h"
#include "Net/DataChannel.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSControlMessages.h"
#include "RedpointEOSCore/Id/AccountIdMap.h"
#include "RedpointEOSNetDriver.h"
#include "RedpointEOSNetworking/Auth/AuthPhase.h"
#include "RedpointEOSNetworking/Auth/AuthPhaseFailureCode.h"
#include "RedpointEOSNetworking/Auth/AuthPhaseRoute.h"
#include "RedpointEOSNetworking/Auth/UserVerificationStatus.h"

#include "RedpointEOSControlChannel.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(
    1581436555,
    Redpoint::EOS::Networking::Auth::Connection,
    FAuthConnectionPhaseContext)
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(
    1384477030,
    Redpoint::EOS::Networking::Auth::Verification,
    FAuthVerificationPhaseContext)
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(233779596, Redpoint::EOS::Networking::Auth::Login, FAuthLoginPhaseContext)
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(3050450103, Redpoint::EOS::Networking::Auth::Beacon, FAuthBeaconPhaseContext)
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(1850804168, Redpoint::EOS::Networking::Auth::Queues, FQueuedLoginEntry)
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(367523679, Redpoint::EOS::Networking::Auth::Queues, FQueuedBeaconEntry)
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(3315570403, Redpoint::EOS::Networking::Auth::Queues, IQueuedEntry)

UCLASS(Transient, CustomConstructor)
class URedpointEOSControlChannel : public UControlChannel
{
    friend ::Redpoint::EOS::Networking::Auth::Queues::FQueuedLoginEntry;
    friend ::Redpoint::EOS::Networking::Auth::Queues::FQueuedBeaconEntry;

    GENERATED_BODY()

public:
    void AddRoute(uint8 MessageType, const ::Redpoint::EOS::Networking::Auth::FAuthPhaseRoute &Route);
    TSharedPtr<::Redpoint::EOS::Networking::Auth::Connection::FAuthConnectionPhaseContext>
    GetAuthConnectionPhaseContext();
    TSharedPtr<::Redpoint::EOS::Networking::Auth::Verification::FAuthVerificationPhaseContext>
    GetAuthVerificationPhaseContext(const FUniqueNetIdRepl &InRepl);
    TSharedPtr<::Redpoint::EOS::Networking::Auth::Login::FAuthLoginPhaseContext> GetAuthLoginPhaseContext(
        const FUniqueNetIdRepl &InRepl);
    TSharedPtr<::Redpoint::EOS::Networking::Auth::Beacon::FAuthBeaconPhaseContext> GetAuthBeaconPhaseContext(
        const FUniqueNetIdRepl &InRepl,
        const FString &InBeaconName);

    bool bClientTrustsServer;
    ::Redpoint::EOS::Core::Id::TAccountIdMap<bool> bRegisteredForAntiCheat;
    ::Redpoint::EOS::Core::Id::TAccountIdMap<::Redpoint::EOS::Networking::Auth::EUserVerificationStatus>
        VerificationDatabase;

private:
    TMap<uint8, ::Redpoint::EOS::Networking::Auth::FAuthPhaseRoute> Routes;

    TSharedPtr<::Redpoint::EOS::Networking::Auth::Connection::FAuthConnectionPhaseContext> AuthConnectionContext;
    ::Redpoint::EOS::Core::Id::TAccountIdMap<
        TSharedPtr<::Redpoint::EOS::Networking::Auth::Verification::FAuthVerificationPhaseContext>>
        AuthVerificationContexts;
    ::Redpoint::EOS::Core::Id::TAccountIdMap<TSharedPtr<::Redpoint::EOS::Networking::Auth::Queues::FQueuedLoginEntry>>
        QueuedLogins;
    ::Redpoint::EOS::Core::Id::TAccountIdMap<
        TMap<FString, TSharedPtr<::Redpoint::EOS::Networking::Auth::Queues::FQueuedBeaconEntry>>>
        QueuedBeacons;

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    TOptional<UE::Online::FAccountId> CachedEACSourceUserId;
#endif

    struct FOriginalParameters_NMT_Hello
    {
        uint8 IsLittleEndian;
        uint32 RemoteNetworkVersion;
        FString EncryptionToken;
        EEngineNetworkRuntimeFeatures NetworkFeatures;
    };

    void On_NMT_Hello(const FOriginalParameters_NMT_Hello &Parameters);
    void Finalize_NMT_Hello(
        ::Redpoint::EOS::Networking::Auth::EAuthPhaseFailureCode Result,
        const FString &ErrorMessage,
        FOriginalParameters_NMT_Hello Parameters);

    void On_NMT_EOS_AntiCheatMessage(
        const UE::Online::FAccountId &SourceUserId,
        const UE::Online::FAccountId &TargetUserId,
        const TArray<uint8> &AntiCheatData);

    FDelegateHandle AuthStatusChangedHandle;
    FDelegateHandle ActionRequiredHandle;
    void OnAntiCheatPlayerAuthStatusChanged(
        UE::Online::FAccountId TargetUserId,
        EOS_EAntiCheatCommonClientAuthStatus NewAuthStatus);
    void OnAntiCheatPlayerActionRequired(
        UE::Online::FAccountId TargetUserId,
        EOS_EAntiCheatCommonClientAction ClientAction,
        EOS_EAntiCheatCommonClientActionReason ActionReasonCode,
        const FString &ActionReasonDetailsString);

    void StartAuthentication(
        const UE::Online::FAccountId &IncomingUser,
        const TSharedRef<::Redpoint::EOS::Networking::Auth::Queues::IQueuedEntry> &NetworkEntry,
        const FString &BeaconName,
        bool bIsBeacon);
    void OnAuthenticationComplete(
        ::Redpoint::EOS::Networking::Auth::EAuthPhaseFailureCode Result,
        const FString &ErrorMessage,
        TSharedRef<::Redpoint::EOS::Networking::Auth::Queues::IQueuedEntry> NetworkEntry);

    void On_NMT_Login(
        const FString &ClientResponse,
        const FString &URLString,
        const UE::Online::FAccountId &IncomingUser,
        const FString &OnlinePlatformNameString);
    void On_NMT_BeaconJoin(const FString &BeaconType, const UE::Online::FAccountId &IncomingUser);

    void OnClientMustDisconnectFromServer(
        ::Redpoint::EOS::Networking::Auth::EAuthPhaseFailureCode FailureCode,
        const FString &ErrorMessage);

public:
    URedpointEOSControlChannel(const FObjectInitializer &ObjectInitializer = FObjectInitializer::Get());

    virtual void Init(UNetConnection *InConnection, int32 InChIndex, EChannelCreateFlags CreateFlags) override;
    virtual bool CleanUp(const bool bForDestroy, EChannelCloseReason CloseReason) override;

    virtual void ReceivedBunch(FInBunch &Bunch) override;
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()