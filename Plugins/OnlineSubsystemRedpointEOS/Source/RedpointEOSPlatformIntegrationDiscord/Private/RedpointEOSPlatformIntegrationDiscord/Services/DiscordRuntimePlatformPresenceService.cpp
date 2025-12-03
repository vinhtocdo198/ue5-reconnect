// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationDiscord/Services/DiscordRuntimePlatformPresenceService.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_DISCORD_ENABLED
#include "Misc/ConfigCacheIni.h"
#include "RedpointEOSConfig/Config.h"
#include "RedpointEOSIdentity/IdentitySystem.h"
#include "RedpointEOSPlatformIntegrationDiscord/Logging.h"
#include "RedpointEOSPresence/PresenceUserState.h"
#include "discordpp.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_DISCORD_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(207761676, Redpoint::EOS::Platform::Integration::Discord)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::Platform::Integration::Discord;

FDiscordRuntimePlatformPresenceService::FDiscordRuntimePlatformPresenceService(
    const TSharedRef<discordpp::Client> &InDiscordClient)
    : DiscordClient(InDiscordClient)
    , UserStorage()
{
}

TOptional<bool> FDiscordRuntimePlatformPresenceService::IsLocalUserOnline(
    const FRuntimePlatformServiceCallContextRef &CallContext) const
{
    // Discord can't indicate online status, because it won't ever be the authentication provider.
    return TOptional<bool>();
}

void FDiscordRuntimePlatformPresenceService::UpdateLocalUserPresence(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FPresenceUserState &InDesiredPresenceState,
    const FOnLocalUserPresenceUpdated &InCompletionDelegate)
{
    auto Clamp = [](const FString &Input) -> FString {
        auto ClampedInput = Input;
        if (ClampedInput.Len() < 2)
        {
            ClampedInput = TEXT("");
        }
        else if (ClampedInput.Len() > 128)
        {
            ClampedInput = ClampedInput.Mid(0, 128);
        }
        return ClampedInput;
    };
    auto AttrToString = [&InDesiredPresenceState](const FString &Key) -> FString {
        auto *Attr = InDesiredPresenceState.CustomAttributes.Find(Key);
        if (Attr != nullptr && Attr->Type() == FRoomAttribute::EType::String)
        {
            return Attr->GetString();
        }
        return TEXT("");
    };
    auto AttrToInt64 = [&InDesiredPresenceState](const FString &Key) -> TOptional<int64> {
        auto *Attr = InDesiredPresenceState.CustomAttributes.Find(Key);
        if (Attr != nullptr && Attr->Type() == FRoomAttribute::EType::Int64)
        {
            return Attr->GetInt64();
        }
        return TOptional<int64>();
    };
    auto StdStr = [](const FString &Value) -> std::string {
        return std::string(StringCast<ANSICHAR>(*Value).Get());
    };

    auto DetailsStr = StdStr(Clamp(InDesiredPresenceState.GetStatusTextFormatted().ToString()));
    auto StateStr = StdStr(Clamp(AttrToString(TEXT("Discord:State"))));
    auto Type = AttrToString(TEXT("Discord:Type"));

    auto &Activity = this->UserStorage[CallContext].Activity;
    if (Type == TEXT("Playing"))
    {
        Activity.SetType(discordpp::ActivityTypes::Playing);
    }
    else if (Type == TEXT("Streaming"))
    {
        Activity.SetType(discordpp::ActivityTypes::Streaming);
    }
    else if (Type == TEXT("Listening"))
    {
        Activity.SetType(discordpp::ActivityTypes::Listening);
    }
    else if (Type == TEXT("Watching"))
    {
        Activity.SetType(discordpp::ActivityTypes::Watching);
    }
    else if (Type == TEXT("CustomStatus"))
    {
        Activity.SetType(discordpp::ActivityTypes::CustomStatus);
    }
    else if (Type == TEXT("Competing"))
    {
        Activity.SetType(discordpp::ActivityTypes::Competing);
    }
    else if (Type == TEXT("HangStatus"))
    {
        Activity.SetType(discordpp::ActivityTypes::HangStatus);
    }
    else
    {
        Activity.SetType(discordpp::ActivityTypes::Playing);
    }
    if (StateStr.length() >= 2 && StateStr.length() <= 128)
    {
        Activity.SetState(StateStr);
    }
    if (DetailsStr.length() >= 2 && DetailsStr.length() <= 128)
    {
        Activity.SetDetails(DetailsStr);
    }
    {
        auto AssetLargeImage = AttrToString(TEXT("Discord:Assets:LargeImage"));
        auto AssetLargeText = AttrToString(TEXT("Discord:Assets:LargeText"));
        auto AssetSmallImage = AttrToString(TEXT("Discord:Assets:SmallImage"));
        auto AssetSmallText = AttrToString(TEXT("Discord:Assets:SmallText"));
        if (AssetLargeImage.IsEmpty() && AssetLargeText.IsEmpty() && AssetSmallImage.IsEmpty() &&
            AssetSmallText.IsEmpty())
        {
            Activity.SetAssets(std::optional<discordpp::ActivityAssets>());
        }
        else
        {
            discordpp::ActivityAssets ActivityAssets;
            ActivityAssets.SetLargeImage(StdStr(AssetLargeImage));
            ActivityAssets.SetLargeImage(StdStr(AssetLargeText));
            ActivityAssets.SetLargeImage(StdStr(AssetSmallImage));
            ActivityAssets.SetLargeImage(StdStr(AssetSmallText));
            Activity.SetAssets(ActivityAssets);
        }
    }
    {
        auto TimeStart = AttrToInt64(TEXT("Discord:Time:Start"));
        auto TimeEnd = AttrToInt64(TEXT("Discord:Time:End"));
        if (TimeStart.IsSet() || TimeEnd.IsSet())
        {
            discordpp::ActivityTimestamps Timestamps;
            if (TimeStart.IsSet())
            {
                Timestamps.SetStart(static_cast<uint64_t>(*TimeStart));
            }
            if (TimeEnd.IsSet())
            {
                Timestamps.SetEnd(static_cast<uint64_t>(*TimeEnd));
            }
            Activity.SetTimestamps(Timestamps);
        }
        else
        {
            Activity.SetTimestamps(std::optional<discordpp::ActivityTimestamps>());
        }
    }
    // We don't do SetParty here - that happens in UpsertRoomAdvertisement/DeleteRoomAdvertisement.
    // We don't do SetSecrets here - that happens in UpsertRoomAdvertisement/DeleteRoomAdvertisement.
    // @note: SetSupportedPlatforms should come from cross-play restrictions on the party.

    this->DiscordClient->UpdateRichPresence(Activity, [InCompletionDelegate](discordpp::ClientResult DiscordResult) {
        InCompletionDelegate.ExecuteIfBound(
            DiscordResult.Successful() ? Errors::Success()
                                       : Errors::UnexpectedError(
                                             ANSI_TO_TCHAR(__FUNCTION__),
                                             UTF8_TO_TCHAR(DiscordResult.ToString().c_str())));
    });
}

