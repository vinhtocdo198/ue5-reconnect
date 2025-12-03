// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationOSSv1/WrappedExternalUserInfo.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(761091540, Redpoint::EOS::Platform::Integration::OSSv1)
{

FWrappedExternalUserInfo::FWrappedExternalUserInfo(
    EOS_EExternalAccountType AccountIdType,
    const FString &AccountId,
    const FString &DisplayName)
    : FExternalUserInfo()
    , LinkedExternalAccounts()
    , Attributes()
{
    this->LinkedExternalAccounts.Add(
        AccountIdType,
        FUserExternalAccountInfo{
            DisplayName,
            AccountId,
            AccountIdType,
            TOptional<int64>(),
        });
}

const TMap<EOS_EExternalAccountType, const FUserExternalAccountInfo> &FWrappedExternalUserInfo::
    GetLinkedExternalAccounts() const
{
    return this->LinkedExternalAccounts;
}

const TMap<FString, FString> &FWrappedExternalUserInfo::GetAttributes() const
{
    return this->Attributes;
}

};

REDPOINT_EOS_CODE_GUARD_END()