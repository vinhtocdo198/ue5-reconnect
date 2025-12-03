// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSIdentity/IdentityHook.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(19988241, Redpoint::EOS::OnlineSubsystemRedpointEOS::Identity)
{
using namespace ::Redpoint::EOS::Identity;

class FAuthenticationUserInterfaceProviderIdentityHook : public IIdentityHook
{
public:
    FAuthenticationUserInterfaceProviderIdentityHook() = default;
    UE_NONCOPYABLE(FAuthenticationUserInterfaceProviderIdentityHook);
    ~FAuthenticationUserInterfaceProviderIdentityHook() = default;

    virtual FString GetHookDebugName() const override
    {
        return TEXT("AuthenticationUserInterfaceProviderIdentityHook");
    }

    virtual TSharedPtr<Redpoint::EOS::Auth::IAuthenticationGraphUserInterface> OnAuthenticationUserInterfaceRequired(
        const FIdentityHookContext &InContext) override;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::Identity
{
REDPOINT_EOS_FILE_NS_EXPORT(
    19988241,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::Identity,
    FAuthenticationUserInterfaceProviderIdentityHook)
}

#endif