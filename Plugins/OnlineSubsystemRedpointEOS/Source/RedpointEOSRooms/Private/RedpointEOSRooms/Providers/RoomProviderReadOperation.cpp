// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Providers/RoomProviderReadOperation.h"

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSRooms/Logging.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1706010629, Redpoint::EOS::Rooms::Providers)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Providers;

#if !NO_LOGGING

void IRoomProviderReadOperation::InitializeDebugJson()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    using namespace ::Redpoint::EOS::Core::Id;

    // @note: We do not do checkf() here on the log verbosity level, as at least one customer reported a scenario where
    // the UE_LOG_ACTIVE expression returns a different value between InitializeOperation() and InitializeDebugJson().

    auto [Info, InfoError] = this->GetInfo();
    auto [Attributes, AttributesError] = this->GetAttributes();
    auto [Members, MembersError] = this->GetMembers();

    auto ReadObject = MakeShared<FJsonObject>();
    ReadObject->SetStringField("ActorUserId", GetUserIdString(this->GetActorUserId()));

    // Serialize the room information.
    if (!InfoError.WasSuccessful())
    {
        ReadObject->SetStringField("Info", InfoError.ToLogString());
    }
    else if (!Info.IsValid())
    {
        ReadObject->SetField("Info", MakeShared<FJsonValueNull>());
    }
    else
    {
        auto InfoObject = MakeShared<FJsonObject>();
        InfoObject->SetStringField("RoomProviderName", Info->GetRoomId()->GetProviderName().ToString());
        InfoObject->SetStringField("RoomId", Info->GetRoomId()->GetRoomId());
        if (Info->IsOwnerDedicatedServer())
        {
            InfoObject->SetBoolField("IsOwnerDedicatedServer", true);
            InfoObject->SetStringField("OwnerDedicatedServerClientId", Info->GetOwnerDedicatedServerClientId());
        }
        else
        {
            InfoObject->SetBoolField("IsOwnerDedicatedServer", false);
            InfoObject->SetStringField("OwnerUserId", GetUserIdString(Info->GetOwnerUserId()));
        }
        InfoObject->SetNumberField("MaxMembers", Info->GetMaxMembers());
        auto RawInfoObject = MakeShared<FJsonObject>();
        Info->InitializeDebugJson(RawInfoObject);
        InfoObject->SetObjectField("RawInfo", RawInfoObject);
        ReadObject->SetObjectField("Info", InfoObject);
    }

    // Serialize the room attributes.
    if (!AttributesError.WasSuccessful())
    {
        ReadObject->SetStringField("Attributes", AttributesError.ToLogString());
    }
    else
    {
        auto AttributesArray = TArray<TSharedPtr<FJsonValue>>();
        for (const auto &KV : Attributes)
        {
            auto AttributeObject = MakeShared<FJsonObject>();
            AttributeObject->SetStringField("AttributeKey", KV.Key);
            AttributeObject->SetStringField(
                "AttributeVisibility",
                KV.Value.GetLobbyAttributeVisibility() == EOS_ELobbyAttributeVisibility::EOS_LAT_PUBLIC ? "Public"
                                                                                                        : "Private");
            switch (KV.Value.Type())
            {
            case FRoomAttribute::EType::String:
                AttributeObject->SetStringField("AttributeValue", KV.Value.GetString());
                break;
            case FRoomAttribute::EType::Int64:
                AttributeObject->SetStringField(
                    "AttributeValue",
                    FString::Printf(TEXT("(int64) %lld"), KV.Value.GetInt64()));
                break;
            case FRoomAttribute::EType::Double:
                AttributeObject->SetNumberField("AttributeValue", KV.Value.GetDouble());
                break;
            case FRoomAttribute::EType::Bool:
                AttributeObject->SetBoolField("AttributeValue", KV.Value.GetBool());
                break;
            default:
                AttributeObject->SetField("AttributeValue", MakeShared<FJsonValueNull>());
                break;
            }
            AttributesArray.Add(MakeShared<FJsonValueObject>(AttributeObject));
        }
        ReadObject->SetArrayField("Attributes", AttributesArray);
    }

    // Serialize the room members.
    if (!MembersError.WasSuccessful())
    {
        ReadObject->SetStringField("Members", MembersError.ToLogString());
    }
    else
    {
        auto MembersObject = MakeShared<FJsonObject>();
        for (const auto &Member : Members)
        {
            auto MemberObject = MakeShared<FJsonObject>();
            auto [MemberAttributes, MemberAttributesError] = this->GetMemberAttributes(Member);
            if (!MemberAttributesError.WasSuccessful())
            {
                MemberObject->SetStringField("Attributes", MemberAttributesError.ToLogString());
            }
            else
            {
                auto MemberAttributesArray = TArray<TSharedPtr<FJsonValue>>();
                for (const auto &KV : MemberAttributes)
                {
                    auto MemberAttributeObject = MakeShared<FJsonObject>();
                    MemberAttributeObject->SetStringField("AttributeKey", KV.Key);
                    MemberAttributeObject->SetStringField(
                        "AttributeVisibility",
                        KV.Value.GetLobbyAttributeVisibility() == EOS_ELobbyAttributeVisibility::EOS_LAT_PUBLIC
                            ? "Public"
                            : "Private");
                    switch (KV.Value.Type())
                    {
                    case FRoomAttribute::EType::String:
                        MemberAttributeObject->SetStringField("AttributeValue", KV.Value.GetString());
                        break;
                    case FRoomAttribute::EType::Int64:
                        MemberAttributeObject->SetStringField(
                            "AttributeValue",
                            FString::Printf(TEXT("(int64) %lld"), KV.Value.GetInt64()));
                        break;
                    case FRoomAttribute::EType::Double:
                        MemberAttributeObject->SetNumberField("AttributeValue", KV.Value.GetDouble());
                        break;
                    case FRoomAttribute::EType::Bool:
                        MemberAttributeObject->SetBoolField("AttributeValue", KV.Value.GetBool());
                        break;
                    default:
                        MemberAttributeObject->SetField("AttributeValue", MakeShared<FJsonValueNull>());
                        break;
                    }
                    MemberAttributesArray.Add(MakeShared<FJsonValueObject>(MemberAttributeObject));
                }
                MemberObject->SetArrayField("Attributes", MemberAttributesArray);
            }
            MembersObject->SetObjectField(GetUserIdString(Member), MemberObject);
        }
        ReadObject->SetObjectField("Members", MembersObject);
    }

    this->GetMutableDebugJson()->SetObjectField("ReadOp", ReadObject);
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()