void FDiscordRuntimePlatformPresenceService::UpsertRoomAdvertisement(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FPresenceUserAdvertisedRoom &InAdvertisedRoom,
    bool bIsParty,
    const FOnUpsertRoomAdvertisementComplete &InCompletionDelegate)
{
    auto PresenceAdvertisementType =
        CallContext->PlatformHandle->GetEnvironment()->Config->GetPresenceAdvertisementType();
    bool bHandle = bIsParty ? (PresenceAdvertisementType == EPresenceAdvertisementType::Party)
                            : (PresenceAdvertisementType == EPresenceAdvertisementType::Session);
    if (!bHandle)
    {
        InCompletionDelegate.ExecuteIfBound(Errors::Success());
        return;
    }

    if (CallContext->LocalUserNum != 0)
    {
        InCompletionDelegate.ExecuteIfBound(Errors::Success());
        return;
    }

    auto StdStr = [](const FString &Value) -> std::string {
        return std::string(StringCast<ANSICHAR>(*Value).Get());
    };

    auto &Activity = this->UserStorage[CallContext].Activity;
    {
        discordpp::ActivityParty Party;
        Party.SetId(StdStr(InAdvertisedRoom.RoomId->ToString()));
        Party.SetCurrentSize(static_cast<int32_t>(InAdvertisedRoom.CurrentMemberCount));
        Party.SetMaxSize(static_cast<int32_t>(InAdvertisedRoom.MaximumMemberCount));
        Party.SetPrivacy(
            InAdvertisedRoom.bJoinable ? discordpp::ActivityPartyPrivacy::Public
                                       : discordpp::ActivityPartyPrivacy::Private);
        Activity.SetParty(Party);
    }
    {
        discordpp::ActivitySecrets Secrets;
        Secrets.SetJoin(StdStr(FString::Printf(
            TEXT("%s|%s"),
            *InAdvertisedRoom.RoomId->ToString(),
            *InAdvertisedRoom.RoomNamespace.ToString())));
        Activity.SetSecrets(Secrets);
    }

    this->DiscordClient->UpdateRichPresence(Activity, [InCompletionDelegate](discordpp::ClientResult DiscordResult) {
        InCompletionDelegate.ExecuteIfBound(
            DiscordResult.Successful() ? Errors::Success()
                                       : Errors::UnexpectedError(
                                             ANSI_TO_TCHAR(__FUNCTION__),
                                             UTF8_TO_TCHAR(DiscordResult.ToString().c_str())));
    });
}

