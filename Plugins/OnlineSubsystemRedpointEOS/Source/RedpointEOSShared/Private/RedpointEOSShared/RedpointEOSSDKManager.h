// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if REDPOINT_EOS_UE_5_5_OR_LATER
#include "RedpointEOSShared/IEOSSDKManager.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if REDPOINT_EOS_UE_5_5_OR_LATER

namespace REDPOINT_EOS_FILE_NS_ID(647807917, Redpoint::EOS::Shared)
{

class FRedpointEOSSDKManager : public IEOSSDKManager
{
public:
    FRedpointEOSSDKManager() = default;
    UE_NONCOPYABLE(FRedpointEOSSDKManager);
    virtual ~FRedpointEOSSDKManager() override = default;

    virtual bool IsInitialized() const override;

    virtual const FEOSSDKPlatformConfig *GetPlatformConfig(const FString &PlatformConfigName, bool bLoadIfMissing)
        override;
    virtual bool AddPlatformConfig(const FEOSSDKPlatformConfig &PlatformConfig, bool bOverwriteExistingConfig) override;
    virtual const FString &GetDefaultPlatformConfigName() override;
    virtual void SetDefaultPlatformConfigName(const FString &PlatformConfigName) override;

    virtual IEOSPlatformHandlePtr CreatePlatform(const FString &PlatformConfigName, FName InstanceName) override;

    virtual IEOSPlatformHandlePtr CreatePlatform(EOS_Platform_Options &PlatformOptions) override;

    virtual TArray<IEOSPlatformHandlePtr> GetActivePlatforms() override;

    virtual FString GetProductName() const override;
    virtual FString GetProductVersion() const override;
    virtual FString GetCacheDirBase() const override;
    virtual FString GetOverrideCountryCode(const EOS_HPlatform Platform) const override;
    virtual FString GetOverrideLocaleCode(const EOS_HPlatform Platform) const override;

    virtual void LogInfo(int32 Indent) const override;
    virtual void LogPlatformInfo(const EOS_HPlatform Platform, int32 Indent) const override;
    virtual void LogAuthInfo(const EOS_HPlatform Platform, const EOS_EpicAccountId LoggedInAccount, int32 Indent)
        const override;
    virtual void LogUserInfo(
        const EOS_HPlatform Platform,
        const EOS_EpicAccountId LoggedInAccount,
        const EOS_EpicAccountId TargetAccount,
        int32 Indent) const override;
    virtual void LogPresenceInfo(
        const EOS_HPlatform Platform,
        const EOS_EpicAccountId LoggedInAccount,
        const EOS_EpicAccountId TargetAccount,
        int32 Indent) const override;
    virtual void LogFriendsInfo(const EOS_HPlatform Platform, const EOS_EpicAccountId LoggedInAccount, int32 Indent)
        const override;
    virtual void LogConnectInfo(const EOS_HPlatform Platform, const EOS_ProductUserId LoggedInAccount, int32 Indent)
        const override;

    virtual void AddCallbackObject(TUniquePtr<FCallbackBase> CallbackObj) override;
};

}

namespace Redpoint::EOS::Shared
{
REDPOINT_EOS_FILE_NS_EXPORT(647807917, Redpoint::EOS::Shared, FRedpointEOSSDKManager)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()