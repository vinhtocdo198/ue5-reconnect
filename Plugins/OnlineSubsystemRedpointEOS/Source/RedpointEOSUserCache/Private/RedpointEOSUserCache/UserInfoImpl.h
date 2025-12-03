// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSUserCache/UserInfo.h"
#include "RedpointEOSUserCache/UserInfoData.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1948841835, Redpoint::EOS::UserCache)
{
using namespace ::Redpoint::EOS::UserCache;

class FUserInfoImpl : public FUserInfo
{
private:
    UE::Online::FAccountId UserId;
    FUserInfoDataConstPtr UserInfoData;
    TMap<FString, FString> UnreadyAttributes;
    static TMap<EOS_EExternalAccountType, const FUserExternalAccountInfo> UnreadyLinkedExternalAccounts;

public:
    FUserInfoImpl(const UE::Online::FAccountId &InUserId);
    FUserInfoImpl(const UE::Online::FAccountId &InUserId, const FUserInfoDataConstRef &InExistingData);
    UE_NONCOPYABLE(FUserInfoImpl);
    virtual ~FUserInfoImpl() override = default;

    virtual bool IsReady() const override;
    virtual UE::Online::FAccountId GetUserId() const override;
    virtual TOptional<EOS_EExternalAccountType> GetMostRecentlySignedInPlatform() const override;
    virtual const TMap<EOS_EExternalAccountType, const FUserExternalAccountInfo> &GetLinkedExternalAccounts()
        const override;
    virtual TOptional<const FUserEpicGamesAccountInfo> GetLinkedEpicGamesAccount() const override;
    virtual const TMap<FString, FString> &GetAttributes() const override;

    void SetUserInfoData(const FUserInfoDataConstRef &InUserInfoData);
};

}

namespace Redpoint::EOS::UserCache
{
REDPOINT_EOS_FILE_NS_EXPORT(1948841835, Redpoint::EOS::UserCache, FUserInfoImpl)
}

REDPOINT_EOS_CODE_GUARD_END()