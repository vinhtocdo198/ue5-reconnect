// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationMeta/Services/MetaExternalUserInfo.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_META_ENABLED
#include "RedpointEOSPlatformIntegrationMeta/MetaSDK.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_META_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(1891948547, Redpoint::EOS::Platform::Integration::Meta::Services)
{
using namespace ::Redpoint::EOS::Platform::Integration::Meta;

FMetaExternalUserInfo::FMetaExternalUserInfo(ovrID UserId, const FString &DisplayName)
    : LinkedExternalAccounts()
    , Attributes()
{
    this->LinkedExternalAccounts.Add(
        EOS_EExternalAccountType::EOS_EAT_OCULUS,
        FUserExternalAccountInfo{
            DisplayName,
            FMetaSDK::UserIdToString(UserId).Get(TEXT("")),
            EOS_EExternalAccountType::EOS_EAT_OCULUS,
            TOptional<int64>(),
        });
}

const TMap<EOS_EExternalAccountType, const FUserExternalAccountInfo> &FMetaExternalUserInfo::GetLinkedExternalAccounts()
    const
{
    return this->LinkedExternalAccounts;
}

const TMap<FString, FString> &FMetaExternalUserInfo::GetAttributes() const
{
    return this->Attributes;
}

};

namespace Redpoint::EOS::Platform::Integration::Meta::Services
{
REDPOINT_EOS_FILE_NS_EXPORT(1891948547, Redpoint::EOS::Platform::Integration::Meta::Services, FMetaExternalUserInfo)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()