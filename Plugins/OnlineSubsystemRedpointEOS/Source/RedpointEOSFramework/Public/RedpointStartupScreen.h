// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "UObject/Interface.h"

#include "RedpointStartupScreen.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UINTERFACE(MinimalAPI, Blueprintable)
class URedpointStartupScreen : public UInterface
{
    GENERATED_BODY()
};

DECLARE_DYNAMIC_DELEGATE(FOnStartupScreenStartLogin);

class REDPOINTEOSFRAMEWORK_API IRedpointStartupScreen
{
    GENERATED_BODY()

public:
    /**
     * This callback is invoked when the startup screen is first displayed, before any login attempt is made. You can
     * either call OnStartLogin immediately to start the login process, or you can defer invoking that callback until
     * later (if you need to display a EULA or "Start Game" button).
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Redpoint EOS Online Framework")
    void OnStartupScreenDisplayed(const FOnStartupScreenStartLogin &OnStartLogin);

    /**
     * This callback is invoked when the progress message on the startup screen should be updated.
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Redpoint EOS Online Framework")
    void UpdateProgressMessage(const FText &Message);

    /**
     * This callback is invoked when the login attempt failed. The player usually needs to either retry logging in (with
     * different login credentials), or take some external action that can't be addressed in-game.
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Redpoint EOS Online Framework")
    void OnLoginFailed(const FText &LoginError, const FOnStartupScreenStartLogin &OnRetryLogin);
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()
