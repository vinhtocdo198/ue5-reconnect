// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPresence/PresenceUserStateSerialization.h"

#include "RedpointEOSRooms/Features/PublicAdvertisementRoomFeature.h"
#include "RedpointEOSRooms/Features/PublicAdvertisementRoomFeatureRequest.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1199869237, Redpoint::EOS::Presence)
{
using namespace ::Redpoint::EOS::Rooms::Features;

void FPresenceUserStateSerialization::SerializeForCreate(
    FCreateRoomRequest &Request,
    bool bParty,
    const TOptional<FPresenceUserAdvertisedRoom> &DesiredLocalUserAdvertisedRoom)
{
    auto Prefix = bParty ? TEXT("AdvertisedParty") : TEXT("AdvertisedSession");

    if (DesiredLocalUserAdvertisedRoom.IsSet())
    {
        Request.Attributes.Add(
            FString::Printf(TEXT("%sId"), Prefix),
            DesiredLocalUserAdvertisedRoom->RoomId->ToString());
        Request.Attributes.Add(
            FString::Printf(TEXT("%sNamespace"), Prefix),
            DesiredLocalUserAdvertisedRoom->RoomNamespace.ToString());
        Request.Attributes.Add(
            FString::Printf(TEXT("%sCurrentMemberCount"), Prefix),
            static_cast<int64>(DesiredLocalUserAdvertisedRoom->CurrentMemberCount));
        Request.Attributes.Add(
            FString::Printf(TEXT("%sMaximumMemberCount"), Prefix),
            static_cast<int64>(DesiredLocalUserAdvertisedRoom->MaximumMemberCount));
        Request.Attributes.Add(FString::Printf(TEXT("%sJoinable"), Prefix), DesiredLocalUserAdvertisedRoom->bJoinable);
        Request.Attributes.Add(
            FString::Printf(TEXT("%sSecretPassword"), Prefix),
            FRoomAttribute(DesiredLocalUserAdvertisedRoom->SecretPassword, FRoomAttribute::EVisibility::Private));
        Request.Attributes.Add(
            FString::Printf(TEXT("%sRequiresSecretPassword"), Prefix),
            DesiredLocalUserAdvertisedRoom->bRequiresSecretPassword);
        Request.Attributes.Add(FString::Printf(TEXT("%sIsMatch"), Prefix), DesiredLocalUserAdvertisedRoom->bIsMatch);
        Request.Attributes.Add(
            FString::Printf(TEXT("%sMatchStarted"), Prefix),
            DesiredLocalUserAdvertisedRoom->bMatchStarted);
    }
}

void FPresenceUserStateSerialization::SerializeForUpdate(
    FUpdateRoomRequest &Request,
    const FRoomRef &CurrentPresenceRoom,
    bool bParty,
    const TOptional<FPresenceUserAdvertisedRoom> &DesiredLocalUserAdvertisedRoom)
{
    auto Prefix = bParty ? TEXT("AdvertisedParty") : TEXT("AdvertisedSession");

    if (DesiredLocalUserAdvertisedRoom.IsSet())
    {
        Request.UpsertAttributes.Add(
            FString::Printf(TEXT("%sId"), Prefix),
            DesiredLocalUserAdvertisedRoom->RoomId->ToString());
        Request.UpsertAttributes.Add(
            FString::Printf(TEXT("%sNamespace"), Prefix),
            DesiredLocalUserAdvertisedRoom->RoomNamespace.ToString());
        Request.UpsertAttributes.Add(
            FString::Printf(TEXT("%sCurrentMemberCount"), Prefix),
            static_cast<int64>(DesiredLocalUserAdvertisedRoom->CurrentMemberCount));
        Request.UpsertAttributes.Add(
            FString::Printf(TEXT("%sMaximumMemberCount"), Prefix),
            static_cast<int64>(DesiredLocalUserAdvertisedRoom->MaximumMemberCount));
        Request.UpsertAttributes.Add(
            FString::Printf(TEXT("%sJoinable"), Prefix),
            DesiredLocalUserAdvertisedRoom->bJoinable);
        Request.UpsertAttributes.Add(
            FString::Printf(TEXT("%sSecretPassword"), Prefix),
            FRoomAttribute(DesiredLocalUserAdvertisedRoom->SecretPassword, FRoomAttribute::EVisibility::Private));
        Request.UpsertAttributes.Add(
            FString::Printf(TEXT("%sRequiresSecretPassword"), Prefix),
            DesiredLocalUserAdvertisedRoom->bRequiresSecretPassword);
        Request.UpsertAttributes.Add(
            FString::Printf(TEXT("%sIsMatch"), Prefix),
            DesiredLocalUserAdvertisedRoom->bIsMatch);
        Request.UpsertAttributes.Add(
            FString::Printf(TEXT("%sMatchStarted"), Prefix),
            DesiredLocalUserAdvertisedRoom->bMatchStarted);
    }
    else
    {
        Request.DeleteAttributes.Add(FString::Printf(TEXT("%sId"), Prefix));
        Request.DeleteAttributes.Add(FString::Printf(TEXT("%sNamespace"), Prefix));
        Request.DeleteAttributes.Add(FString::Printf(TEXT("%sCurrentMemberCount"), Prefix));
        Request.DeleteAttributes.Add(FString::Printf(TEXT("%sMaximumMemberCount"), Prefix));
        Request.DeleteAttributes.Add(FString::Printf(TEXT("%sJoinable"), Prefix));
        Request.DeleteAttributes.Add(FString::Printf(TEXT("%sSecretPassword"), Prefix));
        Request.DeleteAttributes.Add(FString::Printf(TEXT("%sRequiresSecretPassword"), Prefix));
        Request.DeleteAttributes.Add(FString::Printf(TEXT("%sIsMatch"), Prefix));
        Request.DeleteAttributes.Add(FString::Printf(TEXT("%sMatchStarted"), Prefix));
    }
}

TOptional<FPresenceUserAdvertisedRoom> FPresenceUserStateSerialization::Deserialize(
    const FIdentityUserId &TargetAccountId,
    const FRoomSearchResultRef &RoomSearchResult,
    bool bParty)
{
    auto Prefix = bParty ? TEXT("AdvertisedParty") : TEXT("AdvertisedSession");

    const auto &RoomAttributes = RoomSearchResult->GetRoomAttributes();

    TOptional<FPresenceUserAdvertisedRoom> AdvertisedRoom;
    {
        auto *IdAttribute = RoomAttributes.Find(FString::Printf(TEXT("%sId"), Prefix));
        auto *NamespaceAttribute = RoomAttributes.Find(FString::Printf(TEXT("%sNamespace"), Prefix));
        if (IdAttribute != nullptr && IdAttribute->Type() == FRoomAttribute::EType::String &&
            NamespaceAttribute != nullptr && NamespaceAttribute->Type() == FRoomAttribute::EType::String)
        {
            auto RoomId = FRoomId::FromString(IdAttribute->GetString());
            if (RoomId.IsValid())
            {
                AdvertisedRoom =
                    FPresenceUserAdvertisedRoom(RoomId.ToSharedRef(), FName(*NamespaceAttribute->GetString()));
            }
            else
            {
                return TOptional<FPresenceUserAdvertisedRoom>();
            }
        }
        else
        {
            return TOptional<FPresenceUserAdvertisedRoom>();
        }
    }

    {
        auto *Attribute = RoomAttributes.Find(FString::Printf(TEXT("%sCurrentMemberCount"), Prefix));
        if (Attribute != nullptr && Attribute->Type() == FRoomAttribute::EType::Int64)
        {
            AdvertisedRoom->CurrentMemberCount = static_cast<uint32>(Attribute->GetInt64());
        }
    }

    {
        auto *Attribute = RoomAttributes.Find(FString::Printf(TEXT("%sMaximumMemberCount"), Prefix));
        if (Attribute != nullptr && Attribute->Type() == FRoomAttribute::EType::Int64)
        {
            AdvertisedRoom->MaximumMemberCount = static_cast<uint32>(Attribute->GetInt64());
        }
    }

    {
        auto *Attribute = RoomAttributes.Find(FString::Printf(TEXT("%sJoinable"), Prefix));
        if (Attribute != nullptr && Attribute->Type() == FRoomAttribute::EType::Bool)
        {
            AdvertisedRoom->bJoinable = Attribute->GetBool();
        }
    }

    {
        auto *Attribute = RoomAttributes.Find(FString::Printf(TEXT("%sSecretPassword"), Prefix));
        if (Attribute != nullptr && Attribute->Type() == FRoomAttribute::EType::String)
        {
            AdvertisedRoom->SecretPassword = Attribute->GetString();
        }
    }

    {
        auto *Attribute = RoomAttributes.Find(FString::Printf(TEXT("%sRequiresSecretPassword"), Prefix));
        if (Attribute != nullptr && Attribute->Type() == FRoomAttribute::EType::Bool)
        {
            AdvertisedRoom->bRequiresSecretPassword = Attribute->GetBool();
        }
    }

    {
        auto *Attribute = RoomAttributes.Find(FString::Printf(TEXT("%sIsMatch"), Prefix));
        if (Attribute != nullptr && Attribute->Type() == FRoomAttribute::EType::Bool)
        {
            AdvertisedRoom->bIsMatch = Attribute->GetBool();
        }
    }

    {
        auto *Attribute = RoomAttributes.Find(FString::Printf(TEXT("%sMatchStarted"), Prefix));
        if (Attribute != nullptr && Attribute->Type() == FRoomAttribute::EType::Bool)
        {
            AdvertisedRoom->bMatchStarted = Attribute->GetBool();
        }
    }

    return AdvertisedRoom;
}

void FPresenceUserStateSerialization::SerializeForCreate(
    FCreateRoomRequest &Request,
    const FPresenceUserState &DesiredLocalUserStateSnapshot)
{
    if (!DesiredLocalUserStateSnapshot.bPresenceAdvertised)
    {
    }
    else
    {
        Request.AddFeatureRequest(MakeShared<FPublicAdvertisementRoomFeatureRequest>());
    }
    FPresenceUserStateSerialization::SerializeForCreate(
        Request,
        false,
        DesiredLocalUserStateSnapshot.AdvertisedSession);
    FPresenceUserStateSerialization::SerializeForCreate(Request, true, DesiredLocalUserStateSnapshot.AdvertisedParty);
    switch (DesiredLocalUserStateSnapshot.ActivityState)
    {
    case EPresenceUserActivityState::Offline:
        Request.Attributes.Add(TEXT("ActivityState"), TEXT("Offline"));
        break;
    case EPresenceUserActivityState::DoNotDisturb:
        Request.Attributes.Add(TEXT("ActivityState"), TEXT("DoNotDisturb"));
        break;
    case EPresenceUserActivityState::Idle:
        Request.Attributes.Add(TEXT("ActivityState"), TEXT("Idle"));
        break;
    case EPresenceUserActivityState::Online:
    default:
        Request.Attributes.Add(TEXT("ActivityState"), TEXT("Online"));
        break;
    }
    FTextId StatusTextFormatTextId =
        FTextInspector::GetTextId(DesiredLocalUserStateSnapshot.StatusTextFormat.GetSourceText());
#if REDPOINT_EOS_UE_5_5_OR_LATER
    Request.Attributes.Add(TEXT("StatusTextFormatTextId_Key"), *StatusTextFormatTextId.GetKey().ToString());
    Request.Attributes.Add(TEXT("StatusTextFormatTextId_Namespace"), *StatusTextFormatTextId.GetNamespace().ToString());
#else
    Request.Attributes.Add(TEXT("StatusTextFormatTextId_Key"), StatusTextFormatTextId.GetKey().GetChars());
    Request.Attributes.Add(TEXT("StatusTextFormatTextId_Namespace"), StatusTextFormatTextId.GetNamespace().GetChars());
#endif
    TSet<FString> ExpectedStatusTextFormatArgumentAttributes;
    for (const auto &KV : DesiredLocalUserStateSnapshot.StatusTextFormatArguments)
    {
        auto AttributeTypeKey = FString::Printf(TEXT("StatusTextFormatArgument_Type_%s"), *KV.Key);
        auto AttributeKeyKey = FString::Printf(TEXT("StatusTextFormatArgument_Key_%s"), *KV.Key);
        auto AttributeValueKey = FString::Printf(TEXT("StatusTextFormatArgument_Value_%s"), *KV.Key);
        ExpectedStatusTextFormatArgumentAttributes.Add(AttributeTypeKey);
        ExpectedStatusTextFormatArgumentAttributes.Add(AttributeKeyKey);
        ExpectedStatusTextFormatArgumentAttributes.Add(AttributeValueKey);
        Request.Attributes.Add(AttributeKeyKey, KV.Key);
        switch (KV.Value.GetType())
        {
        case EFormatArgumentType::Int:
            Request.Attributes.Add(AttributeTypeKey, TEXT("Int"));
            Request.Attributes.Add(AttributeValueKey, KV.Value.GetIntValue());
            break;
        case EFormatArgumentType::UInt:
            Request.Attributes.Add(AttributeTypeKey, TEXT("UInt"));
            Request.Attributes.Add(AttributeValueKey, static_cast<int64>(KV.Value.GetUIntValue()));
            break;
        case EFormatArgumentType::Float:
            Request.Attributes.Add(AttributeTypeKey, TEXT("Float"));
            Request.Attributes.Add(AttributeValueKey, static_cast<double>(KV.Value.GetFloatValue()));
            break;
        case EFormatArgumentType::Double:
            Request.Attributes.Add(AttributeTypeKey, TEXT("Double"));
            Request.Attributes.Add(AttributeValueKey, KV.Value.GetDoubleValue());
            break;
        case EFormatArgumentType::Text: {
            auto AttributeValueKeyKey = FString::Printf(TEXT("StatusTextFormatArgument_ValueKey_%s"), *KV.Key);
            auto AttributeValueKeyNs = FString::Printf(TEXT("StatusTextFormatArgument_ValueNs_%s"), *KV.Key);
            FTextId StatusTextFormatArgumentTextId = FTextInspector::GetTextId(KV.Value.GetTextValue());
            Request.Attributes.Add(AttributeTypeKey, TEXT("Text"));
#if REDPOINT_EOS_UE_5_5_OR_LATER
            Request.Attributes.Add(AttributeValueKeyKey, StatusTextFormatArgumentTextId.GetKey().ToString());
            Request.Attributes.Add(AttributeValueKeyNs, StatusTextFormatArgumentTextId.GetNamespace().ToString());
#else
            Request.Attributes.Add(AttributeValueKeyKey, StatusTextFormatArgumentTextId.GetKey().GetChars());
            Request.Attributes.Add(AttributeValueKeyNs, StatusTextFormatArgumentTextId.GetNamespace().GetChars());
#endif
            ExpectedStatusTextFormatArgumentAttributes.Remove(AttributeValueKey);
            ExpectedStatusTextFormatArgumentAttributes.Add(AttributeValueKeyKey);
            ExpectedStatusTextFormatArgumentAttributes.Add(AttributeValueKeyNs);
            break;
        }
        case EFormatArgumentType::Gender:
            Request.Attributes.Add(AttributeTypeKey, TEXT("Gender"));
            switch (KV.Value.GetGenderValue())
            {
            case ETextGender::Feminine:
                Request.Attributes.Add(AttributeValueKey, TEXT("F"));
                break;
            case ETextGender::Masculine:
                Request.Attributes.Add(AttributeValueKey, TEXT("M"));
                break;
            case ETextGender::Neuter:
            default:
                Request.Attributes.Add(AttributeValueKey, TEXT("N"));
                break;
            }
            break;
        default:
            ExpectedStatusTextFormatArgumentAttributes.Remove(AttributeTypeKey);
            ExpectedStatusTextFormatArgumentAttributes.Remove(AttributeValueKey);
            break;
        }
    }
    TSet<FString> ExpectedCustomAttributeKeys;
    for (const auto &KV : DesiredLocalUserStateSnapshot.CustomAttributes)
    {
        auto Key = FString::Printf(TEXT("Custom_%s"), *KV.Key);
        ExpectedCustomAttributeKeys.Add(Key);
        Request.Attributes.Add(Key, FRoomAttribute(KV.Value, FRoomAttribute::EVisibility::Public));
    }
}

void FPresenceUserStateSerialization::SerializeForUpdate(
    FUpdateRoomRequest &Request,
    const FRoomRef &CurrentPresenceRoom,
    const FPresenceUserState &DesiredLocalUserStateSnapshot)
{
    if (!DesiredLocalUserStateSnapshot.bPresenceAdvertised)
    {
        if (CurrentPresenceRoom->HasRoomFeature<IPublicAdvertisementRoomFeature>())
        {
            Request.DeleteFeature(FPublicAdvertisementRoomFeatureRequest::GetFeatureNameStatic());
        }
    }
    else
    {
        if (!CurrentPresenceRoom->HasRoomFeature<IPublicAdvertisementRoomFeature>())
        {
            Request.AddFeatureRequest(MakeShared<FPublicAdvertisementRoomFeatureRequest>());
        }
    }
    FPresenceUserStateSerialization::SerializeForUpdate(
        Request,
        CurrentPresenceRoom,
        false,
        DesiredLocalUserStateSnapshot.AdvertisedSession);
    FPresenceUserStateSerialization::SerializeForUpdate(
        Request,
        CurrentPresenceRoom,
        true,
        DesiredLocalUserStateSnapshot.AdvertisedParty);
    switch (DesiredLocalUserStateSnapshot.ActivityState)
    {
    case EPresenceUserActivityState::Offline:
        Request.UpsertAttributes.Add(TEXT("ActivityState"), TEXT("Offline"));
        break;
    case EPresenceUserActivityState::DoNotDisturb:
        Request.UpsertAttributes.Add(TEXT("ActivityState"), TEXT("DoNotDisturb"));
        break;
    case EPresenceUserActivityState::Idle:
        Request.UpsertAttributes.Add(TEXT("ActivityState"), TEXT("Idle"));
        break;
    case EPresenceUserActivityState::Online:
    default:
        Request.UpsertAttributes.Add(TEXT("ActivityState"), TEXT("Online"));
        break;
    }
    FTextId StatusTextFormatTextId =
        FTextInspector::GetTextId(DesiredLocalUserStateSnapshot.StatusTextFormat.GetSourceText());
#if REDPOINT_EOS_UE_5_5_OR_LATER
    Request.UpsertAttributes.Add(TEXT("StatusTextFormatTextId_Key"), *StatusTextFormatTextId.GetKey().ToString());
    Request.UpsertAttributes.Add(
        TEXT("StatusTextFormatTextId_Namespace"),
        *StatusTextFormatTextId.GetNamespace().ToString());
#else
    Request.UpsertAttributes.Add(TEXT("StatusTextFormatTextId_Key"), StatusTextFormatTextId.GetKey().GetChars());
    Request.UpsertAttributes.Add(
        TEXT("StatusTextFormatTextId_Namespace"),
        StatusTextFormatTextId.GetNamespace().GetChars());
#endif
    TSet<FString> ExpectedStatusTextFormatArgumentAttributes;
    for (const auto &KV : DesiredLocalUserStateSnapshot.StatusTextFormatArguments)
    {
        auto AttributeTypeKey = FString::Printf(TEXT("StatusTextFormatArgument_Type_%s"), *KV.Key);
        auto AttributeKeyKey = FString::Printf(TEXT("StatusTextFormatArgument_Key_%s"), *KV.Key);
        auto AttributeValueKey = FString::Printf(TEXT("StatusTextFormatArgument_Value_%s"), *KV.Key);
        ExpectedStatusTextFormatArgumentAttributes.Add(AttributeTypeKey);
        ExpectedStatusTextFormatArgumentAttributes.Add(AttributeKeyKey);
        ExpectedStatusTextFormatArgumentAttributes.Add(AttributeValueKey);
        Request.UpsertAttributes.Add(AttributeKeyKey, KV.Key);
        switch (KV.Value.GetType())
        {
        case EFormatArgumentType::Int:
            Request.UpsertAttributes.Add(AttributeTypeKey, TEXT("Int"));
            Request.UpsertAttributes.Add(AttributeValueKey, KV.Value.GetIntValue());
            break;
        case EFormatArgumentType::UInt:
            Request.UpsertAttributes.Add(AttributeTypeKey, TEXT("UInt"));
            Request.UpsertAttributes.Add(AttributeValueKey, static_cast<int64>(KV.Value.GetUIntValue()));
            break;
        case EFormatArgumentType::Float:
            Request.UpsertAttributes.Add(AttributeTypeKey, TEXT("Float"));
            Request.UpsertAttributes.Add(AttributeValueKey, static_cast<double>(KV.Value.GetFloatValue()));
            break;
        case EFormatArgumentType::Double:
            Request.UpsertAttributes.Add(AttributeTypeKey, TEXT("Double"));
            Request.UpsertAttributes.Add(AttributeValueKey, KV.Value.GetDoubleValue());
            break;
        case EFormatArgumentType::Text: {
            auto AttributeValueKeyKey = FString::Printf(TEXT("StatusTextFormatArgument_ValueKey_%s"), *KV.Key);
            auto AttributeValueKeyNs = FString::Printf(TEXT("StatusTextFormatArgument_ValueNs_%s"), *KV.Key);
            FTextId StatusTextFormatArgumentTextId = FTextInspector::GetTextId(KV.Value.GetTextValue());
            Request.UpsertAttributes.Add(AttributeTypeKey, TEXT("Text"));
#if REDPOINT_EOS_UE_5_5_OR_LATER
            Request.UpsertAttributes.Add(AttributeValueKeyKey, StatusTextFormatArgumentTextId.GetKey().ToString());
            Request.UpsertAttributes.Add(AttributeValueKeyNs, StatusTextFormatArgumentTextId.GetNamespace().ToString());
#else
            Request.UpsertAttributes.Add(AttributeValueKeyKey, StatusTextFormatArgumentTextId.GetKey().GetChars());
            Request.UpsertAttributes.Add(AttributeValueKeyNs, StatusTextFormatArgumentTextId.GetNamespace().GetChars());
#endif
            ExpectedStatusTextFormatArgumentAttributes.Remove(AttributeValueKey);
            ExpectedStatusTextFormatArgumentAttributes.Add(AttributeValueKeyKey);
            ExpectedStatusTextFormatArgumentAttributes.Add(AttributeValueKeyNs);
            break;
        }
        case EFormatArgumentType::Gender:
            Request.UpsertAttributes.Add(AttributeTypeKey, TEXT("Gender"));
            switch (KV.Value.GetGenderValue())
            {
            case ETextGender::Feminine:
                Request.UpsertAttributes.Add(AttributeValueKey, TEXT("F"));
                break;
            case ETextGender::Masculine:
                Request.UpsertAttributes.Add(AttributeValueKey, TEXT("M"));
                break;
            case ETextGender::Neuter:
            default:
                Request.UpsertAttributes.Add(AttributeValueKey, TEXT("N"));
                break;
            }
            break;
        default:
            ExpectedStatusTextFormatArgumentAttributes.Remove(AttributeTypeKey);
            ExpectedStatusTextFormatArgumentAttributes.Remove(AttributeValueKey);
            break;
        }
    }
    for (const auto &KV : CurrentPresenceRoom->GetRoomAttributes())
    {
        if (KV.Key.StartsWith(TEXT("StatusTextFormatArgument_")) &&
            !ExpectedStatusTextFormatArgumentAttributes.Contains(KV.Key))
        {
            Request.DeleteAttributes.Add(KV.Key);
        }
    }
    TSet<FString> ExpectedCustomAttributeKeys;
    for (const auto &KV : DesiredLocalUserStateSnapshot.CustomAttributes)
    {
        auto Key = FString::Printf(TEXT("Custom_%s"), *KV.Key);
        ExpectedCustomAttributeKeys.Add(Key);
        Request.UpsertAttributes.Add(Key, FRoomAttribute(KV.Value, FRoomAttribute::EVisibility::Public));
    }
    for (const auto &KV : CurrentPresenceRoom->GetRoomAttributes())
    {
        if (KV.Key.StartsWith(TEXT("Custom_")) && !ExpectedCustomAttributeKeys.Contains(KV.Key))
        {
            Request.DeleteAttributes.Add(KV.Key);
        }
    }
}

FPresenceUserState FPresenceUserStateSerialization::Deserialize(
    const FIdentityUserId &TargetAccountId,
    const FRoomSearchResultRef &RoomSearchResult)
{
    const auto &RoomAttributes = RoomSearchResult->GetRoomAttributes();

    auto PresenceState = FPresenceUserState(TargetAccountId);
    PresenceState.bPresenceAdvertised = true;
    PresenceState.bIsPlayingThisGame = true;

    PresenceState.AdvertisedSession =
        FPresenceUserStateSerialization::Deserialize(TargetAccountId, RoomSearchResult, false);
    PresenceState.AdvertisedParty =
        FPresenceUserStateSerialization::Deserialize(TargetAccountId, RoomSearchResult, true);

    {
        auto *Attribute = RoomAttributes.Find(TEXT("ActivityState"));
        if (Attribute != nullptr && Attribute->Type() == FRoomAttribute::EType::String)
        {
            auto ActivityState = Attribute->GetString();
            if (ActivityState == TEXT("Offline"))
            {
                PresenceState.ActivityState = EPresenceUserActivityState::Offline;
            }
            else if (ActivityState == TEXT("DoNotDisturb"))
            {
                PresenceState.ActivityState = EPresenceUserActivityState::DoNotDisturb;
            }
            else if (ActivityState == TEXT("Idle"))
            {
                PresenceState.ActivityState = EPresenceUserActivityState::Idle;
            }
            else if (ActivityState == TEXT("Online"))
            {
                PresenceState.ActivityState = EPresenceUserActivityState::Online;
            }
            else
            {
                // Unknown; treat as Online.
                PresenceState.ActivityState = EPresenceUserActivityState::Online;
            }
        }
        else
        {
            // Not set; treat as Online (they can't truly be offline - we have a presence room for them).
            PresenceState.ActivityState = EPresenceUserActivityState::Online;
        }
    }

    {
        auto *NamespaceAttribute = RoomAttributes.Find(TEXT("StatusTextFormatTextId_Namespace"));
        auto *KeyAttribute = RoomAttributes.Find(TEXT("StatusTextFormatTextId_Key"));
        if (NamespaceAttribute != nullptr && NamespaceAttribute->Type() == FRoomAttribute::EType::String &&
            KeyAttribute != nullptr && KeyAttribute->Type() == FRoomAttribute::EType::String)
        {
            FText FoundText;
#if REDPOINT_EOS_UE_5_5_OR_LATER
            if (FText::FindTextInLiveTable_Advanced(
                    FTextKey(NamespaceAttribute->GetString()),
                    FTextKey(KeyAttribute->GetString()),
                    FoundText))
#else
            if (FText::FindText(
                    FTextKey(NamespaceAttribute->GetString()),
                    FTextKey(KeyAttribute->GetString()),
                    FoundText))
#endif
            {
                PresenceState.StatusTextFormat = FTextFormat(FoundText);
            }
            else
            {
                PresenceState.StatusTextFormat = FTextFormat(FText::GetEmpty());
            }
        }
        else
        {
            PresenceState.StatusTextFormat = FTextFormat(FText::GetEmpty());
        }
    }

    // Deserialize text format arguments and custom attributes at the same time.
    for (const auto &KV : RoomAttributes)
    {
        if (KV.Key.StartsWith(TEXT("StatusTextFormatArgument_Type_")))
        {
            if (KV.Value.Type() != FRoomAttribute::EType::String)
            {
                continue;
            }

            auto ArgumentNameSuffix = KV.Key.Mid(FString(TEXT("StatusTextFormatArgument_Type_")).Len());
            auto &TypeAttribute = KV.Value;
            auto TypeAttributeString = TypeAttribute.GetString();

            // @note: The keys are always returned uppercase from the EOS SDK backend, but we need the correct casing so
            // that FText::Format works correctly when the text string has an argument with non-uppercase characters in
            // the name.
            auto *KeyAttribute =
                RoomAttributes.Find(FString::Printf(TEXT("StatusTextFormatArgument_Key_%s"), *ArgumentNameSuffix));
            if (KeyAttribute == nullptr || KeyAttribute->Type() != FRoomAttribute::EType::String)
            {
                continue;
            }
            auto ArgumentName = KeyAttribute->GetString();
            if (!ArgumentName.Equals(ArgumentNameSuffix, ESearchCase::IgnoreCase))
            {
                // Prevent StatusTextFormatArgument_Key_... from being set to a value that doesn't at least match in a
                // case insensitive manner.
                continue;
            }

            if (TypeAttributeString == TEXT("Text"))
            {
                // Value keys are 'ValueKey_' and 'ValueNs_'.
                auto *ValueNamespaceAttribute = RoomAttributes.Find(
                    FString::Printf(TEXT("StatusTextFormatArgument_ValueNs_%s"), *ArgumentNameSuffix));
                auto *ValueKeyAttribute = RoomAttributes.Find(
                    FString::Printf(TEXT("StatusTextFormatArgument_ValueKey_%s"), *ArgumentNameSuffix));
                if (ValueNamespaceAttribute == nullptr || ValueKeyAttribute == nullptr ||
                    ValueNamespaceAttribute->Type() != FRoomAttribute::EType::String ||
                    ValueKeyAttribute->Type() != FRoomAttribute::EType::String)
                {
                    PresenceState.StatusTextFormatArguments.Add(ArgumentName, FFormatArgumentValue());
                    continue;
                }

                FText FoundText;
#if REDPOINT_EOS_UE_5_5_OR_LATER
                if (FText::FindTextInLiveTable_Advanced(
                        FTextKey(ValueNamespaceAttribute->GetString()),
                        FTextKey(ValueKeyAttribute->GetString()),
                        FoundText))
#else
                if (FText::FindText(
                        FTextKey(ValueNamespaceAttribute->GetString()),
                        FTextKey(ValueKeyAttribute->GetString()),
                        FoundText))
#endif
                {
                    PresenceState.StatusTextFormatArguments.Add(ArgumentName, FFormatArgumentValue(FoundText));
                }
                else
                {
                    PresenceState.StatusTextFormatArguments.Add(ArgumentName, FFormatArgumentValue(FText::GetEmpty()));
                }
            }
            else
            {
                // Value key is 'Value_'.
                auto *ValueAttribute =
                    RoomAttributes.Find(FString::Printf(TEXT("StatusTextFormatArgument_Value_%s"), *ArgumentName));
                if (ValueAttribute == nullptr)
                {
                    PresenceState.StatusTextFormatArguments.Add(ArgumentName, FFormatArgumentValue());
                    continue;
                }

                if (TypeAttributeString == TEXT("Int") && ValueAttribute->Type() == FRoomAttribute::EType::Int64)
                {
                    PresenceState.StatusTextFormatArguments.Add(
                        ArgumentName,
                        FFormatArgumentValue(ValueAttribute->GetInt64()));
                }
                else if (TypeAttributeString == TEXT("UInt") && ValueAttribute->Type() == FRoomAttribute::EType::Int64)
                {
                    PresenceState.StatusTextFormatArguments.Add(
                        ArgumentName,
                        FFormatArgumentValue(static_cast<uint64>(ValueAttribute->GetInt64())));
                }
                else if (
                    TypeAttributeString == TEXT("Float") && ValueAttribute->Type() == FRoomAttribute::EType::Double)
                {
                    PresenceState.StatusTextFormatArguments.Add(
                        ArgumentName,
                        FFormatArgumentValue(static_cast<float>(ValueAttribute->GetDouble())));
                }
                else if (
                    TypeAttributeString == TEXT("Double") && ValueAttribute->Type() == FRoomAttribute::EType::Double)
                {
                    PresenceState.StatusTextFormatArguments.Add(
                        ArgumentName,
                        FFormatArgumentValue(ValueAttribute->GetDouble()));
                }
                else if (
                    TypeAttributeString == TEXT("Gender") && ValueAttribute->Type() == FRoomAttribute::EType::String)
                {
                    auto Gender = ValueAttribute->GetString();
                    if (Gender == TEXT("F"))
                    {
                        PresenceState.StatusTextFormatArguments.Add(
                            ArgumentName,
                            FFormatArgumentValue(ETextGender::Feminine));
                    }
                    else if (Gender == TEXT("F"))
                    {
                        PresenceState.StatusTextFormatArguments.Add(
                            ArgumentName,
                            FFormatArgumentValue(ETextGender::Masculine));
                    }
                    else
                    {
                        PresenceState.StatusTextFormatArguments.Add(
                            ArgumentName,
                            FFormatArgumentValue(ETextGender::Neuter));
                    }
                }
                else
                {
                    PresenceState.StatusTextFormatArguments.Add(ArgumentName, FFormatArgumentValue());
                }
            }

            continue;
        }
        else if (KV.Key.StartsWith(TEXT("Custom_")))
        {
            PresenceState.CustomAttributes.Add(KV.Key.Mid(FString(TEXT("Custom_")).Len()), KV.Value);
        }
    }

    return PresenceState;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()