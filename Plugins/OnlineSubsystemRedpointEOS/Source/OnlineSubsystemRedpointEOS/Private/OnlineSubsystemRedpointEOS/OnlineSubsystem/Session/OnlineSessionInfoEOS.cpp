// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Session/OnlineSessionInfoEOS.h"

#include "OnlineBeaconHost.h"
#include "OnlineSubsystemRedpointEOS/Logging.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Session/AttributeNames.h"
#include "OnlineSubsystemRedpointEOS/Shared/EOSCommon.h"
#include "RedpointEOSConfig/Config.h"
#include "RedpointEOSCore/Legacy/EOSString.h"
#include "SocketSubsystem.h"
#if !UE_BUILD_SHIPPING
#include "Misc/CommandLine.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1903314576, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Session)
{
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS;

FOnlineSessionInfoEOS::FOnlineSessionInfoEOS(
    const FPlatformHandle &InPlatformHandle,
    EOS_HSessionDetails InHandle,
    EOS_SessionDetails_Info *InInfo)
    : PlatformHandle(InPlatformHandle)
    , Handle(InHandle)
    , Info(InInfo)
{
    check(this->Handle != nullptr);
    check(this->Info != nullptr);

    this->Id = MakeShared<FUniqueNetIdEOSSession>(this->Info->SessionId);
}

FOnlineSessionInfoEOS::FOnlineSessionInfoEOS(const TSharedRef<FUniqueNetIdEOSSession> &InId)
    : PlatformHandle()
    , Id(InId)
    , Handle(nullptr)
    , Info(nullptr)
{
}

FOnlineSessionInfoEOS::~FOnlineSessionInfoEOS()
{
    if (this->Handle != nullptr)
    {
        EOS_SessionDetails_Release(this->Handle);
    }
    if (this->Info != nullptr)
    {
        EOS_SessionDetails_Info_Release(this->Info);
    }
}

EOS_HSessionDetails FOnlineSessionInfoEOS::GetHandle() const
{
    return this->Handle;
}

bool FOnlineSessionInfoEOS::GetResolvedConnectString(FString &OutConnectionString, FName PortType) const
{
    if (this->Info == nullptr || this->Info->HostAddress == nullptr)
    {
        return false;
    }

    int OverridePortNumber = 0;
    if (PortType.IsEqual(NAME_GamePort) || PortType.IsEqual(NAME_None) || PortType.IsEqual(NAME_Default))
    {
        // No port override, use the port specified in the URL.
    }
    else if (PortType.IsEqual(NAME_BeaconPort))
    {
        // Override to the port specified by config for AOnlineBeaconHost.
        OverridePortNumber = GetDefault<AOnlineBeaconHost>()->ListenPort;
    }
    else if (!PortType.ToString().IsEmpty() && PortType.ToString().IsNumeric())
    {
        // Override to the port specified in PortType.
        OverridePortNumber = FCString::Atoi(*PortType.ToString());
    }
    else
    {
        // Check to see if we have a string attribute on the session
        // that starts with port_<name>.
        FString AttributeName = FString::Printf(TEXT("Port_%s"), *PortType.ToString());
        auto AttributeNameStr = EOSString_SessionModification_AttributeKey::ToAnsiString(AttributeName);
        EOS_SessionDetails_Attribute *OverrideAttribute = nullptr;
        EOS_SessionDetails_CopySessionAttributeByKeyOptions OverrideCopyOpts = {};
        OverrideCopyOpts.ApiVersion = EOS_SESSIONDETAILS_COPYSESSIONATTRIBUTEBYKEY_API_LATEST;
        OverrideCopyOpts.AttrKey = AttributeNameStr.Ptr.Get();
        if (EOS_SessionDetails_CopySessionAttributeByKey(this->Handle, &OverrideCopyOpts, &OverrideAttribute) !=
                EOS_EResult::EOS_Success ||
            OverrideAttribute == nullptr || OverrideAttribute->Data == nullptr ||
            OverrideAttribute->Data->ValueType != EOS_ESessionAttributeType::EOS_AT_STRING)
        {
            // Could not get port override by attribute.
            UE_LOG(
                LogOnlineSubsystemRedpointEOS,
                Warning,
                TEXT("GetResolvedConnectString requested named port '%s', but there is no session attribute called "
                     "'%s' with a string value."),
                *PortType.ToString(),
                *AttributeName);
            return false;
        }
        OverridePortNumber = FCString::Atoi(
            *EOSString_SessionModification_AttributeStringValue::FromUtf8String(OverrideAttribute->Data->Value.AsUtf8));
        if (OverridePortNumber == 0)
        {
            UE_LOG(
                LogOnlineSubsystemRedpointEOS,
                Warning,
                TEXT("GetResolvedConnectString requested named port '%s', but the session attribute called "
                     "'%s' did not have a valid value."),
                *PortType.ToString(),
                *AttributeName);
            return false;
        }
    }

    FURL OldURL;
    FURL URL(&OldURL, ANSI_TO_TCHAR(this->Info->HostAddress), ETravelType::TRAVEL_Absolute);
    FString Address = FString(ANSI_TO_TCHAR(this->Info->HostAddress));
    if (URL.Host.EndsWith(INTERNET_ADDR_EOS_P2P_DOMAIN_SUFFIX))
    {
        if (OverridePortNumber == 0)
        {
            // This is a P2P address, so it's already complete.
            OutConnectionString = Address;
        }
        else
        {
            // We need to modify the server port component of the P2P address.
            URL.Port = OverridePortNumber;
            OutConnectionString = URL.GetHostPortString();
            return true;
        }
        return true;
    }

    // Get the AddressUnreal attribute, which contains port information and the connection string.
    EOS_SessionDetails_Attribute *Attribute = nullptr;
    EOS_SessionDetails_CopySessionAttributeByKeyOptions CopyOpts = {};
    CopyOpts.ApiVersion = EOS_SESSIONDETAILS_COPYSESSIONATTRIBUTEBYKEY_API_LATEST;
    CopyOpts.AttrKey = EOS_WELL_KNOWN_ATTRIBUTE_ADDRESS_BOUND;
    if (EOS_SessionDetails_CopySessionAttributeByKey(this->Handle, &CopyOpts, &Attribute) != EOS_EResult::EOS_Success ||
        Attribute->Data == nullptr || Attribute->Data->ValueType != EOS_ESessionAttributeType::EOS_AT_STRING)
    {
        // Could not get any additional information, return the HostAddress field as-is.
        OutConnectionString = Address;
        return true;
    }
    FString PortURLRaw = UTF8_TO_TCHAR(Attribute->Data->Value.AsUtf8);
    EOS_SessionDetails_Attribute_Release(Attribute);

    // Merge the EOS provided IP address and the port from AddressUnreal.
    TSharedPtr<FInternetAddr> PortURLAddr = ISocketSubsystem::Get()->CreateInternetAddr();
    TSharedPtr<FInternetAddr> HostURLAddr = ISocketSubsystem::Get()->CreateInternetAddr();
    bool bPortValid, bHostValid;
    PortURLAddr->SetIp(*PortURLRaw, bPortValid);
    HostURLAddr->SetIp(*Address, bHostValid);
    int ActivePortNumber = PortURLAddr->GetPort();
    if (OverridePortNumber != 0)
    {
        ActivePortNumber = OverridePortNumber;
    }
    HostURLAddr->SetPort(ActivePortNumber);

    OutConnectionString = HostURLAddr->ToString(true);

#if !UE_BUILD_SHIPPING
    if ((!this->PlatformHandle.IsValid() ||
         this->PlatformHandle->GetEnvironment()->Config->GetUseFallbackAddressesInDevelopment()) &&
        !FParse::Param(FCommandLine::Get(), TEXT("emulateeosshipping")))
    {
        // We support multiple IP addresses in our resolution because we're running a development build.
        // Lookup the dev addresses attribute, and append it onto our host URL.
        CopyOpts.AttrKey = EOS_WELL_KNOWN_ATTRIBUTE_ADDRESS_DEV;
        if (EOS_SessionDetails_CopySessionAttributeByKey(this->Handle, &CopyOpts, &Attribute) !=
                EOS_EResult::EOS_Success ||
            Attribute->Data == nullptr || Attribute->Data->ValueType != EOS_ESessionAttributeType::EOS_AT_STRING)
        {
            // Could not get development addresses. Return what we've already got.
            return true;
        }
        FString DevelopmentAddresses = UTF8_TO_TCHAR(Attribute->Data->Value.AsUtf8);
        EOS_SessionDetails_Attribute_Release(Attribute);

        if (DevelopmentAddresses.Len() > 0)
        {
            OutConnectionString = FString::Printf(
                TEXT("%s,%s:%d"),
                *HostURLAddr->ToString(false),
                *DevelopmentAddresses,
                ActivePortNumber);
        }
    }
#endif

    return true;
}

const FUniqueNetId &FOnlineSessionInfoEOS::GetSessionId() const
{
    return this->Id.ToSharedRef().Get();
}

const uint8 *FOnlineSessionInfoEOS::GetBytes() const
{
    return this->Id->GetBytes();
}

int32 FOnlineSessionInfoEOS::GetSize() const
{
    return this->Id->GetSize();
}

bool FOnlineSessionInfoEOS::IsValid() const
{
    return this->Id->IsValid();
}

FString FOnlineSessionInfoEOS::ToString() const
{
    return this->Id->ToString();
}

FString FOnlineSessionInfoEOS::ToDebugString() const
{
    return this->Id->ToDebugString();
}

}

REDPOINT_EOS_CODE_GUARD_END()