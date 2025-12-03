// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSUserCache/ExternalUserInfo.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(57052363, Redpoint::EOS::Platform::Integration::Epic)
{
using namespace ::Redpoint::EOS::UserCache;

class FEpicExternalUserInfo : public FExternalUserInfo
{
private:
    TMap<EOS_EExternalAccountType, const FUserExternalAccountInfo> LinkedExternalAccounts;
    TMap<FString, FString> Attributes;

public:
    FEpicExternalUserInfo(EOS_EpicAccountId InEpicAccountId);
    UE_NONCOPYABLE(FEpicExternalUserInfo);
    virtual ~FEpicExternalUserInfo() override = default;

    virtual const TMap<EOS_EExternalAccountType, const FUserExternalAccountInfo> &GetLinkedExternalAccounts()
        const override;
    virtual const TMap<FString, FString> &GetAttributes() const override;
};

};

namespace Redpoint::EOS::Platform::Integration::Epic
{
REDPOINT_EOS_FILE_NS_EXPORT(57052363, Redpoint::EOS::Platform::Integration::Epic, FEpicExternalUserInfo)
}

REDPOINT_EOS_CODE_GUARD_END()