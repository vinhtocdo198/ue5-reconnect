// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPresence/PresenceUserState.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1841536699, Redpoint::EOS::Presence)
{

FPresenceUserState::FPresenceUserState(const FIdentityUserId &InUserId)
    : UserId(InUserId)
    // @note: This is set to false initially because the presence room is never advertised upon creation.
    , bPresenceAdvertised(false)
    , bIsPlayingThisGame(false)
    , AdvertisedSession()
    , AdvertisedParty()
    , ActivityState(EPresenceUserActivityState::Online)
    , StatusTextFormat()
    , StatusTextFormatArguments()
    , CustomAttributes()
{
}

FPresenceUserState::FPresenceUserState(const FPresenceUserState &InUserState)
    : UserId(InUserState.UserId)
    , bPresenceAdvertised(InUserState.bPresenceAdvertised)
    , bIsPlayingThisGame(InUserState.bIsPlayingThisGame)
    , AdvertisedSession(InUserState.AdvertisedSession)
    , AdvertisedParty(InUserState.AdvertisedParty)
    , ActivityState(InUserState.ActivityState)
    , StatusTextFormat(InUserState.StatusTextFormat)
    , StatusTextFormatArguments(InUserState.StatusTextFormatArguments)
    , CustomAttributes(InUserState.CustomAttributes)
{
}

FPresenceUserState &FPresenceUserState::operator=(const FPresenceUserState &InUserState)
{
    checkf(this->UserId == InUserState.UserId, TEXT("Can not assign presence state to different user."));
    this->bPresenceAdvertised = InUserState.bPresenceAdvertised;
    this->bIsPlayingThisGame = InUserState.bIsPlayingThisGame;
    this->AdvertisedSession = InUserState.AdvertisedSession;
    this->AdvertisedParty = InUserState.AdvertisedParty;
    this->ActivityState = InUserState.ActivityState;
    this->StatusTextFormat = InUserState.StatusTextFormat;
    this->StatusTextFormatArguments = InUserState.StatusTextFormatArguments;
    this->CustomAttributes = InUserState.CustomAttributes;
    return *this;
}

FText FPresenceUserState::GetStatusTextFormatted() const
{
    return FText::Format(this->StatusTextFormat, this->StatusTextFormatArguments);
}

uint32 GetTypeHash(const FPresenceUserState &P)
{
    auto StatusTextFormatTextId = FTextInspector::GetTextId(P.StatusTextFormat.GetSourceText());

    uint32 H = GetTypeHashHelper(P.bPresenceAdvertised);
    H = HashCombineFast(H, GetTypeHashHelper(P.AdvertisedSession));
    H = HashCombineFast(H, GetTypeHashHelper(P.AdvertisedParty));
    H = HashCombineFast(H, GetTypeHashHelper(P.ActivityState));
    H = HashCombineFast(H, GetTypeHashHelper(StatusTextFormatTextId));
    H = HashCombineFast(H, GetTypeHashHelper(P.StatusTextFormatArguments.Num()));
    for (const auto &KV : P.StatusTextFormatArguments)
    {
        H = HashCombineFast(H, GetTypeHashHelper(KV.Key));
        H = HashCombineFast(H, GetTypeHashHelper(KV.Value.GetType()));
        switch (KV.Value.GetType())
        {
        case EFormatArgumentType::Int:
            H = HashCombineFast(H, GetTypeHashHelper(KV.Value.GetIntValue()));
            break;
        case EFormatArgumentType::UInt:
            H = HashCombineFast(H, GetTypeHashHelper(KV.Value.GetUIntValue()));
            break;
        case EFormatArgumentType::Float:
            H = HashCombineFast(H, GetTypeHashHelper(KV.Value.GetFloatValue()));
            break;
        case EFormatArgumentType::Double:
            H = HashCombineFast(H, GetTypeHashHelper(KV.Value.GetDoubleValue()));
            break;
        case EFormatArgumentType::Text: {
            auto StatusTextFormatArgumentTextId = FTextInspector::GetTextId(KV.Value.GetTextValue());
            H = HashCombineFast(H, GetTypeHashHelper(StatusTextFormatArgumentTextId));
            break;
        }
        case EFormatArgumentType::Gender:
            H = HashCombineFast(H, GetTypeHashHelper(KV.Value.GetGenderValue()));
            break;
        default:
            break;
        }
    }
    H = HashCombineFast(H, GetTypeHashHelper(P.CustomAttributes.Num()));
    for (const auto &KV : P.CustomAttributes)
    {
        H = HashCombineFast(H, GetTypeHashHelper(KV.Key));
        H = HashCombineFast(H, GetTypeHashHelper(KV.Value));
    }
    return H;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()