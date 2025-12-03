// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointFriendListEntry.h"

#include "RedpointFriendListKickFromPartyAsyncAction.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UCLASS()
class REDPOINTEOSFRAMEWORK_API URedpointFriendListKickFromPartyAsyncAction : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FRedpointFriendListActionComplete OnComplete;

    UPROPERTY()
    TObjectPtr<URedpointFriendListEntry> Entry;

    /**
     * Kicks a player from the local player's primary party based on the friend list entry.
     *
     * This blueprint node is intended to be used with friend list entries returned by the "Redpoint EOS Online
     * Framework Local Player Subsystem". If you want to kick arbitrary party members or are building your own
     * friend list system without using the Redpoint EOS Online Framework helper functions, you should use the blueprint
     * nodes on "Online Party Subsystem" instead.
     */
    UFUNCTION(
        BlueprintCallable,
        Category = "Redpoint EOS Online Framework|Friend Lists for UMG",
        meta = (DisplayName = "Kick Friend List Entry from Party", BlueprintInternalUseOnly = "true"))
    static URedpointFriendListKickFromPartyAsyncAction *KickFromParty(URedpointFriendListEntry *FriendListEntry);

    virtual void Activate() override;
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()