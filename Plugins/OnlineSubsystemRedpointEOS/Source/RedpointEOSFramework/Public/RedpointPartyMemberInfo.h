// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "GameFramework/OnlineReplStructs.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointPartyAttributeValue.h"

#include "RedpointPartyMemberInfo.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

/**
 * Represents information about a party member in a party slot.
 */
USTRUCT(BlueprintType, meta = (Category = "Redpoint EOS Online Framework", DisplayName = "Redpoint Party Member Info"))
struct REDPOINTEOSFRAMEWORK_API FRedpointPartyMemberInfo
{
    GENERATED_BODY()

public:
    /**
     * If set, this party member slot has a player in it.
     *
     * If this is false, all of the other values in this structure will be empty/default values.
     */
    UPROPERTY(BlueprintReadOnly, Category = "Redpoint EOS Online Framework")
    bool bHasPartyMember = false;

    /**
     * The user ID of the party member in this slot.
     */
    UPROPERTY(BlueprintReadOnly, Category = "Redpoint EOS Online Framework")
    FUniqueNetIdRepl UserId;

    /**
     * The player name of the party member in this slot.
     */
    UPROPERTY(BlueprintReadOnly, Category = "Redpoint EOS Online Framework")
    FString PlayerName;

    /**
     * All of the user attributes available for the player in this slot. These attributes represent global information
     * about the user; not the party-specific attributes.
     */
    UPROPERTY(BlueprintReadOnly, Category = "Redpoint EOS Online Framework")
    TMap<FString, FString> PlayerUserAttributes;

    /**
     * The party member attributes for this party member. This is the attribute map that will contain any game-specific
     * or party-specific information about this party member, such as skins, styling, player settings, etc.
     */
    UPROPERTY(BlueprintReadOnly, Category = "Redpoint EOS Online Framework")
    TMap<FString, FRedpointPartyAttributeValue> PartyMemberAttributes;

    /**
     * If true, this player is the leader of the party.
     */
    UPROPERTY(BlueprintReadOnly, Category = "Redpoint EOS Online Framework")
    bool bIsPartyLeader = false;
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()