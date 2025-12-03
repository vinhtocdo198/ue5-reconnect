// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSPlatform/RuntimePlatform.h"

namespace Redpoint::EOS::Platform::IOS
{

class FRuntimePlatformIOS : public Redpoint::EOS::Platform::IRuntimePlatform
{
private:
    EOS_Platform_RTCOptions RTCOptions;

public:
    FRuntimePlatformIOS() = default;
    UE_NONCOPYABLE(FRuntimePlatformIOS);
    virtual ~FRuntimePlatformIOS() = default;

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
};

}