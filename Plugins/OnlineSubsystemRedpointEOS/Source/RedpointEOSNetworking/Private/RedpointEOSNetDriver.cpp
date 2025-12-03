// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSNetDriver.h"

#include "Misc/CommandLine.h"
#include "RedpointEOSNetworking/MultiResolve/SocketSubsystemMultiIpResolve.h"
#include "RedpointEOSNetworking/NetDriverImpl.h"
#if REDPOINT_EOS_UE_5_4_OR_LATER
#include "Net/NetworkMetricsDatabase.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

#define REDPOINT_EOS_CHECK_IMPL_SET()                                                                                  \
    checkf(                                                                                                            \
        this->Impl.IsValid(),                                                                                          \
        TEXT("%s: Expected NetDriverImpl to have been assigned already!"),                                             \
        ANSI_TO_TCHAR(__FUNCTION__))

bool URedpointEOSNetDriver::IsAvailable() const
{
    // @note: This is called on the CDO, where we won't have Impl set or any other information. The networking driver is
    // always available at the CDO level.
    return true;
}

bool URedpointEOSNetDriver::IsNetResourceValid()
{
    REDPOINT_EOS_CHECK_IMPL_SET();

    return this->Impl->IsNetResourceValid();
}

ISocketSubsystem *URedpointEOSNetDriver::GetSocketSubsystem()
{
    REDPOINT_EOS_CHECK_IMPL_SET();

    return this->Impl->GetSocketSubsystem(this);
}

void URedpointEOSNetDriver::PostInitProperties()
{
    Super::PostInitProperties();

    // @note: The implementation sets up NetDriver properties as part of it's constructor, when the implementation is
    // constructed during OnNetDriverCreated.
    this->Impl = nullptr;
}

bool URedpointEOSNetDriver::InitConnect(FNetworkNotify *InNotify, const FURL &ConnectURL, FString &Error)
{
    REDPOINT_EOS_CHECK_IMPL_SET();

    return this->Impl->InitConnect(InNotify, ConnectURL, Error);
}

bool URedpointEOSNetDriver::InitListen(
    FNetworkNotify *InNotify,
    FURL &ListenURL,
    bool bReuseAddressAndPort,
    FString &Error)
{
    REDPOINT_EOS_CHECK_IMPL_SET();

    return this->Impl->InitListen(InNotify, ListenURL, bReuseAddressAndPort, Error);
}

void URedpointEOSNetDriver::InitConnectionlessHandler()
{
    REDPOINT_EOS_CHECK_IMPL_SET();

    this->Impl->InitConnectionlessHandler();
    return;
}

void URedpointEOSNetDriver::TickDispatch(float DeltaTime)
{
    REDPOINT_EOS_CHECK_IMPL_SET();

    this->Impl->TickDispatch(DeltaTime);
    return;
}

void URedpointEOSNetDriver::TickFlush(float DeltaTime)
{
    REDPOINT_EOS_CHECK_IMPL_SET();

    this->Impl->TickFlush(DeltaTime);
    return;
}

void URedpointEOSNetDriver::LowLevelSend(
    TSharedPtr<const FInternetAddr> Address,
    void *Data,
    int32 CountBits,
    FOutPacketTraits &Traits)
{
    REDPOINT_EOS_CHECK_IMPL_SET();

    this->Impl->LowLevelSend(Address, Data, CountBits, Traits);
    return;
}

#if REDPOINT_EOS_UE_5_4_OR_LATER

namespace NetDriverAccess_Shutdown
{
template <typename Tag, typename Tag::type M> struct Rob
{
    friend typename Tag::type get(Tag)
    {
        return M;
    }
};
struct UNetDriver_NetworkMetricsDatabase
{
    typedef TObjectPtr<UNetworkMetricsDatabase> UNetDriver::*type;
    friend type get(UNetDriver_NetworkMetricsDatabase);
};
template struct Rob<UNetDriver_NetworkMetricsDatabase, &UNetDriver::NetworkMetricsDatabase>;
}

