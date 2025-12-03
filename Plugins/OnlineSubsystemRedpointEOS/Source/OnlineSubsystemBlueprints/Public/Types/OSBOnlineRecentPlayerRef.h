// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Online/CoreOnline.h"
#include "Types/OSBOnlineUserRef.h"

#include "OSBOnlineRecentPlayerRef.generated.h"

UCLASS(BlueprintType, Transient, Meta = (DontUseGenericSpawnObject, IsBlueprintBase = "false"))
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineRecentPlayerRef : public UOnlineUserRef
{
    GENERATED_BODY()

private:
    TSharedPtr<FOnlineRecentPlayer> RecentPlayer;

public:
    static UOnlineRecentPlayerRef *FromRecentPlayer(const TSharedPtr<FOnlineRecentPlayer> &RecentPlayer);
    TSharedPtr<FOnlineRecentPlayer> GetRecentPlayer() const
    {
        return this->RecentPlayer;
    }

    /**
     * Returns the last time this user was seen.
     */
    UFUNCTION(BlueprintPure, Category = "Online")
    FDateTime GetLastSeen();
};