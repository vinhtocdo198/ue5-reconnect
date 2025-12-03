// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSUserCache/UserInfo.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3225458625, Redpoint::EOS::Friends::Unification)
{
using namespace ::Redpoint::EOS::UserCache;

class FUnifiedUserInfo : public FUserInfo
{
private:
    TSharedRef<FUserInfo> WrappedUserInfo;
    TMap<FString, TSharedPtr<FExternalUserInfo>> RuntimePlatformIntegrationExternalFriends;
    TOptional<TMap<FString, FString>> MaterializedAttributes;
    TOptional<FString> AvatarUrl;

public:
    FUnifiedUserInfo(
        const TSharedRef<FUserInfo> &InWrappedUserInfo,
        const TMap<FString, TSharedPtr<FExternalUserInfo>> &InRuntimePlatformIntegrationExternalFriends,
        const TOptional<FString> &InAvatarUrl);
    UE_NONCOPYABLE(FUnifiedUserInfo);
    virtual ~FUnifiedUserInfo() override = default;

    virtual bool IsReady() const override;
    virtual UE::Online::FAccountId GetUserId() const override;
    virtual TOptional<EOS_EExternalAccountType> GetMostRecentlySignedInPlatform() const override;
    virtual TOptional<const FUserEpicGamesAccountInfo> GetLinkedEpicGamesAccount() const override;
    virtual const TMap<EOS_EExternalAccountType, const FUserExternalAccountInfo> &GetLinkedExternalAccounts()
        const override;
    virtual const TMap<FString, FString> &GetAttributes() const override;

private:
    const TMap<FString, FString> &MaterializeAttributes();
};

typedef TSharedRef<FUnifiedUserInfo> FUnifiedUserInfoRef;
typedef TSharedPtr<FUnifiedUserInfo> FUnifiedUserInfoPtr;

}

namespace Redpoint::EOS::Friends::Unification
{
REDPOINT_EOS_FILE_NS_EXPORT(3225458625, Redpoint::EOS::Friends::Unification, FUnifiedUserInfo)
REDPOINT_EOS_FILE_NS_EXPORT(3225458625, Redpoint::EOS::Friends::Unification, FUnifiedUserInfoRef)
REDPOINT_EOS_FILE_NS_EXPORT(3225458625, Redpoint::EOS::Friends::Unification, FUnifiedUserInfoPtr)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()