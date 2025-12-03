// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "FabSessionProvider.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3493870958, Redpoint::Fab::Authentication)
{
using namespace ::Redpoint::EOS::API;

class FFabRedpointSessionProvider : public IFabSessionProvider, public TSharedFromThis<FFabRedpointSessionProvider>
{
private:
    FOnFabLoggedIn LoggedInDelegate;
    FPlatformOptionalRefCountedHandle PlatformRefHandle;

public:
    FFabRedpointSessionProvider() = default;
    UE_NONCOPYABLE(FFabRedpointSessionProvider);
    virtual ~FFabRedpointSessionProvider() override = default;

    virtual void Init(
        const FString &ProductId,
        const FString &SandboxId,
        const FString &DeploymentId,
        const FString &ClientId,
        const FString &ClientSecret,
        const FString &EncryptionKey,
        bool bIsGamedev) override;
    virtual void Shutdown() override;
    virtual void Login() override;
    virtual FOnFabLoggedIn &OnLoggedIn() override;
    virtual void Logout() override;
    virtual FString GetAuthToken() override;
    virtual FString GetRefreshToken() override;
};

}

namespace Redpoint::Fab::Authentication
{
REDPOINT_EOS_FILE_NS_EXPORT(3493870958, Redpoint::Fab::Authentication, FFabRedpointSessionProvider);
}

REDPOINT_EOS_CODE_GUARD_END()