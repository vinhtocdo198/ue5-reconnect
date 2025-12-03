// Copyright June Rhodes. All Rights Reserved.

#include "Types/OSBOnlineUserPresenceData.h"

FOnlineUserPresenceStatusData FOnlineUserPresenceStatusData::FromNative(const FOnlineUserPresenceStatus &InObj)
{
    FOnlineUserPresenceStatusData Status;
    Status.Status = InObj.StatusStr;
    Status.State = (FOnlineUserPresenceStatusState)InObj.State;
    for (auto Prop : InObj.Properties)
    {
        FString Val;
        Status.Properties.Add(Prop.Key, FVariantDataBP::FromNative(Prop.Value));
    }
    return Status;
}

FOnlineUserPresenceStatus FOnlineUserPresenceStatusData::ToNative() const
{
    FOnlineUserPresenceStatus Result;
    Result.StatusStr = this->Status;
    Result.State = (EOnlinePresenceState::Type)this->State;
    for (auto Prop : this->Properties)
    {
        Result.Properties.Add(Prop.Key, Prop.Value.ToNative());
    }
    return Result;
}

FOnlineUserPresenceData FOnlineUserPresenceData::FromNative(const TSharedPtr<FOnlineUserPresence> &InObj)
{
    FOnlineUserPresenceData Data;
    if (InObj == nullptr)
    {
        return Data;
    }

    Data.SessionId = FUniqueNetIdRepl(InObj->SessionId);
    Data.IsOnline = InObj->bIsOnline;
    Data.IsPlaying = InObj->bIsPlaying;
    Data.IsPlayingThisGame = InObj->bIsPlayingThisGame;
    Data.IsJoinable = InObj->bIsJoinable;
    Data.HasVoiceSupport = InObj->bHasVoiceSupport;
    Data.LastOnline = InObj->LastOnline;
    Data.Status = FOnlineUserPresenceStatusData::FromNative(InObj->Status);
    return Data;
}

FOnlineUserPresenceData FOnlineUserPresenceData::FromNative(const FOnlineUserPresence &InObj)
{
    FOnlineUserPresenceData Data;
    Data.SessionId = FUniqueNetIdRepl(InObj.SessionId);
    Data.IsOnline = InObj.bIsOnline;
    Data.IsPlaying = InObj.bIsPlaying;
    Data.IsPlayingThisGame = InObj.bIsPlayingThisGame;
    Data.IsJoinable = InObj.bIsJoinable;
    Data.HasVoiceSupport = InObj.bHasVoiceSupport;
    Data.LastOnline = InObj.LastOnline;
    Data.Status = FOnlineUserPresenceStatusData::FromNative(InObj.Status);
    return Data;
}

FOnlineUserPresence FOnlineUserPresenceData::ToNative() const
{
    FOnlineUserPresence Presence;
    Presence.SessionId = this->SessionId.GetUniqueNetId();
    Presence.bIsOnline = this->IsOnline;
    Presence.bIsPlaying = this->IsPlaying;
    Presence.bIsPlayingThisGame = this->IsPlayingThisGame;
    Presence.bIsJoinable = this->IsJoinable;
    Presence.bHasVoiceSupport = this->HasVoiceSupport;
    Presence.LastOnline = this->LastOnline;
    Presence.Status = this->Status.ToNative();
    return Presence;
}