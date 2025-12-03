// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSNetworking/Socket/InternetAddrRedpointEOS.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "Containers/UnrealString.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSNetworking/Logging.h"
#include "RedpointEOSNetworking/Socket/P2PChannel.h"
#include "RedpointEOSNetworking/Socket/ProtocolType.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3518527788, Redpoint::EOS::Networking::Socket)
{
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Networking;
using namespace ::Redpoint::EOS::Networking::Socket;

FInternetAddrRedpointEOS::FInternetAddrRedpointEOS(
    UE::Online::FAccountId InUserId,
    const FSocketId &InSymmetricSocketName,
    uint8 InSymmetricChannel)
    : UserId(InUserId)
    , SymmetricSocketName(InSymmetricSocketName)
    , SymmetricChannel(InSymmetricChannel)
{
    check(IsProductUserId(this->UserId));
    check(this->SymmetricSocketName.Len() > 0 && this->SymmetricSocketName.Len() <= P2PSocketNameMaxLength);
}

FInternetAddrRedpointEOS::FInternetAddrRedpointEOS()
    : UserId()
    , SymmetricSocketName(TEXT(""))
    , SymmetricChannel(0)
{
}

void FInternetAddrRedpointEOS::SetFromParameters(
    UE::Online::FAccountId InUserId,
    const FSocketId &InSymmetricSocketName,
    uint8 InSymmetricChannel)
{
    check(IsProductUserId(InUserId));
    check(InSymmetricSocketName.Len() > 0 && InSymmetricSocketName.Len() <= P2PSocketNameMaxLength);

    this->UserId = InUserId;
    this->SymmetricSocketName = InSymmetricSocketName;
    this->SymmetricChannel = InSymmetricChannel;
}

UE::Online::FAccountId FInternetAddrRedpointEOS::GetUserId() const
{
    return this->UserId;
}

FSocketId FInternetAddrRedpointEOS::GetSymmetricSocketId() const
{
    return this->SymmetricSocketName;
}

void FInternetAddrRedpointEOS::SetIp(uint32 InAddr)
{
    UE_LOG(LogRedpointEOSNetworking, Error, TEXT("%s is not supported."), ANSI_TO_TCHAR(__FUNCTION__));
}

void FInternetAddrRedpointEOS::SetIp(const TCHAR *InAddr, bool &bIsValid)
{
    FString AddrAndPort = InAddr;

    int32 PortIdx;
    FP2PChannelId ParsedChannel;
    FString Addr;
    if (AddrAndPort.FindLastChar(TEXT(':'), PortIdx))
    {
        Addr = AddrAndPort.Left(PortIdx);
        ParsedChannel = (uint8)(FCString::Atoi(*AddrAndPort.Mid(PortIdx + 1)) % (int32)(P2PChannelIdModulo));
    }
    else
    {
        Addr = AddrAndPort;
        ParsedChannel = 0;
    }

    if (!Addr.EndsWith(FString::Printf(TEXT(".%s"), *P2PDomainSuffix)))
    {
        bIsValid = false;
        UE_LOG(
            LogRedpointEOSNetworking,
            Error,
            TEXT("FInternetAddrRedpointEOS::SetIp does not support plain IP addresses yet."));
        return;
    }

    TArray<FString> AddressComponents;
    Addr.ParseIntoArray(AddressComponents, TEXT("."), true);
    if (AddressComponents.Num() != 3)
    {
        bIsValid = false;
        UE_LOG(
            LogRedpointEOSNetworking,
            Error,
            TEXT("FInternetAddrRedpointEOS::SetIp received an incomplete address: '%s'"),
            *Addr);
        return;
    }

    auto ResultProductUserId = TryParseAccountId(AddressComponents[0]);
    if (!ResultProductUserId.IsSet())
    {
        bIsValid = false;
        UE_LOG(
            LogRedpointEOSNetworking,
            Error,
            TEXT("FInternetAddrRedpointEOS::SetIp did not have a valid product user ID. If you want to create an "
                 "address for binding locally, use the ID of the user that will be listening."));
        return;
    }

    FString ResultSocketName = AddressComponents[1];
    if (ResultSocketName.IsEmpty())
    {
        bIsValid = false;
        UE_LOG(
            LogRedpointEOSNetworking,
            Error,
            TEXT("FInternetAddrRedpointEOS::SetIp did not have a valid socket name."));
        return;
    }

    if (AddressComponents[2] != P2PDomainSuffix)
    {
        bIsValid = false;
        UE_LOG(
            LogRedpointEOSNetworking,
            Error,
            TEXT("FInternetAddrRedpointEOS::SetIp was missing '.%s' domain suffix."),
            *P2PDomainSuffix);
        return;
    }

    this->UserId = *ResultProductUserId;
    this->SymmetricSocketName = ResultSocketName;
    this->SymmetricChannel = ParsedChannel;
    bIsValid = true;
}

void FInternetAddrRedpointEOS::GetIp(uint32 &OutAddr) const
{
    UE_LOG(
        LogRedpointEOSNetworking,
        Error,
        TEXT("%s is not supported and will set OutAddr to 0."),
        ANSI_TO_TCHAR(__FUNCTION__));
    OutAddr = 0;
}

void FInternetAddrRedpointEOS::SetPort(int32 InPort)
{
    this->SymmetricChannel = (uint8)(InPort % (int32)(P2PChannelIdModulo));
}

int32 FInternetAddrRedpointEOS::GetPort() const
{
    return this->SymmetricChannel;
}

void FInternetAddrRedpointEOS::SetSymmetricChannel(FP2PChannelId InSymmetricChannel)
{
    this->SymmetricChannel = InSymmetricChannel;
}

FP2PChannelId FInternetAddrRedpointEOS::GetSymmetricChannel() const
{
    return this->SymmetricChannel;
}

void FInternetAddrRedpointEOS::SetRawIp(const TArray<uint8> &RawAddr)
{
    FMemoryReader Reader(RawAddr, true);

    FString ProductUserIdStr;
    FString SocketNameStr;
    FP2PChannelId ChannelRaw;

    Reader << ProductUserIdStr;
    Reader << SocketNameStr;
    Reader << ChannelRaw;

    auto ProductUserId = TryParseAccountId(ProductUserIdStr);
    if (!ProductUserId.IsSet())
    {
        UE_LOG(
            LogRedpointEOSNetworking,
            Error,
            TEXT("FInternetAddrRedpointEOS::SetRawIp did not have a valid product user ID. If you want to create an "
                 "address for binding locally, use the ID of the user that will be listening."));
        return;
    }

    if (SocketNameStr.IsEmpty())
    {
        UE_LOG(
            LogRedpointEOSNetworking,
            Error,
            TEXT("FInternetAddrRedpointEOS::SetRawIp did not have a valid socket name."));
        return;
    }

    this->UserId = *ProductUserId;
    this->SymmetricSocketName = SocketNameStr;
    this->SymmetricChannel = ChannelRaw;
}

TArray<uint8> FInternetAddrRedpointEOS::GetRawIp() const
{
    TArray<uint8> Result;
    FMemoryWriter Writer(Result, true);

    if (!IsProductUserId(this->UserId))
    {
        UE_LOG(
            LogRedpointEOSNetworking,
            Error,
            TEXT("FInternetAddrRedpointEOS::GetRawIp could not serialize the current user ID."));
        return TArray<uint8>();
    }

    auto ProductUserIdStr = GetUserIdString(this->UserId);

    FString SocketNameStr = FString(this->SymmetricSocketName);
    Writer << ProductUserIdStr;
    Writer << SocketNameStr;
    FP2PChannelId ChanLeft = (FP2PChannelId)this->SymmetricChannel;
    Writer << ChanLeft;

    Writer.Close();

    return Result;
}

void FInternetAddrRedpointEOS::SetAnyAddress()
{
    UE_LOG(LogRedpointEOSNetworking, Error, TEXT("%s is not supported."), ANSI_TO_TCHAR(__FUNCTION__));
}

void FInternetAddrRedpointEOS::SetBroadcastAddress()
{
    UE_LOG(LogRedpointEOSNetworking, Error, TEXT("%s is not supported."), ANSI_TO_TCHAR(__FUNCTION__));
}

void FInternetAddrRedpointEOS::SetLoopbackAddress()
{
    UE_LOG(LogRedpointEOSNetworking, Error, TEXT("%s is not supported."), ANSI_TO_TCHAR(__FUNCTION__));
}

FString FInternetAddrRedpointEOS::ToString(bool bAppendPort) const
{
    auto ProductUserIdStr = GetUserIdString(this->UserId);

    FString Result = FString::Printf(TEXT("%s.%s.%s"), *ProductUserIdStr, *this->SymmetricSocketName, *P2PDomainSuffix);
    if (bAppendPort)
    {
        Result = FString::Printf(TEXT("%s:%u"), *Result, this->SymmetricChannel);
    }
    return Result;
}

uint32 FInternetAddrRedpointEOS::GetTypeHash() const
{
    return GetTypeHashHelper(this->ToString(true));
}

bool FInternetAddrRedpointEOS::IsValid() const
{
    return IsProductUserId(this->UserId) && this->SymmetricSocketName.Len() > 0 &&
           this->SymmetricSocketName.Len() <= P2PSocketNameMaxLength;
}

TSharedRef<FInternetAddr> FInternetAddrRedpointEOS::Clone() const
{
    auto Clone = MakeShared<FInternetAddrRedpointEOS>();
    Clone->UserId = this->UserId;
    Clone->SymmetricSocketName = this->SymmetricSocketName;
    Clone->SymmetricChannel = this->SymmetricChannel;
    return Clone;
}

bool FInternetAddrRedpointEOS::operator==(const FInternetAddr &Other) const
{
    if (!Other.GetProtocolType().IsEqual(ProtocolType))
    {
        return false;
    }

    return Other.ToString(true) == this->ToString(true);
}

FName FInternetAddrRedpointEOS::GetProtocolType() const
{
    return ProtocolType;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()