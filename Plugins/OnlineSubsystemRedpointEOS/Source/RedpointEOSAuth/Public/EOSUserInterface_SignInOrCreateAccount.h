// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "EOSUserInterface_SignInOrCreateAccount_Context.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "UObject/Interface.h"

#include "EOSUserInterface_SignInOrCreateAccount.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UINTERFACE(MinimalAPI, Blueprintable)
class UEOSUserInterface_SignInOrCreateAccount : public UInterface
{
    GENERATED_BODY()
};

class REDPOINTEOSAUTH_API IEOSUserInterface_SignInOrCreateAccount
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "EOS|User Interface")
    void SetupUserInterface(UEOSUserInterface_SignInOrCreateAccount_Context *Context);
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()
