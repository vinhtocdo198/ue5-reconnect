// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSFriends/Database/SerializedCachedFriend.h"
#include "RedpointEOSUserCache/ExternalUserInfo.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(4144957489, Redpoint::EOS::Friends::Unification)
{
using namespace ::Redpoint::EOS::Friends::Database;
using namespace ::Redpoint::EOS::UserCache;

class FCachedExternalUserInfo : public FExternalUserInfo
{
private:
    TMap<EOS_EExternalAccountType, const FUserExternalAccountInfo> LinkedExternalAccounts;
    TMap<FString, FString> Attributes;
    FString AvatarUrl;

public:
    FCachedExternalUserInfo(const FSerializedCachedFriend &SerializedCachedFriend);
    UE_NONCOPYABLE(FCachedExternalUserInfo);
    virtual ~FCachedExternalUserInfo() override = default;

    virtual const TMap<EOS_EExternalAccountType, const FUserExternalAccountInfo> &GetLinkedExternalAccounts()
        const override;
    virtual const TMap<FString, FString> &GetAttributes() const override;

    TOptional<FString> GetAvatarUrl();
};

};

namespace Redpoint::EOS::Friends::Unification
{
REDPOINT_EOS_FILE_NS_EXPORT(4144957489, Redpoint::EOS::Friends::Unification, FCachedExternalUserInfo)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()