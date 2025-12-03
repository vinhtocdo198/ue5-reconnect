// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationEpic/EpicExternalUserInfo.h"

#include "RedpointEOSCore/Id/Id.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(57052363, Redpoint::EOS::Platform::Integration::Epic)
{
using namespace ::Redpoint::EOS::Core::Id;

FEpicExternalUserInfo::FEpicExternalUserInfo(EOS_EpicAccountId InEpicAccountId)
    : FExternalUserInfo()
    , LinkedExternalAccounts()
    , Attributes()
{
    this->LinkedExternalAccounts.Add(
        EOS_EExternalAccountType::EOS_EAT_EPIC,
        FUserExternalAccountInfo{
            TEXT(""),
            GetEpicGamesUserIdString(InEpicAccountId),
            EOS_EExternalAccountType::EOS_EAT_EPIC,
            TOptional<int64>(),
        });
}

const TMap<EOS_EExternalAccountType, const FUserExternalAccountInfo> &FEpicExternalUserInfo::GetLinkedExternalAccounts()
    const
{
    return this->LinkedExternalAccounts;
}

const TMap<FString, FString> &FEpicExternalUserInfo::GetAttributes() const
{
    return this->Attributes;
}

};

REDPOINT_EOS_CODE_GUARD_END()