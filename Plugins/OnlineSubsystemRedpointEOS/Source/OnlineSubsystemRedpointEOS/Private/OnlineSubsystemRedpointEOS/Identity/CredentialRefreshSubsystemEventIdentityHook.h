// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSIdentity/IdentityHook.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1533336711, Redpoint::EOS::OnlineSubsystemRedpointEOS::Identity)
{
using namespace ::Redpoint::EOS::Identity;

class FCredentialRefreshSubsystemEventIdentityHook : public IIdentityHook
{
public:
    FCredentialRefreshSubsystemEventIdentityHook() = default;
    UE_NONCOPYABLE(FCredentialRefreshSubsystemEventIdentityHook);
    ~FCredentialRefreshSubsystemEventIdentityHook() = default;

    virtual FString GetHookDebugName() const override
    {
        return TEXT("CredentialRefreshSubsystemEventIdentityHook");
    }

    virtual void OnPostCredentialRefresh(
        const FIdentityHookContext &InContext,
        const FIdentityUserRef &InUser,
        bool bWasSuccessful) override;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::Identity
{
REDPOINT_EOS_FILE_NS_EXPORT(
    1533336711,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::Identity,
    FCredentialRefreshSubsystemEventIdentityHook)
}

#endif