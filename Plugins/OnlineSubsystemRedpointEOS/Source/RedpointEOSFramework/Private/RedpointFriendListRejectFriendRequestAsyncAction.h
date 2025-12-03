// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"
#include "RedpointEOSAsync/Task.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointFriendListEntry.h"

#include "RedpointFriendListRejectFriendRequestAsyncAction.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UCLASS()
class REDPOINTEOSFRAMEWORK_API URedpointFriendListRejectFriendRequestAsyncAction : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FRedpointFriendListActionComplete OnComplete;

    UPROPERTY()
    TObjectPtr<URedpointFriendListEntry> Entry;

    /**
     * Rejects the friend request based on the friend list entry.
     *
     * This blueprint node is intended to be used with friend list entries returned by the "Redpoint EOS Online
     * Framework Local Player Subsystem". If you want to reject friend requests from arbitrary users or are building
     * your own friend list system without using the Redpoint EOS Online Framework helper functions, you should use the
     * blueprint nodes on "Online Friend Subsystem" instead.
     */
    UFUNCTION(
        BlueprintCallable,
        Category = "Redpoint EOS Online Framework|Friend Lists for UMG",
        meta = (DisplayName = "Reject Friend Request from Friend List Entry", BlueprintInternalUseOnly = "true"))
    static URedpointFriendListRejectFriendRequestAsyncAction *RejectFriendRequest(
        URedpointFriendListEntry *FriendListEntry);

    virtual void Activate() override;

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
private:
    ::Redpoint::EOS::Async::TTask<void> ActivateInternal();
#endif
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()