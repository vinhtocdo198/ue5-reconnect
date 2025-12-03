// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/AccountIdMap.h"
#include "RedpointEOSNetworking/InternetAddrMap.h"
#include "RedpointEOSNetworking/NetworkingSystem.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(870658517, Redpoint::EOS::Networking)
{
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Networking;

class FNetworkingSystemImpl : public INetworkingSystem, public TSharedFromThis<FNetworkingSystemImpl>
{
private:
    struct FAddressState
    {
        int32 ReferenceCount;
        TArray<TSharedRef<const FInternetAddr>> DeveloperInternetAddrs;

        FAddressState()
            : ReferenceCount()
            , DeveloperInternetAddrs()
        {
        }
    };

    struct FListeningState
    {
        TInternetAddrMap<FAddressState> Addresses;
        TSharedPtr<const FInternetAddr> CurrentAddress;

        FListeningState()
            : Addresses()
            , CurrentAddress()
        {
        }
    };

    TAccountIdMap<FListeningState> ListeningStates;

    FOnListeningStateChanged OnListeningStateChangedDelegate;

public:
    FNetworkingSystemImpl() = default;
    UE_NONCOPYABLE(FNetworkingSystemImpl);
    virtual ~FNetworkingSystemImpl() override = default;

    virtual FOnListeningStateChanged &OnListeningStateChanged() override;

    virtual void GetListeningState(
        const UE::Online::FAccountId &ListeningUserId,
        TSharedPtr<const FInternetAddr> &OutInternetAddr,
        TArray<TSharedRef<const FInternetAddr>> &OutDeveloperInternetAddrs) override;

    virtual void AddListeningState(
        const UE::Online::FAccountId &ListeningUserId,
        const TSharedRef<const FInternetAddr> &InternetAddr,
        const TArray<TSharedRef<const FInternetAddr>> &DeveloperInternetAddrs) override;

    virtual void RemoveListeningState(
        const UE::Online::FAccountId &ListeningUserId,
        const TSharedRef<const FInternetAddr> &InternetAddr) override;

    virtual TOptional<FString> GetNetDriverLocalListeningURL(UNetDriver *NetDriver, uint16 TargetPortForP2P) override;
};

}

namespace Redpoint::EOS::Networking
{
REDPOINT_EOS_FILE_NS_EXPORT(870658517, Redpoint::EOS::Networking, FNetworkingSystemImpl)
}

REDPOINT_EOS_CODE_GUARD_END()