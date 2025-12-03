// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Session/OnlineSessionSearchResultEOS.h"

#include "OnlineSubsystemRedpointEOS/Logging.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Session/AttributeNames.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Session/OnlineSessionInfoEOS.h"
#include "OnlineSubsystemRedpointEOS/Shared/EOSCommon.h"
#include "RedpointEOSCore/Id/UniqueNetIdEOS.h"
#include "RedpointEOSCore/Legacy/EOSString.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(92848785, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Session)
{
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Session;

FOnlineSessionSearchResultEOS FOnlineSessionSearchResultEOS::CreateInvalid()
{
    return FOnlineSessionSearchResultEOS();
}

FOnlineSessionSearchResultEOS FOnlineSessionSearchResultEOS::CreateFromDetails(
    const FPlatformHandle &InPlatformHandle,
    EOS_HSessionDetails InHandle)
{
    check(InHandle != nullptr);

    EOS_SessionDetails_CopyInfoOptions CopyInfoOpts = {};
    CopyInfoOpts.ApiVersion = EOS_SESSIONDETAILS_COPYINFO_API_LATEST;

    EOS_SessionDetails_Info *SessionDetailsInfo = nullptr;
    if (EOS_SessionDetails_CopyInfo(InHandle, &CopyInfoOpts, &SessionDetailsInfo) != EOS_EResult::EOS_Success)
    {
        EOS_SessionDetails_Release(InHandle);
        return FOnlineSessionSearchResultEOS();
    }

    check(SessionDetailsInfo != nullptr);
    check(SessionDetailsInfo->Settings != nullptr);

    // Read bUsesPresence from attribute data if possible.
    bool bUsesPresence = false;
    {
        EOS_SessionDetails_Attribute *PresenceAttribute = nullptr;
        EOS_SessionDetails_CopySessionAttributeByKeyOptions CopyByKeyOpts = {};
        CopyByKeyOpts.ApiVersion = EOS_SESSIONDETAILS_COPYSESSIONATTRIBUTEBYKEY_API_LATEST;
        CopyByKeyOpts.AttrKey = EOS_WELL_KNOWN_ATTRIBUTE_BUSESPRESENCE;
        if (EOS_SessionDetails_CopySessionAttributeByKey(InHandle, &CopyByKeyOpts, &PresenceAttribute) ==
            EOS_EResult::EOS_Success)
        {
            if (PresenceAttribute->Data != nullptr &&
                PresenceAttribute->Data->ValueType == EOS_ESessionAttributeType::EOS_AT_BOOLEAN)
            {
                bUsesPresence = PresenceAttribute->Data->Value.AsBool == EOS_TRUE;
            }
        }
        if (PresenceAttribute != nullptr)
        {
            EOS_SessionDetails_Attribute_Release(PresenceAttribute);
            PresenceAttribute = nullptr;
        }
    }

    FOnlineSessionSettings Settings;
    Settings.NumPublicConnections = static_cast<int32>(SessionDetailsInfo->Settings->NumPublicConnections);
    Settings.NumPrivateConnections = 0;
    Settings.bShouldAdvertise =
        SessionDetailsInfo->Settings->PermissionLevel == EOS_EOnlineSessionPermissionLevel::EOS_OSPF_PublicAdvertised;
    Settings.bAllowJoinInProgress = SessionDetailsInfo->Settings->bAllowJoinInProgress == EOS_TRUE;
    Settings.bIsLANMatch = false;
    Settings.bIsDedicated = !EOSString_SessionModification_HostAddress::FromAnsiString(SessionDetailsInfo->HostAddress)
                                 .EndsWith(FString::Printf(TEXT(".%s"), INTERNET_ADDR_EOS_P2P_DOMAIN_SUFFIX));
    Settings.bUsesStats = false;
    Settings.bAllowInvites = SessionDetailsInfo->Settings->bInvitesAllowed == EOS_TRUE;
    Settings.bUsesPresence = bUsesPresence;
    Settings.bAllowJoinViaPresence =
        SessionDetailsInfo->Settings->PermissionLevel == EOS_EOnlineSessionPermissionLevel::EOS_OSPF_JoinViaPresence ||
        SessionDetailsInfo->Settings->PermissionLevel == EOS_EOnlineSessionPermissionLevel::EOS_OSPF_PublicAdvertised;
    Settings.bAllowJoinViaPresenceFriendsOnly =
        SessionDetailsInfo->Settings->PermissionLevel == EOS_EOnlineSessionPermissionLevel::EOS_OSPF_JoinViaPresence;
    Settings.bAntiCheatProtected = false;
    Settings.BuildUniqueId = 0; // @todo: build ID filtering

    EOS_SessionDetails_GetSessionAttributeCountOptions CountOpts = {};
    CountOpts.ApiVersion = EOS_SESSIONDETAILS_GETSESSIONATTRIBUTECOUNT_API_LATEST;
    uint32_t AttributeCount = EOS_SessionDetails_GetSessionAttributeCount(InHandle, &CountOpts);
    for (uint32_t i = 0; i < AttributeCount; i++)
    {
        EOS_SessionDetails_CopySessionAttributeByIndexOptions CopyOpts = {};
        CopyOpts.ApiVersion = EOS_SESSIONDETAILS_COPYSESSIONATTRIBUTEBYINDEX_API_LATEST;
        CopyOpts.AttrIndex = i;

        EOS_SessionDetails_Attribute *Attribute = nullptr;
        if (EOS_SessionDetails_CopySessionAttributeByIndex(InHandle, &CopyOpts, &Attribute) != EOS_EResult::EOS_Success)
        {
            UE_LOG(LogOnlineSubsystemRedpointEOS, Warning, TEXT("Unable to copy session attribute at index %u"), i);
            continue;
        }

        FName Key = FName(ANSI_TO_TCHAR(Attribute->Data->Key));
        EOnlineDataAdvertisementType::Type AdvertType =
            Attribute->AdvertisementType == EOS_ESessionAttributeAdvertisementType::EOS_SAAT_Advertise
                ? EOnlineDataAdvertisementType::ViaOnlineService
                : EOnlineDataAdvertisementType::DontAdvertise;
        FOnlineSessionSetting Value;
        bool ValueValid = true;
        switch (Attribute->Data->ValueType)
        {
        case EOS_ESessionAttributeType::EOS_AT_BOOLEAN:
            Value = FOnlineSessionSetting(Attribute->Data->Value.AsBool == EOS_TRUE, AdvertType);
            break;
        case EOS_ESessionAttributeType::EOS_AT_DOUBLE:
            Value = FOnlineSessionSetting(Attribute->Data->Value.AsDouble, AdvertType);
            break;
        case EOS_ESessionAttributeType::EOS_AT_INT64:
            Value = FOnlineSessionSetting((int64)Attribute->Data->Value.AsInt64, AdvertType);
            break;
        case EOS_ESessionAttributeType::EOS_AT_STRING:
            Value = FOnlineSessionSetting(FString(UTF8_TO_TCHAR(Attribute->Data->Value.AsUtf8)), AdvertType);
            break;
        default:
            UE_LOG(LogOnlineSubsystemRedpointEOS, Warning, TEXT("Unable to copy session attribute at index %u"), i);
            ValueValid = false;
            break;
        }
        if (!ValueValid)
        {
            EOS_SessionDetails_Attribute_Release(Attribute);
            continue;
        }

        Settings.Settings.Add(Key, Value);
        EOS_SessionDetails_Attribute_Release(Attribute);
    }

    FOnlineSessionSearchResultEOS Result;
    Result.Session.OwningUserId = SessionDetailsInfo->OwnerUserId != nullptr
                                      ? MakeShared<FUniqueNetIdEOS>(SessionDetailsInfo->OwnerUserId)
                                      : FUniqueNetIdEOS::DedicatedServerId();
    Result.Session.OwningUserName = TEXT("");
    Result.Session.SessionSettings = Settings;
    Result.Session.SessionInfo = MakeShared<FOnlineSessionInfoEOS>(InPlatformHandle, InHandle, SessionDetailsInfo);
    Result.Session.NumOpenPrivateConnections = 0;
    Result.Session.NumOpenPublicConnections = static_cast<int32>(SessionDetailsInfo->NumOpenPublicConnections);
    Result.PingInMs = 0;
    return Result;
}

}

REDPOINT_EOS_CODE_GUARD_END()