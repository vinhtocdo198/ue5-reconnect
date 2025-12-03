// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Features/IModularFeatures.h"

typedef TMulticastDelegate<void(const FString &AccessToken)> FOnFabLoggedIn;

// IMPORTANT: The order of virtual functions in this header must not be changed or
// removed, as external plugins will have a copy of it.
class IFabSessionProvider : public IModularFeature
{
public:
    virtual ~IFabSessionProvider() = default;

    virtual void Init(
        const FString &ProductId,
        const FString &SandboxId,
        const FString &DeploymentId,
        const FString &ClientId,
        const FString &ClientSecret,
        const FString &EncryptionKey,
        bool bIsGamedev) = 0;
    virtual void Shutdown() = 0;
    virtual void Login() = 0;
    virtual FOnFabLoggedIn &OnLoggedIn() = 0;
    virtual void Logout() = 0;
    virtual FString GetAuthToken() = 0;
    virtual FString GetRefreshToken() = 0;
};