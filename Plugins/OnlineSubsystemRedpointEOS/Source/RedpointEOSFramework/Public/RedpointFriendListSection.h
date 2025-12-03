// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointFriendListEntry.h"
#include "RedpointFriendListSectionType.h"

#include "RedpointFriendListSection.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UCLASS(BlueprintType, Category = "Redpoint EOS Online Framework|Friend Lists for UMG")
class REDPOINTEOSFRAMEWORK_API URedpointFriendListSection : public UObject
{
    GENERATED_BODY()

public:
    /**
     * The type of entries that are present in this section.
     */
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Redpoint EOS Online Framework|Friend Lists for UMG")
    ERedpointFriendListSectionType Type;

    /**
     * The default title used for this section. You don't need to use this value; you can present the title in a
     * different way in your UMG widget using the Type property instead.
     */
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Redpoint EOS Online Framework|Friend Lists for UMG")
    FText Title;

    /**
     * Section objects are persisted even when the locally signed in user changes, so that list views don't need to call
     * "Set Items" again on login events. This variable tracks whether an initial sync needs to happen upon calling "Get
     * Friend List Children" after a login state change.
     */
    UPROPERTY()
    bool bRequiresInit = true;

    /**
     * The entries in this section. This property is not visible to blueprints; the "GetFriendListChildren" blueprint
     * node on the framework subsystem instead.
     */
    UPROPERTY()
    TArray<TObjectPtr<URedpointFriendListEntry>> Entries;

    /**
     * Returns whether this section has any entries.
     */
    UFUNCTION(BlueprintPure, Category = "Redpoint EOS Online Framework|Friend Lists for UMG")
    bool HasAnyEntries() const;
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()