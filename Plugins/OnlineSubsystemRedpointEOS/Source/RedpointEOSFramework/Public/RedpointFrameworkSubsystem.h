// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Engine/Texture.h"
#include "GameFramework/OnlineReplStructs.h"
#include "Online/CoreOnline.h"
#include "RedpointAvatarCacheEntry.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointFriendListSection.h"
#include "RedpointFriendListSectionType.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "RedpointFrameworkSubsystem.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UCLASS(
    Blueprintable,
    Category = "Redpoint EOS Online Framework",
    meta = (DisplayName = "Redpoint EOS Online Framework Subsystem"))
class REDPOINTEOSFRAMEWORK_API URedpointFrameworkSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    URedpointFrameworkSubsystem(const FObjectInitializer &ObjectInitializer);

    /**
     * If enabled, local users will always have a presence-enabled party. The game will automatically create a party for
     * them when they sign into EOS, or accept a party invite if there is one pending. If the user ever leaves their
     * party and isn't already in a party, a new party will automatically be created for them.
     */
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Redpoint EOS Online Framework")
    bool bEnsureAlwaysInParty = false;

    /**
     * If bEnsureAlwaysInParty is enabled, this specifies the size of the party that is automatically created; i.e. the
     * maximum number of members that can be in the party.
     */
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Redpoint EOS Online Framework")
    int32 PartyMaximumSize = 4;

    /**
     * If enabled, the automatically managed party will have voice chat enabled when it is created.
     */
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Redpoint EOS Online Framework")
    bool bEnableVoiceChatInParty = false;

private:
    /**
     * The avatar cache for players. This is used by the Redpoint Avatar UMG widget.
     */
    UPROPERTY()
    TMap<FUniqueNetIdRepl, FRedpointAvatarCacheEntry> AvatarCache;

public:
    /**
     * Starts fetching an avatar for the target user ID from the local user ID, returning None until the texture has
     * been fetched.
     */
    UFUNCTION()
    UTexture *FetchAvatar(
        FUniqueNetIdRepl LocalUserId,
        FUniqueNetIdRepl TargetUserId,
        FTimespan Expiration,
        bool &bOutHasFailed);

private:
    /**
     * Handles the avatar fetch callback.
     */
    void OnGetAvatarComplete(
        bool bWasSuccessful,
        TSoftObjectPtr<UTexture> ResultTexture,
        FUniqueNetIdRepl LocalUserId,
        FUniqueNetIdRepl TargetUserId,
        FTimespan Expiration);

public:
    /**
     * Return the current release version determined when this project was built with UET (the Unreal Engine Tool). If
     * you're not building and deploying your project with UET, this will always be an empty string.
     */
    UFUNCTION(BlueprintPure, Category = "Redpoint EOS Online Framework")
    FString GetReleaseVersion() const;
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()