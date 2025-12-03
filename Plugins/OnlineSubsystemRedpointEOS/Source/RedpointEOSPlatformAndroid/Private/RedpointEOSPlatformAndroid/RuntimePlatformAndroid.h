// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSPlatform/RuntimePlatform.h"

namespace Redpoint::EOS::Platform::Android
{

class FRuntimePlatformAndroid : public Redpoint::EOS::Platform::IRuntimePlatform
{
private:
    TSharedPtr<EOS_Android_InitializeOptions> Opts = {};
    EOS_Platform_RTCOptions RTCOptions;

public:
    FRuntimePlatformAndroid() = default;
    UE_NONCOPYABLE(FRuntimePlatformAndroid);
    virtual ~FRuntimePlatformAndroid() = default;

    virtual void Load() override;
    virtual void Unload() override;
    virtual bool IsValid() override;
    virtual void *GetSystemInitializeOptions() override;
    virtual EOS_Platform_RTCOptions *GetRTCOptions() override;
    virtual FString GetCacheDirectory() override;
    virtual void ClearStoredEASRefreshToken(int32 LocalUserNum) override{};
#if !UE_BUILD_SHIPPING
    virtual FString GetPathToEASAutomatedTestingCredentials() override;
#endif
    virtual bool UseCrossPlatformFriendStorage() const override;
    virtual bool IsDelegatedSubsystemForciblyExcluded(FName InSubsystemName) const override;

    static const char *InternalPath;
    static const char *ExternalPath;
};

}