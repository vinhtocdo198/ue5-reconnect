// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSIdentity/IdentityHook.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3758799363, Redpoint::EOS::OnlineSubsystemRedpointEOS::Identity)
{
using namespace ::Redpoint::EOS::Identity;

class FPrecachePlayerDataStorageFilesIdentityHook : public IIdentityHook,
                                                    public TSharedFromThis<FPrecachePlayerDataStorageFilesIdentityHook>
{
public:
    FPrecachePlayerDataStorageFilesIdentityHook() = default;
    UE_NONCOPYABLE(FPrecachePlayerDataStorageFilesIdentityHook);
    ~FPrecachePlayerDataStorageFilesIdentityHook() = default;

    virtual FString GetHookDebugName() const override
    {
        return TEXT("PrecachePlayerDataStorageFilesIdentityHook");
    }

    virtual void OnStartSystem(
        const FIdentityHookContext &InContext,
        const FIdentityUserRef &InUser,
        const FOnStartSystemComplete &InOnComplete) override;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::Identity
{
REDPOINT_EOS_FILE_NS_EXPORT(
    3758799363,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::Identity,
    FPrecachePlayerDataStorageFilesIdentityHook)
}

#endif