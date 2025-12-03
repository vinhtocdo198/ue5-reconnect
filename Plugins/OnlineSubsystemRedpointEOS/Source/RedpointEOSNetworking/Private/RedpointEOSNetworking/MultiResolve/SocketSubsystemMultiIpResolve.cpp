// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSNetworking/MultiResolve/SocketSubsystemMultiIpResolve.h"

#if !UE_BUILD_SHIPPING
#include "RedpointEOSNetworking/MultiResolve/GetAddressInfoTask.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !UE_BUILD_SHIPPING

namespace REDPOINT_EOS_FILE_NS_ID(1038774575, Redpoint::EOS::Networking::MultiIpResolve)
{
using namespace ::Redpoint::EOS::Networking::MultiIpResolve;

bool FSocketSubsystemMultiIpResolve::Init(FString &Error)
{
    return ISocketSubsystem::Get()->Init(Error);
}

void FSocketSubsystemMultiIpResolve::Shutdown()
{
    return ISocketSubsystem::Get()->Shutdown();
}

FSocket *FSocketSubsystemMultiIpResolve::CreateSocket(
    const FName &SocketType,
    const FString &SocketDescription,
    bool bForceUDP)
{
    return ISocketSubsystem::Get()->CreateSocket(SocketType, SocketDescription, bForceUDP);
}

FSocket *FSocketSubsystemMultiIpResolve::CreateSocket(
    const FName &SocketType,
    const FString &SocketDescription,
    ESocketProtocolFamily ProtocolType)
{
    PRAGMA_DISABLE_DEPRECATION_WARNINGS
    return ISocketSubsystem::Get()->CreateSocket(SocketType, SocketDescription, ProtocolType);
    PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

FSocket *FSocketSubsystemMultiIpResolve::CreateSocket(
    const FName &SocketType,
    const FString &SocketDescription,
    const FName &ProtocolName)
{
    return ISocketSubsystem::Get()->CreateSocket(SocketType, SocketDescription, ProtocolName);
}

class FResolveInfoCached *FSocketSubsystemMultiIpResolve::CreateResolveInfoCached(TSharedPtr<FInternetAddr> Addr) const
{
    return ISocketSubsystem::Get()->CreateResolveInfoCached(Addr);
}

void FSocketSubsystemMultiIpResolve::DestroySocket(FSocket *Socket)
{
    return ISocketSubsystem::Get()->DestroySocket(Socket);
}

FAddressInfoResult FSocketSubsystemMultiIpResolve::GetAddressInfo(
    const TCHAR *HostName,
    const TCHAR *ServiceName,
    EAddressInfoFlags QueryFlags,
    ESocketProtocolFamily ProtocolType,
    ESocketType SocketType)
{
    TArray<FAddressInfoResult> Results;
    TArray<FString> HostNames;
    FString(HostName).ParseIntoArray(HostNames, TEXT(","), true);

    // If we have no hostnames to query, just passthrough.
    if (HostNames.Num() == 0)
    {
        PRAGMA_DISABLE_DEPRECATION_WARNINGS
        return ISocketSubsystem::Get()->GetAddressInfo(HostName, ServiceName, QueryFlags, ProtocolType, SocketType);
        PRAGMA_ENABLE_DEPRECATION_WARNINGS
    }

    // Query each of the hostnames.
    for (const auto &HostNameEntry : HostNames)
    {
        PRAGMA_DISABLE_DEPRECATION_WARNINGS
        Results.Add(
            ISocketSubsystem::Get()->GetAddressInfo(*HostNameEntry, ServiceName, QueryFlags, ProtocolType, SocketType));
        PRAGMA_ENABLE_DEPRECATION_WARNINGS
    }

    // Merge all the results together.
    FAddressInfoResult FinalResult = Results[0];
    for (int i = 1; i < Results.Num(); i++)
    {
        for (const auto &Result : Results[i].Results)
        {
            FinalResult.Results.Add(Result);
        }
    }
    return FinalResult;
}

FAddressInfoResult FSocketSubsystemMultiIpResolve::GetAddressInfo(
    const TCHAR *HostName,
    const TCHAR *ServiceName,
    EAddressInfoFlags QueryFlags,
    const FName ProtocolTypeName,
    ESocketType SocketType)
{
    TArray<FAddressInfoResult> Results;
    TArray<FString> HostNames;
    FString(HostName).ParseIntoArray(HostNames, TEXT(","), true);

    // If we have no hostnames to query, just passthrough.
    if (HostNames.Num() == 0)
    {
        return ISocketSubsystem::Get()->GetAddressInfo(HostName, ServiceName, QueryFlags, ProtocolTypeName, SocketType);
    }

    // Query each of the hostnames.
    for (const auto &HostNameEntry : HostNames)
    {
        Results.Add(ISocketSubsystem::Get()
                        ->GetAddressInfo(*HostNameEntry, ServiceName, QueryFlags, ProtocolTypeName, SocketType));
    }

    // Merge all the results together.
    FAddressInfoResult FinalResult = Results[0];
    for (int i = 1; i < Results.Num(); i++)
    {
        for (const auto &Result : Results[i].Results)
        {
            FinalResult.Results.Add(Result);
        }
    }
    return FinalResult;
}

void FSocketSubsystemMultiIpResolve::GetAddressInfoAsync(
    FAsyncGetAddressInfoCallback Callback,
    const TCHAR *HostName,
    const TCHAR *ServiceName,
    EAddressInfoFlags QueryFlags,
    const FName ProtocolTypeName,
    ESocketType SocketType)
{
    (new FAutoDeleteAsyncTask<
         FGetAddressInfoTask>(this, HostName, ServiceName, QueryFlags, ProtocolTypeName, SocketType, Callback))
        ->StartBackgroundTask();
}

TSharedPtr<FInternetAddr> FSocketSubsystemMultiIpResolve::GetAddressFromString(const FString &InAddress)
{
    return ISocketSubsystem::Get()->GetAddressFromString(InAddress);
}

ESocketErrors FSocketSubsystemMultiIpResolve::GetHostByName(const ANSICHAR *HostName, FInternetAddr &OutAddr)
{
    PRAGMA_DISABLE_DEPRECATION_WARNINGS
    return ISocketSubsystem::Get()->GetHostByName(HostName, OutAddr);
    PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

class FResolveInfo *FSocketSubsystemMultiIpResolve::GetHostByName(const ANSICHAR *HostName)
{
    return ISocketSubsystem::Get()->GetHostByName(HostName);
}

bool FSocketSubsystemMultiIpResolve::RequiresChatDataBeSeparate()
{
    return ISocketSubsystem::Get()->RequiresChatDataBeSeparate();
}

bool FSocketSubsystemMultiIpResolve::RequiresEncryptedPackets()
{
    return ISocketSubsystem::Get()->RequiresEncryptedPackets();
}

bool FSocketSubsystemMultiIpResolve::GetHostName(FString &HostName)
{
    return ISocketSubsystem::Get()->GetHostName(HostName);
}

TSharedRef<FInternetAddr> FSocketSubsystemMultiIpResolve::CreateInternetAddr(uint32 Address, uint32 Port)
{
    PRAGMA_DISABLE_DEPRECATION_WARNINGS
    return ISocketSubsystem::Get()->CreateInternetAddr(Address, Port);
    PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

TSharedRef<FInternetAddr> FSocketSubsystemMultiIpResolve::CreateInternetAddr()
{
    return ISocketSubsystem::Get()->CreateInternetAddr();
}

TSharedRef<FInternetAddr> FSocketSubsystemMultiIpResolve::CreateInternetAddr(const FName ProtocolType)
{
    return ISocketSubsystem::Get()->CreateInternetAddr(ProtocolType);
}

TUniquePtr<FRecvMulti> FSocketSubsystemMultiIpResolve::CreateRecvMulti(
    int32 MaxNumPackets,
    int32 MaxPacketSize,
    ERecvMultiFlags Flags)
{
    return ISocketSubsystem::Get()->CreateRecvMulti(MaxNumPackets, MaxPacketSize, Flags);
}

bool FSocketSubsystemMultiIpResolve::HasNetworkDevice()
{
    return ISocketSubsystem::Get()->HasNetworkDevice();
}

const TCHAR *FSocketSubsystemMultiIpResolve::GetSocketAPIName() const
{
    return ISocketSubsystem::Get()->GetSocketAPIName();
}

ESocketErrors FSocketSubsystemMultiIpResolve::GetLastErrorCode()
{
    return ISocketSubsystem::Get()->GetLastErrorCode();
}

ESocketErrors FSocketSubsystemMultiIpResolve::TranslateErrorCode(int32 Code)
{
    return ISocketSubsystem::Get()->TranslateErrorCode(Code);
}

bool FSocketSubsystemMultiIpResolve::GetLocalAdapterAddresses(TArray<TSharedPtr<FInternetAddr>> &OutAddresses)
{
    return ISocketSubsystem::Get()->GetLocalAdapterAddresses(OutAddresses);
}

TSharedRef<FInternetAddr> FSocketSubsystemMultiIpResolve::GetLocalBindAddr(FOutputDevice &Out)
{
    return ISocketSubsystem::Get()->GetLocalBindAddr(Out);
}

TArray<TSharedRef<FInternetAddr>> FSocketSubsystemMultiIpResolve::GetLocalBindAddresses()
{
    return ISocketSubsystem::Get()->GetLocalBindAddresses();
}

TSharedRef<FInternetAddr> FSocketSubsystemMultiIpResolve::GetLocalHostAddr(FOutputDevice &Out, bool &bCanBindAll)
{
    return ISocketSubsystem::Get()->GetLocalHostAddr(Out, bCanBindAll);
}

bool FSocketSubsystemMultiIpResolve::GetMultihomeAddress(TSharedRef<FInternetAddr> &Addr)
{
    return ISocketSubsystem::Get()->GetMultihomeAddress(Addr);
}

bool FSocketSubsystemMultiIpResolve::IsSocketRecvMultiSupported() const
{
    return ISocketSubsystem::Get()->IsSocketRecvMultiSupported();
}

bool FSocketSubsystemMultiIpResolve::IsSocketWaitSupported() const
{
    return ISocketSubsystem::Get()->IsSocketWaitSupported();
}

double FSocketSubsystemMultiIpResolve::TranslatePacketTimestamp(
    const FPacketTimestamp &Timestamp,
    ETimestampTranslation Translation)
{
    return ISocketSubsystem::Get()->TranslatePacketTimestamp(Timestamp, Translation);
}

ESocketProtocolFamily FSocketSubsystemMultiIpResolve::GetProtocolFamilyFromName(const FName &InProtocolName) const
{
    if (InProtocolName == FNetworkProtocolTypes::IPv6)
    {
        return ESocketProtocolFamily::IPv6;
    }
    else if (InProtocolName == FNetworkProtocolTypes::IPv4)
    {
        return ESocketProtocolFamily::IPv4;
    }

    return ESocketProtocolFamily::None;
}

FName FSocketSubsystemMultiIpResolve::GetProtocolNameFromFamily(ESocketProtocolFamily InProtocolFamily) const
{
    switch (InProtocolFamily)
    {
    default:
        return NAME_None;
    case ESocketProtocolFamily::IPv4:
        return FNetworkProtocolTypes::IPv4;
    case ESocketProtocolFamily::IPv6:
        return FNetworkProtocolTypes::IPv6;
    }
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()