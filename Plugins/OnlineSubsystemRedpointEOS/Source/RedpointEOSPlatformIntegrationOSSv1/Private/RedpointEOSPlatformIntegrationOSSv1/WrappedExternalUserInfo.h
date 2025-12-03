// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSUserCache/ExternalUserInfo.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(761091540, Redpoint::EOS::Platform::Integration::OSSv1)
{
using namespace ::Redpoint::EOS::UserCache;

class FWrappedExternalUserInfo : public FExternalUserInfo
{
private:
    TMap<EOS_EExternalAccountType, const FUserExternalAccountInfo> LinkedExternalAccounts;
    TMap<FString, FString> Attributes;

public:
    FWrappedExternalUserInfo(
        EOS_EExternalAccountType AccountIdType,
        const FString &AccountId,
        const FString &DisplayName);
    UE_NONCOPYABLE(FWrappedExternalUserInfo);
    virtual ~FWrappedExternalUserInfo() override = default;

    virtual const TMap<EOS_EExternalAccountType, const FUserExternalAccountInfo> &GetLinkedExternalAccounts()
        const override;
    virtual const TMap<FString, FString> &GetAttributes() const override;
};

};

namespace Redpoint::EOS::Platform::Integration::OSSv1
{
REDPOINT_EOS_FILE_NS_EXPORT(761091540, Redpoint::EOS::Platform::Integration::OSSv1, FWrappedExternalUserInfo)
}

REDPOINT_EOS_CODE_GUARD_END()