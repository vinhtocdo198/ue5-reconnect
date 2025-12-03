
// Copyright June Rhodes. All Rights Reserved.

#include "RedpointOSSv1OnlinePartyRequestToJoinInfo.h"

URedpointOSSv1OnlinePartyRequestToJoinInfo *URedpointOSSv1OnlinePartyRequestToJoinInfo::FromNative(
    const IOnlinePartyRequestToJoinInfoConstPtr &InObj)
{
    auto Ref = NewObject<URedpointOSSv1OnlinePartyRequestToJoinInfo>();
    Ref->OnlinePartyRequestToJoinInfo = InObj;
    return Ref;
}

URedpointOSSv1OnlinePartyRequestToJoinInfo *URedpointOSSv1OnlinePartyRequestToJoinInfo::FromNative(
    const IOnlinePartyRequestToJoinInfo &InObj)
{
    auto Ref = NewObject<URedpointOSSv1OnlinePartyRequestToJoinInfo>();
    Ref->OnlinePartyRequestToJoinInfo = InObj.AsShared();
    return Ref;
}

FUniqueNetIdRepl URedpointOSSv1OnlinePartyRequestToJoinInfo::GetUserId() const
{
    if (!IsValid())
    {
        return FUniqueNetIdRepl();
    }

    return this->OnlinePartyRequestToJoinInfo->GetUserId();
}

FString URedpointOSSv1OnlinePartyRequestToJoinInfo::GetDisplayName() const
{
    if (!IsValid())
    {
        return TEXT("");
    }

    return this->OnlinePartyRequestToJoinInfo->GetDisplayName();
}

FString URedpointOSSv1OnlinePartyRequestToJoinInfo::GetPlatformData() const
{
    if (!IsValid())
    {
        return TEXT("");
    }

    return this->OnlinePartyRequestToJoinInfo->GetPlatformData();
}

FDateTime URedpointOSSv1OnlinePartyRequestToJoinInfo::GetExpirationTime() const
{
    if (!IsValid())
    {
        return FDateTime::MinValue();
    }

    return this->OnlinePartyRequestToJoinInfo->GetExpirationTime();
}

int64 URedpointOSSv1OnlinePartyRequestToJoinInfo::GetPartyTypeId() const
{
    if (!IsValid())
    {
        return 0;
    }

    return static_cast<int64>(this->OnlinePartyRequestToJoinInfo->GetPartyTypeId().GetValue());
}