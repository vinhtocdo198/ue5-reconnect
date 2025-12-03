
// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "GameFramework/OnlineReplStructs.h"
#include "Interfaces/OnlinePartyInterface.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointOSSv1OnlinePartyRequestToJoinInfo.generated.h"

/**
 * Represents a request by another player to join a party that the local player is the owner of.
 */
UCLASS(
    BlueprintType,
    Transient,
    Meta = (DontUseGenericSpawnObject, IsBlueprintBase = "false", DisplayName = "Online Party Request To Join Info"))
class ONLINESUBSYSTEMBLUEPRINTS_API URedpointOSSv1OnlinePartyRequestToJoinInfo : public UObject
{
    GENERATED_BODY()

private:
    IOnlinePartyRequestToJoinInfoConstPtr OnlinePartyRequestToJoinInfo;

public:
    static URedpointOSSv1OnlinePartyRequestToJoinInfo *FromNative(const IOnlinePartyRequestToJoinInfoConstPtr &InObj);
    static URedpointOSSv1OnlinePartyRequestToJoinInfo *FromNative(const IOnlinePartyRequestToJoinInfo &InObj);
    IOnlinePartyRequestToJoinInfoConstPtr ToNative() const
    {
        return this->OnlinePartyRequestToJoinInfo.ToSharedRef();
    }
    bool IsValid() const
    {
        return this->OnlinePartyRequestToJoinInfo.IsValid();
    }

    /**
     * Get the ID of the user requesting to join.
     */
    UFUNCTION(BlueprintPure, Category = "Online")
    FUniqueNetIdRepl GetUserId() const;

    /**
     * Get the display name of the user requesting to join.
     */
    UFUNCTION(BlueprintPure, Category = "Online")
    FString GetDisplayName() const;

    /**
     * Get the platform data associated with this request.
     */
    UFUNCTION(BlueprintPure, Category = "Online")
    FString GetPlatformData() const;

    /**
     * Get the expiration time of the request to join.
     */
    UFUNCTION(BlueprintPure, Category = "Online")
    FDateTime GetExpirationTime() const;

    /**
     * Get the party type ID of the request to join.
     */
    UFUNCTION(BlueprintPure, Category = "Online")
    int64 GetPartyTypeId() const;
};