// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "GameFramework/OnlineReplStructs.h"
#include "RedpointAvatarLoadingIndicator.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointAvatar.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UCLASS(BlueprintType, Category = "Redpoint EOS Online Framework")
class REDPOINTEOSFRAMEWORK_API URedpointAvatar : public UUserWidget
{
    GENERATED_BODY()

public:
    URedpointAvatar(const FObjectInitializer &ObjectInitializer);

    /**
     * If true, the target user will be the local user that owns this widget, instead of the TargetUserId property.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Redpoint EOS Online Framework")
    bool bTargetUserIsLocalUser;

    /**
     * The target user to display the avatar for. The local user that owns this widget will be used as the local user
     * for querying avatars.
     */
    UPROPERTY(
        BlueprintReadWrite,
        EditAnywhere,
        Category = "Redpoint EOS Online Framework",
        meta = (EditCondition = "!bTargetUserIsLocalUser"))
    FUniqueNetIdRepl TargetUserId;

    /**
     * An optional texture to display if the user doesn't have an avatar, the avatar fails to load, or if the avatar is
     * loading and the loading indicator is set to "DefaultTexture".
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Redpoint EOS Online Framework")
    TObjectPtr<UTexture> DefaultTexture;

    /**
     * The type of loading indicator to display while the avatar is loading. If the avatar fails to load, the default
     * texture is always rendered if one is set.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Redpoint EOS Online Framework")
    ERedpointAvatarLoadingIndicator LoadingIndicator;

    /**
     * The duration in minutes that avatars will be kept in the cache after they have last been used.
     */
    UPROPERTY(
        BlueprintReadWrite,
        EditAnywhere,
        Category = "Redpoint EOS Online Framework",
        meta = (ClampMin = "1", ClampMax = "1440"))
    double ExpirationMinutes;

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;

private:
    FSlateBrush CurrentImage;
    bool bCurrentIsLoading;
    bool bCurrentIsFailed;
    FUniqueNetIdRepl PreviousTargetUserId;
    const FSlateBrush *GetImage() const;
    int GetWidgetSwitcherIndex() const;

    virtual void NativeTick(const FGeometry &MyGeometry, float InDeltaTime) override;
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()
