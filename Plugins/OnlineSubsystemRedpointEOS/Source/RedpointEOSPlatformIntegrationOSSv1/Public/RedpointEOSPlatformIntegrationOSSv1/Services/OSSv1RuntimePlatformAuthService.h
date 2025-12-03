// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "Interfaces/OnlineExternalUIInterface.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSPlatform/Services/RuntimePlatformAuthService.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3619559314, Redpoint::EOS::Platform::Integration::OSSv1::Services)
{
using namespace ::Redpoint::EOS::Platform::Services;
using namespace ::Redpoint::EOS::Auth;

class REDPOINTEOSPLATFORMINTEGRATIONOSSV1_API FOSSv1RuntimePlatformAuthService
    : public IRuntimePlatformAuthService,
      public TSharedFromThis<FOSSv1RuntimePlatformAuthService>
{
private:
    FName PlatformName;
    EOS_EExternalCredentialType ObtainedCredentialType;
    FString AuthenticatedWithValue;
    FString TokenAuthAttributeName;

public:
    FOSSv1RuntimePlatformAuthService(
        FName InPlatformName,
        EOS_EExternalCredentialType InObtainedCredentialType,
        const FString &InAuthenticatedWithValue,
        const FString &InTokenAuthAttributeName);
    UE_NONCOPYABLE(FOSSv1RuntimePlatformAuthService);
    virtual ~FOSSv1RuntimePlatformAuthService() override = default;

private:
    void GetInterfaces(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        TSharedPtr<IOnlineIdentity> &OutIdentityInterface,
        TSharedPtr<IOnlineExternalUI> &OutExternalUIInterface) const;

public:
    virtual FName GetPlatformName() const override;
    virtual int32 GetSelectionScore(const FRuntimePlatformServiceCallContextRef &CallContext) const override;
    virtual void EnsureSignedInOnLocalPlatform(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FOnEnsureSignedInOnLocalPlatform &OnComplete) override;

private:
    void OnLoginUIClosed(
        TSharedPtr<const FUniqueNetId> UniqueId,
        const int ControllerIndex,
        const FOnlineError &Error,
        FRuntimePlatformServiceCallContextRef CallContext,
        FOnEnsureSignedInOnLocalPlatform OnComplete) const;
    void StartLoginCall(FRuntimePlatformServiceCallContextRef CallContext, FOnEnsureSignedInOnLocalPlatform OnComplete)
        const;
    void OnLoginComplete(
        int32 LocalUserNum,
        bool bWasSuccessful,
        const FUniqueNetId &UserId,
        const FString &Error,
        FRuntimePlatformServiceCallContextRef CallContext,
        FOnEnsureSignedInOnLocalPlatform OnComplete) const;

public:
    virtual void IsOnlinePlayPermitted(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FOnIsOnlinePlayPermitted &OnComplete) override;
    virtual void GetCredentialsFromLocalPlatformDuringAuthenticationGraph(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const TSharedRef<FAuthenticationGraphState> &State,
        const FOnGetCredentialsFromLocalPlatformDuringAuthenticationGraph &OnComplete) override;
};

}; // namespace REDPOINT_EOS_FILE_NS_ID(3619559314,Redpoint::EOS::Platform::Integration::OSSv1::Services)

namespace Redpoint::EOS::Platform::Integration::OSSv1::Services
{
REDPOINT_EOS_FILE_NS_EXPORT(
    3619559314,
    Redpoint::EOS::Platform::Integration::OSSv1::Services,
    FOSSv1RuntimePlatformAuthService)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()