// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if REDPOINT_EOS_UE_5_5_OR_LATER
#include "RedpointEOSShared/IEOSSDKManager.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if REDPOINT_EOS_UE_5_5_OR_LATER

namespace REDPOINT_EOS_FILE_NS_ID(3627867287, Redpoint::EOS::Shared)
{

class FRedpointEOSPlatformHandle : public IEOSPlatformHandle
{
public:
    FRedpointEOSPlatformHandle(EOS_HPlatform InPlatformHandle);

    virtual ~FRedpointEOSPlatformHandle() override = default;

    virtual void Tick() override;

    virtual FString GetConfigName() const override;
    virtual FString GetOverrideCountryCode() const override;
    virtual FString GetOverrideLocaleCode() const override;

    virtual void LogInfo(int32 Indent) const override;
    virtual void LogAuthInfo(const EOS_EpicAccountId LoggedInAccount, int32 Indent) const override;
    virtual void LogUserInfo(
        const EOS_EpicAccountId LoggedInAccount,
        const EOS_EpicAccountId TargetAccount,
        int32 Indent) const override;
    virtual void LogPresenceInfo(
        const EOS_EpicAccountId LoggedInAccount,
        const EOS_EpicAccountId TargetAccount,
        int32 Indent) const override;
    virtual void LogFriendsInfo(const EOS_EpicAccountId LoggedInAccount, int32 Indent) const override;
    virtual void LogConnectInfo(const EOS_ProductUserId LoggedInAccount, int32 Indent) const override;
};

}

namespace Redpoint::EOS::Shared
{
REDPOINT_EOS_FILE_NS_EXPORT(3627867287, Redpoint::EOS::Shared, FRedpointEOSPlatformHandle)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()