// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSPlatform/RuntimePlatform.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1340308852, Redpoint::EOS::Platform::Desktop)
{

class FRuntimePlatformDesktopBase : public Redpoint::EOS::Platform::IRuntimePlatform
{
private:
    void *DynamicLibraryHandle;
    EOS_Platform_RTCOptions RTCOptions;

protected:
    FRuntimePlatformDesktopBase() = default;

    virtual void GetSearchPathsAndDllName(
#if WITH_EDITOR
        const FString &BaseDir,
#endif
        TArray<FString> &SearchPaths,
        FString &DllName) = 0;
    virtual void PostLoadSuccess(const FString &BaseSearchPath);
    virtual bool IsRTCOptionsValid() const;
    virtual void *GetRTCPlatformSpecificOptions();

public:
    UE_NONCOPYABLE(FRuntimePlatformDesktopBase);
    virtual ~FRuntimePlatformDesktopBase() override = default;

    virtual void Load() override;
    virtual void Unload() override;
    virtual bool IsValid() override;
    virtual void *GetSystemInitializeOptions() override;
    virtual EOS_Platform_RTCOptions *GetRTCOptions() override;
    virtual FString GetCacheDirectory() override;
    virtual void ClearStoredEASRefreshToken(int32 LocalUserNum) override {};
#if !UE_BUILD_SHIPPING
    virtual FString GetPathToEASAutomatedTestingCredentials() override;
#endif
    virtual bool UseCrossPlatformFriendStorage() const override;

#if !defined(UE_SERVER) || !UE_SERVER
    virtual bool ShouldPollLifecycleApplicationStatus() const override;
    virtual bool ShouldPollLifecycleNetworkStatus() const override;
    virtual bool PollLifecycleApplicationStatus(EOS_EApplicationStatus &OutApplicationStatus) const override;
    virtual bool PollLifecycleNetworkStatus(EOS_ENetworkStatus &OutNetworkStatus) const override;
#else
    virtual void SetLifecycleForNewPlatformInstance(Redpoint::EOS::API::FPlatformHandle InPlatform) override;
#endif
};

}

namespace Redpoint::EOS::Platform::Desktop
{
REDPOINT_EOS_FILE_NS_EXPORT(1340308852, Redpoint::EOS::Platform::Desktop, FRuntimePlatformDesktopBase)
}

REDPOINT_EOS_CODE_GUARD_END()