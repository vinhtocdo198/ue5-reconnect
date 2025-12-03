// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_META_ENABLED
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSPlatformIntegrationMeta/MetaSDK.h"
#include "RedpointEOSUserCache/ExternalUserInfo.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_META_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(1891948547, Redpoint::EOS::Platform::Integration::Meta::Services)
{
using namespace ::Redpoint::EOS::UserCache;

class FMetaExternalUserInfo : public FExternalUserInfo
{
private:
    TMap<EOS_EExternalAccountType, const FUserExternalAccountInfo> LinkedExternalAccounts;
    TMap<FString, FString> Attributes;

public:
    FMetaExternalUserInfo(ovrID UserId, const FString &DisplayName);
    UE_NONCOPYABLE(FMetaExternalUserInfo);
    virtual ~FMetaExternalUserInfo() override = default;

    virtual const TMap<EOS_EExternalAccountType, const FUserExternalAccountInfo> &GetLinkedExternalAccounts()
        const override;
    virtual const TMap<FString, FString> &GetAttributes() const override;
};

};

namespace Redpoint::EOS::Platform::Integration::Meta::Services
{
REDPOINT_EOS_FILE_NS_EXPORT(1891948547, Redpoint::EOS::Platform::Integration::Meta::Services, FMetaExternalUserInfo)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()