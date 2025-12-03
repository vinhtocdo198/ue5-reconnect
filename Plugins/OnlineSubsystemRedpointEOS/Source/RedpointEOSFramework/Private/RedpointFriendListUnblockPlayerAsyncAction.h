// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"
#include "RedpointEOSAsync/Task.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointFriendListEntry.h"

#include "RedpointFriendListUnblockPlayerAsyncAction.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UCLASS()
class REDPOINTEOSFRAMEWORK_API URedpointFriendListUnblockPlayerAsyncAction : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FRedpointFriendListActionComplete OnComplete;

    UPROPERTY()
    TObjectPtr<URedpointFriendListEntry> Entry;

    /**
     * Unblocks a player based on a friend list entry.
     *
     * This blueprint node is intended to be used with friend list entries returned by the "Redpoint EOS Online
     * Framework Local Player Subsystem". If you want to unblock arbitrary users or are building your own
     * friend list system without using the Redpoint EOS Online Framework helper functions, you should use the blueprint
     * nodes on "Online Friend Subsystem" instead.
     */
    UFUNCTION(
        BlueprintCallable,
        Category = "Redpoint EOS Online Framework|Friend Lists for UMG",
        meta = (DisplayName = "Unblock Friend List Entry", BlueprintInternalUseOnly = "true"))
    static URedpointFriendListUnblockPlayerAsyncAction *UnblockPlayer(URedpointFriendListEntry *FriendListEntry);

    virtual void Activate() override;

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
private:
    ::Redpoint::EOS::Async::TTask<void> ActivateInternal();
#endif
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()