void FDiscordRuntimePlatformPresenceService::DeleteRoomAdvertisement(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FRoomIdRef &InRoomId,
    bool bIsParty,
    const FOnDeleteRoomAdvertisementComplete &InCompletionDelegate)
{
    auto PresenceAdvertisementType =
        CallContext->PlatformHandle->GetEnvironment()->Config->GetPresenceAdvertisementType();
    bool bHandle = bIsParty ? (PresenceAdvertisementType == EPresenceAdvertisementType::Party)
                            : (PresenceAdvertisementType == EPresenceAdvertisementType::Session);
    if (!bHandle)
    {
        InCompletionDelegate.ExecuteIfBound(Errors::Success());
        return;
    }

    if (CallContext->LocalUserNum != 0)
    {
        InCompletionDelegate.ExecuteIfBound(Errors::Success());
        return;
    }

    // We don't use ClearRichPresence, because we only want to clear the current party/session information.

    auto &Activity = this->UserStorage[CallContext].Activity;
    Activity.SetParty(std::optional<discordpp::ActivityParty>());
    Activity.SetSecrets(std::optional<discordpp::ActivitySecrets>());

    this->DiscordClient->UpdateRichPresence(Activity, [InCompletionDelegate](discordpp::ClientResult DiscordResult) {
        InCompletionDelegate.ExecuteIfBound(
            DiscordResult.Successful() ? Errors::Success()
                                       : Errors::UnexpectedError(
                                             ANSI_TO_TCHAR(__FUNCTION__),
                                             UTF8_TO_TCHAR(DiscordResult.ToString().c_str())));
    });
}

IRuntimePlatformPresenceService::FOnRoomAdvertisementInviteReceived &FDiscordRuntimePlatformPresenceService::
    OnRoomAdvertisementInviteReceived(const FRuntimePlatformServiceCallContextRef &CallContext)
{
    return this->UserStorage[CallContext].OnRoomAdvertisementInviteReceived;
}

IRuntimePlatformPresenceService::FOnRoomAdvertisementInviteAccepted &FDiscordRuntimePlatformPresenceService::
    OnRoomAdvertisementInviteAccepted(const FRuntimePlatformServiceCallContextRef &CallContext)
{
    return this->UserStorage[CallContext].OnRoomAdvertisementInviteAccepted;
}

TOptional<EPresenceUserActivityState> FDiscordRuntimePlatformPresenceService::GetNonUnifiedFriendPresence(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const EOS_EExternalAccountType &FriendAccountType,
    const FString &FriendAccountId)
{
    // @note: It's not possible to get Discord friends without doing full authentication.
    return TOptional<EPresenceUserActivityState>();
}

IRuntimePlatformPresenceService::FOnNonUnifiedFriendPresenceChanged &FDiscordRuntimePlatformPresenceService::
    OnNonUnifiedFriendPresenceChanged(const FRuntimePlatformServiceCallContextRef &CallContext)
{
    return this->UserStorage[CallContext].OnNonUnifiedFriendPresenceChanged;
}

void FDiscordRuntimePlatformPresenceService::SendRoomInviteToNonUnifiedFriend(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const EOS_EExternalAccountType &FriendAccountType,
    const FString &FriendAccountId,
    const FRoomIdRef &RoomId,
    const FOnSendRoomInviteToNonUnifiedFriendComplete &InCompletionDelegate)
{
    // @note: This could technically be implemented by calling SendActivityInvite, but we'd need to have Discord
    // friends, which we can't get without full authentication.
    InCompletionDelegate.ExecuteIfBound(Errors::NotImplemented());
}

bool FDiscordRuntimePlatformPresenceService::TryOpenRoomInviteRecipientSelection(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FRoomIdRef &RoomId)
{
    // @note: Discord does not have a UI for selecting friends to invite.
    return false;
}

void FDiscordRuntimePlatformPresenceService::OnLocalUserSignInAfterSystemStart(
    const FRuntimePlatformServiceCallContextRef &CallContext)
{
    this->DiscordClient->SetActivityJoinCallback([Presence = this->AsShared(), CallContext](std::string JoinSecret) {
        auto IdentitySystem = CallContext->PlatformHandle->GetSystem<IIdentitySystem>();
        auto IdentityUser = IdentitySystem->GetUser(0);
        if (!IdentityUser.IsValid())
        {
            // @note: This can only happen if the user is signed out, since we don't register the callback until
            // the first login.
            return;
        }

        TArray<FString> Components;
        FString(JoinSecret.c_str()).ParseIntoArray(Components, TEXT("|"));
        if (Components.Num() == 2)
        {
            auto RoomId = FRoomId::FromString(Components[0]);
            auto RoomNamespace = FName(*Components[1]);

            if (RoomId.IsValid())
            {
                UE_LOG(
                    LogRedpointEOSPlatformIntegrationDiscord,
                    Verbose,
                    TEXT("Accepted EOS invite via Discord for room ID: %s"),
                    *RoomId->ToString());
                Presence->UserStorage[CallContext].OnRoomAdvertisementInviteAccepted.Broadcast(
                    IdentityUser->GetUserId(),
                    nullptr,
                    RoomId.ToSharedRef(),
                    RoomNamespace);
            }
        }
    });
}

void FDiscordRuntimePlatformPresenceService::OnLocalUserSignOutBeforeSystemStop(
    const FRuntimePlatformServiceCallContextRef &CallContext)
{
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()