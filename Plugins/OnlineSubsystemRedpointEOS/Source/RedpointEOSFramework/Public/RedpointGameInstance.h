// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "Engine/GameInstance.h"
#include "Engine/Texture2D.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSIdentity/IdentityUser.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointStartupScreen.h"

#include "RedpointGameInstance.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UCLASS(Blueprintable, Category = "Redpoint EOS Online Framework")
class REDPOINTEOSFRAMEWORK_API URedpointGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    URedpointGameInstance(const FObjectInitializer &ObjectInitializer);

    /**
     * If enabled, local users will automatically be signed in when the game starts (even before your startup map
     * loads).
     *
     * If the user is ever signed out for any reason, AutoLogin will automatically be called again to ensure the
     * user is signed in again. In addition, the game will be paused while the sign-in process takes place; the user
     * won't be able to return to the game until they've successfully signed in again.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Startup")
    bool bEnsureAlwaysSignedIn = true;

    /**
     * The startup screen that should be displayed in standalone games while the local user is automatically being
     * signed in. If this is not set, a default startup screen will be displayed.
     *
     * This should be a UMG widget that implements the RedpointStartupScreen interface.
     */
    UPROPERTY(
        BlueprintReadOnly,
        EditDefaultsOnly,
        Category = "Startup",
        meta = (MetaClass = "/Script/UMG.Widget", MustImplement = "/Script/RedpointEOSFramework.RedpointStartupScreen"))
    FSoftClassPath StartupScreen;

    /**
     * The game logo that will be displayed on the default startup screen. You can use this option if you don't want to
     * replace the whole startup screen with your own version.
     */
    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Startup")
    TObjectPtr<UTexture> StartupScreenLogo;

    /**
     * The minimum size and desired ratio of the game logo to display on the startup screen.
     */
    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Startup")
    FVector2D StartupScreenLogoSize;

    /**
     * The implementation of UOnlineSession to use. This should be left as the default value unless you know what you're
     * doing.
     */
    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Online", NoClear)
    TSubclassOf<UOnlineSession> OnlineSessionClass;

private:
    /**
     * The user widget currently being shown on screen during startup.
     */
    UPROPERTY()
    TObjectPtr<UUserWidget> StartupScreenWidget;

    FDelegateHandle OnBeginFrameDelegateHandle;

public:
    virtual void StartGameInstance() override;

private:
    UFUNCTION()
    void OnStartLogin();

    void OnFinishLogin(::Redpoint::EOS::API::FError ErrorCode, ::Redpoint::EOS::Identity::FIdentityUserPtr NewUser);

    void StartGameInstanceOnNextFrame();

    FDelegateHandle OnPostWorldInitializationHandle;
    FDelegateHandle OnWorldBeginPlayHandle;

    void OnStartupMapWorldInitialized(UWorld *World, const UWorld::InitializationValues);

    void OnStartupMapWorldBeginPlay(UWorld *World);

protected:
    virtual TSubclassOf<UOnlineSession> GetOnlineSessionClass() override;
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()