void URedpointEOSNetDriver::Shutdown()
{
    using namespace NetDriverAccess_Shutdown;

    REDPOINT_EOS_CHECK_IMPL_SET();

    // If the UNetDriver::InitBase didn't run, or the host or connect otherwise failed, the base net driver may not have
    // set up the NetworkMetricsDatabase field. The UNetDriver::Shutdown implementation does not guard on this value
    // being initialized, which means we get a hard crash. If it isn't set up, make sure we create it before running the
    // base Shutdown call.
    if (!this->GetMetrics())
    {
        this->*get(UNetDriver_NetworkMetricsDatabase()) = NewObject<UNetworkMetricsDatabase>();
    }

    UNetDriver::Shutdown();
}

#endif

void URedpointEOSNetDriver::LowLevelDestroy()
{
    REDPOINT_EOS_CHECK_IMPL_SET();

    // @note: At the point LowLevelDestroy is called, the TSoftObjectPtr to the NetDriver inside the Impl is no longer
    // valid because the NetDriver is marked for garbage collection. Thus we pass the raw pointer as a parameter for use
    // inside LowLevelDestroy instead.
    this->Impl->LowLevelDestroy(this);
    return;
}

TSharedRef<::Redpoint::EOS::Networking::INetDriver> URedpointEOSNetDriver::GetImpl() const
{
    REDPOINT_EOS_CHECK_IMPL_SET();

    return this->Impl.ToSharedRef();
}

bool URedpointEOSNetDriver::UIpNetDriver_InitConnect(FNetworkNotify *InNotify, const FURL &ConnectURL, FString &Error)
{
    return UIpNetDriver::InitConnect(InNotify, ConnectURL, Error);
}

bool URedpointEOSNetDriver::UIpNetDriver_InitListen(
    FNetworkNotify *InNotify,
    FURL &LocalURL,
    bool bReuseAddressAndPort,
    FString &Error)
{
    return UIpNetDriver::InitListen(InNotify, LocalURL, bReuseAddressAndPort, Error);
}

bool URedpointEOSNetDriver::UIpNetDriver_IsNetResourceValid()
{
    return UIpNetDriver::IsNetResourceValid();
}

ISocketSubsystem *URedpointEOSNetDriver::UIpNetDriver_GetSocketSubsystem()
{
    return UIpNetDriver::GetSocketSubsystem();
}

void URedpointEOSNetDriver::UIpNetDriver_InitConnectionlessHandler()
{
    UIpNetDriver::InitConnectionlessHandler();
}

void URedpointEOSNetDriver::UIpNetDriver_TickDispatch(float DeltaTime)
{
    UIpNetDriver::TickDispatch(DeltaTime);
}

void URedpointEOSNetDriver::UIpNetDriver_TickFlush(float DeltaTime)
{
    UIpNetDriver::TickFlush(DeltaTime);
}

void URedpointEOSNetDriver::UIpNetDriver_LowLevelSend(
    TSharedPtr<const FInternetAddr> Address,
    void *Data,
    int32 CountBits,
    FOutPacketTraits &Traits)
{
    UIpNetDriver::LowLevelSend(Address, Data, CountBits, Traits);
}

void URedpointEOSNetDriver::UIpNetDriver_LowLevelDestroy()
{
    UIpNetDriver::LowLevelDestroy();
}

bool URedpointEOSNetDriver::UNetDriver_InitBase(
    bool bInitAsClient,
    FNetworkNotify *InNotify,
    const FURL &URL,
    bool bReuseAddressAndPort,
    FString &Error)
{
    return UNetDriver::InitBase(bInitAsClient, InNotify, URL, bReuseAddressAndPort, Error);
}

void URedpointEOSNetDriver::UNetDriver_TickDispatch(float DeltaTime)
{
    UNetDriver::TickDispatch(DeltaTime);
}

#undef REDPOINT_EOS_CHECK_IMPL_SET

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()