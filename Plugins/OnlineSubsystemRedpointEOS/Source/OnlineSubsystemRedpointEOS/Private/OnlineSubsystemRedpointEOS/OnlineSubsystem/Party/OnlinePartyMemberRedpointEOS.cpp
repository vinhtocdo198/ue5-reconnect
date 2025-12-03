// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Party/OnlinePartyMemberRedpointEOS.h"

#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSCore/Id/UniqueNetIdEOS.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(621763951, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party)
{
using namespace ::Redpoint::EOS::Core::Id;

TSharedRef<FOnlineUserEOS> FOnlinePartyMemberRedpointEOS::GetOnlineUserEOSInstance(
    TSharedRef<FEOSUserFactory> InUserFactory,
    TSharedRef<const FUniqueNetIdEOS> InLocalUserId,
    FRoomUserId InMemberId)
{
    TArray<TSharedRef<const FUniqueNetIdEOS>> MemberIds;
    MemberIds.Add(GetUniqueNetId(InMemberId).ToSharedRef());
    auto ResultMap = FOnlineUserEOS::GetUnresolved(InUserFactory, InLocalUserId, MemberIds);
    return ResultMap[*GetUniqueNetId(InMemberId)].ToSharedRef();
}

FOnlinePartyMemberRedpointEOS::FOnlinePartyMemberRedpointEOS(
    TSharedRef<FEOSUserFactory> InUserFactory,
    TSharedRef<const FUniqueNetIdEOS> InLocalUserId,
    FRoomRef InRoom,
    FRoomMemberRef InRoomMember,
    FRoomUserId InMemberId)
    : Room(InRoom)
    , RoomMember(InRoomMember)
    , MemberId(InMemberId)
    , UserRedpointEOS(FOnlinePartyMemberRedpointEOS::GetOnlineUserEOSInstance(InUserFactory, InLocalUserId, InMemberId))
{
}

void FOnlinePartyMemberRedpointEOS::RegisterEvents()
{
}

FOnlinePartyMemberRedpointEOS::~FOnlinePartyMemberRedpointEOS()
{
}

FUniqueNetIdRef FOnlinePartyMemberRedpointEOS::GetUserId() const
{
    return this->UserRedpointEOS->GetUserId();
}

FString FOnlinePartyMemberRedpointEOS::GetRealName() const
{
    return this->UserRedpointEOS->GetRealName();
}

FString FOnlinePartyMemberRedpointEOS::GetDisplayName(const FString &Platform) const
{
    return this->UserRedpointEOS->GetDisplayName(Platform);
}

bool FOnlinePartyMemberRedpointEOS::GetUserAttribute(const FString &AttrName, FString &OutAttrValue) const
{
    return this->UserRedpointEOS->GetUserAttribute(AttrName, OutAttrValue);
}

bool FOnlinePartyMemberRedpointEOS::SetUserLocalAttribute(const FString &AttrName, const FString &InAttrValue)
{
    return this->UserRedpointEOS->SetUserLocalAttribute(AttrName, InAttrValue);
}

FOnlinePartyDataConstPtr FOnlinePartyMemberRedpointEOS::GetPartyMemberData(const FName &InNamespace) const
{
    auto Data = MakeShared<FOnlinePartyData>();
    for (const auto &AttributeKV : this->RoomMember->GetAttributes())
    {
        if (AttributeKV.Key == TEXT("RedpointOSSv1EventForcingAttribute"))
        {
            continue;
        }

        FVariantData AttributeData;
        switch (AttributeKV.Value.Type())
        {
        case FRoomAttribute::EType::String:
            AttributeData = FVariantData(AttributeKV.Value.GetString());
            break;
        case FRoomAttribute::EType::Int64:
            AttributeData = FVariantData(AttributeKV.Value.GetInt64());
            break;
        case FRoomAttribute::EType::Double:
            AttributeData = FVariantData(AttributeKV.Value.GetDouble());
            break;
        case FRoomAttribute::EType::Bool:
            AttributeData = FVariantData(AttributeKV.Value.GetBool());
            break;
        default:
            checkf(false, TEXT("Unhandled FRoomAttribute::EType value!"));
            break;
        }
        Data->SetAttribute(AttributeKV.Key, AttributeData);
    }
    Data->ClearDirty();
    return Data;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()