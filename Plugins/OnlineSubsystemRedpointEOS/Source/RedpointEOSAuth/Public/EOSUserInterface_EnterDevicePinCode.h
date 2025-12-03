// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "EOSUserInterface_EnterDevicePinCode_Context.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "UObject/Interface.h"

#include "EOSUserInterface_EnterDevicePinCode.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UINTERFACE(MinimalAPI, Blueprintable)
class UEOSUserInterface_EnterDevicePinCode : public UInterface
{
    GENERATED_BODY()
};

class REDPOINTEOSAUTH_API IEOSUserInterface_EnterDevicePinCode
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "EOS|User Interface")
    void SetupUserInterface(
        UEOSUserInterface_EnterDevicePinCode_Context *Context,
        const FString &VerificationUrl,
        const FString &PinCode);
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()
