// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSUserCache/UserExternalAccountInfo.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1941373948, Redpoint::EOS::UserCache)
{
using namespace ::Redpoint::EOS::UserCache;

/**
 * Represents the externally linked accounts of an Epic Online Services user, or the account information of a locally
 * obtained but external user (such as a Steam friend that does not have an associated Epic Online Services user
 * account).
 */
class REDPOINTEOSUSERCACHE_API FExternalUserInfo
{
public:
    FExternalUserInfo() = default;
    UE_NONCOPYABLE(FExternalUserInfo);
    virtual ~FExternalUserInfo() = default;

    /**
     * Returns the external accounts linked to this user.
     */
    virtual const TMap<EOS_EExternalAccountType, const FUserExternalAccountInfo> &GetLinkedExternalAccounts() const = 0;

    /**
     * Returns the attributes of this user.
     */
    virtual const TMap<FString, FString> &GetAttributes() const = 0;
};

typedef TSharedRef<FExternalUserInfo> FExternalUserInfoRef;
typedef TSharedPtr<FExternalUserInfo> FExternalUserInfoPtr;

}

namespace Redpoint::EOS::UserCache
{
REDPOINT_EOS_FILE_NS_EXPORT(1941373948, Redpoint::EOS::UserCache, FExternalUserInfo)
REDPOINT_EOS_FILE_NS_EXPORT(1941373948, Redpoint::EOS::UserCache, FExternalUserInfoRef)
REDPOINT_EOS_FILE_NS_EXPORT(1941373948, Redpoint::EOS::UserCache, FExternalUserInfoPtr)
}

REDPOINT_EOS_CODE_GUARD_END()