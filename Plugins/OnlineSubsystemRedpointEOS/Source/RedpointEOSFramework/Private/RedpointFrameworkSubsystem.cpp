// Copyright June Rhodes. All Rights Reserved.

#include "RedpointFrameworkSubsystem.h"

#include "Misc/ConfigCacheIni.h"
#include "OnlineSubsystemUtils.h"
#include "RedpointEOSConfig/IniConfig.h"
#include "RedpointEOSInterfaces/Interfaces/OnlineAvatarInterface.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

URedpointFrameworkSubsystem::URedpointFrameworkSubsystem(const FObjectInitializer &ObjectInitializer)
{
    using namespace ::Redpoint::EOS::Config;

    // Always use the INI config. Technically configuration can be instantiated in memory for automation tests, but
    // automation tests rarely have a world or game instance anyway, so this subsystem should not be initialized for
    // them.
    auto Config = MakeShared<FIniConfig>();
    this->bEnsureAlwaysInParty = Config->GetFrameworkDefaultEnsureAlwaysInParty();
    this->PartyMaximumSize = Config->GetFrameworkDefaultPartyMaximumSize();
    this->bEnableVoiceChatInParty = Config->GetFrameworkDefaultEnableVoiceChatInParty();
}

UTexture *URedpointFrameworkSubsystem::FetchAvatar(
    FUniqueNetIdRepl LocalUserId,
    FUniqueNetIdRepl TargetUserId,
    FTimespan Expiration,
    bool &bOutHasFailed)
{
    bOutHasFailed = false;

    // Return the existing value, or None if the avatar is being fetched or failed to fetch.
    FRedpointAvatarCacheEntry *ExistingEntry = this->AvatarCache.Find(TargetUserId);
    if (ExistingEntry != nullptr)
    {
        if (IsValid(ExistingEntry->Avatar))
        {
            auto NewExpiration = FDateTime::UtcNow() + Expiration;
            if (ExistingEntry->ExpiresAt < NewExpiration)
            {
                ExistingEntry->ExpiresAt = NewExpiration;
            }
            return ExistingEntry->Avatar;
        }

        bOutHasFailed = ExistingEntry->bFailed;
        return nullptr;
    }

    // Get the online subsystem and avatar interface.
    IOnlineSubsystem *OSS = Online::GetSubsystem(this->GetWorld());
    if (OSS == nullptr)
    {
        return nullptr;
    }
    auto Avatar = Online::GetAvatarInterface(OSS);
    if (Avatar == nullptr)
    {
        return nullptr;
    }
    if (!LocalUserId.IsValid() || !TargetUserId.IsValid())
    {
        return nullptr;
    }

    // Start fetching.
    FRedpointAvatarCacheEntry Entry;
    Entry.Avatar = nullptr;
    Entry.bFailed = false;
    Entry.bFetchInProgress = true;
    Entry.ExpiresAt = FDateTime::UtcNow() + Expiration;
    this->AvatarCache.Add(TargetUserId, Entry);
    if (!Avatar->GetAvatar(
            *LocalUserId,
            *TargetUserId,
            nullptr,
            FOnGetAvatarComplete::CreateUObject(
                this,
                &URedpointFrameworkSubsystem::OnGetAvatarComplete,
                LocalUserId,
                TargetUserId,
                Expiration)))
    {
        Entry.bFailed = true;
        Entry.bFetchInProgress = false;
        this->AvatarCache.Add(TargetUserId, Entry);
    }
    return nullptr;
}

void URedpointFrameworkSubsystem::OnGetAvatarComplete(
    bool bWasSuccessful,
    TSoftObjectPtr<UTexture> ResultTexture,
    FUniqueNetIdRepl LocalUserId,
    FUniqueNetIdRepl TargetUserId,
    FTimespan Expiration)
{
    // If the entry no longer exists, ignore this result callback.
    FRedpointAvatarCacheEntry *ExistingEntry = this->AvatarCache.Find(TargetUserId);
    if (ExistingEntry == nullptr)
    {
        return;
    }

    // Update the cache and we're done. The next call to FetchAvatar will start returning the texture.
    ExistingEntry->Avatar = ResultTexture.Get();
    ExistingEntry->bFailed = !bWasSuccessful || ResultTexture.IsNull();
    ExistingEntry->bFetchInProgress = false;
    ExistingEntry->ExpiresAt = FDateTime::UtcNow() + Expiration;
}

FString URedpointFrameworkSubsystem::GetReleaseVersion() const
{
    FString ReleaseVersion = TEXT("");
    GConfig->GetString(TEXT("Redpoint.UnrealEngineTool"), TEXT("ReleaseVersion"), ReleaseVersion, GEngineIni);
    return ReleaseVersion;
}

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()