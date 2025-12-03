// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "IPAddress.h"
#include "Online/CoreOnline.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

class UNetDriver;

namespace REDPOINT_EOS_FILE_NS_ID(1078758172, Redpoint::EOS::Networking)
{
using namespace ::Redpoint::EOS::API;

class REDPOINTEOSNETWORKING_API INetworkingSystem : public IPlatformInstanceSystem
{
public:
    static FName GetSystemName();
    static TSharedRef<INetworkingSystem> ConstructSystem(
        FPlatformHandle InPlatformHandle,
        FPlatformInstanceEnvironmentRef InPlatformInstanceEnvironment);

public:
    INetworkingSystem() = default;
    UE_NONCOPYABLE(INetworkingSystem);
    virtual ~INetworkingSystem() override = default;

    /**
     * The delegate type used to signal changes in listening state.
     */
    typedef TMulticastDelegate<void(
        const UE::Online::FAccountId &ListeningUserId,
        const TSharedPtr<const FInternetAddr> &InternetAddr,
        const TArray<TSharedRef<const FInternetAddr>> &DeveloperInternetAddrs)>
        FOnListeningStateChanged;

    /**
     * An event which fires whenever the game starts or stops listening for inbound game connections to a multiplayer
     * map.
     *
     * The ListeningUserId will be the dedicated server user ID if there was no user signed into EOS or if the game is
     * listening for connections over IP.
     */
    virtual FOnListeningStateChanged &OnListeningStateChanged() = 0;

    /**
     * Get the current listening state of a local user or the dedicated server user ID.
     */
    virtual void GetListeningState(
        const UE::Online::FAccountId &ListeningUserId,
        TSharedPtr<const FInternetAddr> &OutInternetAddr,
        TArray<TSharedRef<const FInternetAddr>> &OutDeveloperInternetAddrs) = 0;

    /**
     * Add a new listening state for a local user or dedicated server user ID.
     */
    virtual void AddListeningState(
        const UE::Online::FAccountId &ListeningUserId,
        const TSharedRef<const FInternetAddr> &InternetAddr,
        const TArray<TSharedRef<const FInternetAddr>> &DeveloperInternetAddrs) = 0;

    /**
     * Remove an existing listening state for a local user or dedicated server user ID.
     */
    virtual void RemoveListeningState(
        const UE::Online::FAccountId &ListeningUserId,
        const TSharedRef<const FInternetAddr> &InternetAddr) = 0;

    /**
     * If the provided NetDriver is internally a networking driver for the Redpoint EOS plugin, this returns the known
     * local listening URL for that networking driver. For drivers that are listening over IP, this will be a localhost
     * address and port. For drivers that are listening over P2P, this will be the correct ".eosp2p" domain and port.
     *
     * This function is intended for internal use by automation tests, and should not generally be used in games.
     *
     * The return value is left unset if the networking driver is not the Redpoint EOS networking driver.
     */
    virtual TOptional<FString> GetNetDriverLocalListeningURL(UNetDriver *NetDriver, uint16 TargetPortForP2P) = 0;
};

}

namespace Redpoint::EOS::Networking
{
REDPOINT_EOS_FILE_NS_EXPORT(1078758172, Redpoint::EOS::Networking, INetworkingSystem)
}

REDPOINT_EOS_CODE_GUARD_END()