// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSNetworking/NetworkingSystemImpl.h"

#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSNetDriver.h"
#include "RedpointEOSNetworking/Logging.h"
#include "RedpointEOSNetworking/NetDriverImpl.h"
#include "RedpointEOSNetworking/Socket/P2PChannel.h"
#include "RedpointEOSNetworking/Socket/SocketRedpointEOS.h"
#include "Sockets.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(870658517, Redpoint::EOS::Networking)
{
using namespace ::Redpoint::EOS::Core::Id;
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
using namespace ::Redpoint::EOS::Networking::Socket;
#endif

INetworkingSystem::FOnListeningStateChanged &FNetworkingSystemImpl::OnListeningStateChanged()
{
    return this->OnListeningStateChangedDelegate;
}

void FNetworkingSystemImpl::GetListeningState(
    const UE::Online::FAccountId &ListeningUserId,
    TSharedPtr<const FInternetAddr> &OutInternetAddr,
    TArray<TSharedRef<const FInternetAddr>> &OutDeveloperInternetAddrs)
{
    auto *ListeningState = this->ListeningStates.Find(ListeningUserId);
    if (ListeningState == nullptr || !ListeningState->CurrentAddress.IsValid())
    {
        OutInternetAddr.Reset();
        OutDeveloperInternetAddrs.Empty();
        return;
    }

    auto &AddressState = ListeningState->Addresses.FindChecked(ListeningState->CurrentAddress.ToSharedRef());
    OutInternetAddr = ListeningState->CurrentAddress.ToSharedRef();
    OutDeveloperInternetAddrs = AddressState.DeveloperInternetAddrs;
    return;
}

void FNetworkingSystemImpl::AddListeningState(
    const UE::Online::FAccountId &ListeningUserId,
    const TSharedRef<const FInternetAddr> &InternetAddr,
    const TArray<TSharedRef<const FInternetAddr>> &DeveloperInternetAddrs)
{
    auto &ListeningState = this->ListeningStates.FindOrAdd(ListeningUserId);

    auto &AddressState = ListeningState.Addresses.FindOrAdd(InternetAddr);
    AddressState.ReferenceCount++;
    AddressState.DeveloperInternetAddrs = DeveloperInternetAddrs;

    if (!ListeningState.CurrentAddress.IsValid())
    {
        ListeningState.CurrentAddress = InternetAddr;
        UE_LOG(
            LogRedpointEOSNetworking,
            Verbose,
            TEXT("User '%s' is now listening on Internet address '%s' with %d developer addresses."),
            *GetUserIdDebugString(ListeningUserId),
            *InternetAddr->ToString(true),
            DeveloperInternetAddrs.Num());
        this->OnListeningStateChanged().Broadcast(ListeningUserId, InternetAddr, DeveloperInternetAddrs);
    }
    else if (*ListeningState.CurrentAddress == *InternetAddr)
    {
        // The only scenario that we should fire this event in is if we're updating the DeveloperInternetAddrs for the
        // current active address. We have to check if the development internet addresses are effectively different by
        // hashing the array in a stable way and comparing the hash.

        // this->OnListeningStateChanged().Broadcast(ListeningUserId, InternetAddr, DeveloperInternetAddrs);
    }
}

void FNetworkingSystemImpl::RemoveListeningState(
    const UE::Online::FAccountId &ListeningUserId,
    const TSharedRef<const FInternetAddr> &InternetAddr)
{
    auto *ListeningState = this->ListeningStates.Find(ListeningUserId);
    if (ListeningState == nullptr)
    {
        return;
    }

    auto *AddressState = ListeningState->Addresses.Find(InternetAddr);
    if (AddressState == nullptr)
    {
        return;
    }

    bool bWasAddressRemoved = false;
    AddressState->ReferenceCount--;
    if (AddressState->ReferenceCount == 0)
    {
        ListeningState->Addresses.Remove(InternetAddr);
        bWasAddressRemoved = true;
    }

    if (bWasAddressRemoved && ListeningState->CurrentAddress.IsValid() &&
        *ListeningState->CurrentAddress == *InternetAddr)
    {
        // See if there's another address to switch to.
        bool bSwitched = false;
        for (const auto &KV : ListeningState->Addresses)
        {
            if (KV.Value.ReferenceCount > 0)
            {
                ListeningState->CurrentAddress = KV.Key;
                UE_LOG(
                    LogRedpointEOSNetworking,
                    Verbose,
                    TEXT("User '%s' has stopped listening on previous Internet address and is now listening on "
                         "Internet address '%s' with %d developer addresses."),
                    *GetUserIdDebugString(ListeningUserId),
                    *KV.Key->ToString(true),
                    KV.Value.DeveloperInternetAddrs.Num());
                this->OnListeningStateChanged().Broadcast(ListeningUserId, KV.Key, KV.Value.DeveloperInternetAddrs);
                bSwitched = true;
                break;
            }
        }

        // There isn't, we're now no longer listening.
        if (!bSwitched)
        {
            ListeningState->CurrentAddress.Reset();
            UE_LOG(
                LogRedpointEOSNetworking,
                Verbose,
                TEXT("User '%s' has stopped listening on previous Internet address and is no longer listening on any "
                     "address."),
                *GetUserIdDebugString(ListeningUserId));
            this->OnListeningStateChanged().Broadcast(
                ListeningUserId,
                nullptr,
                TArray<TSharedRef<const FInternetAddr>>());
        }
    }
}

TOptional<FString> FNetworkingSystemImpl::GetNetDriverLocalListeningURL(UNetDriver *NetDriver, uint16 TargetPortForP2P)
{
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    auto *EOSNetDriver = Cast<URedpointEOSNetDriver>(NetDriver);

    if (IsValid(EOSNetDriver))
    {
        auto Impl = EOSNetDriver->GetImpl();

        if (Impl->IsIpMode())
        {
            TSharedPtr<FInternetAddr> Addr = Impl->GetSocketSubsystem(EOSNetDriver)->CreateInternetAddr();
            FSocket *Socket = EOSNetDriver->GetSocket();
            Socket->GetAddress(*Addr);
            if (Socket == nullptr)
            {
                return TOptional<FString>();
            }
            return FString::Printf(TEXT("127.0.0.1:%d"), Addr->GetPort());
        }
        else if (Impl->GetRegisteredListeningUser().IsSet())
        {
            // @note: Retrieving the port from the P2P socket breaks the 'Networking.OverP2P.TwoBeaconsAndOneReconnects'
            // test, and I suspect it's because the wrong beacon is being selected. For now, maintain our existing
            // behaviour in the previous networking/test code by having the P2P port passed directly in.
#if 0
            TSharedPtr<FInternetAddr> Addr = Impl->GetSocketSubsystem()->CreateInternetAddr();
            // @note: Calling 'EOSNetDriver->GetSocket()' is not valid in P2P mode, because that function only returns
            // IP sockets.
            auto Socket = EOSNetDriver->GetImpl()->Socket.Pin();
            if (!Socket.IsValid())
            {
                return TOptional<FString>();
            }
            Socket->GetAddress(*Addr);
#endif

            return FString::Printf(
                TEXT("%s.default.eosp2p:%d"),
                *GetUserIdString(Impl->GetRegisteredListeningUser().GetValue()),
                TargetPortForP2P % P2PChannelIdModulo);
        }
    }
#endif

    return TOptional<FString>();
}

}

REDPOINT_EOS_CODE_GUARD_END()