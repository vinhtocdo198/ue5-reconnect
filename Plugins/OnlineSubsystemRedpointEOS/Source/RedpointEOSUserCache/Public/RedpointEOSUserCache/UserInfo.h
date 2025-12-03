// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Online/CoreOnline.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSUserCache/ExternalUserInfo.h"
#include "RedpointEOSUserCache/UserEpicGamesAccountInfo.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3531376250, Redpoint::EOS::UserCache)
{
using namespace ::Redpoint::EOS::UserCache;

/**
 * Represents information about an Epic Online Services user.
 */
class REDPOINTEOSUSERCACHE_API FUserInfo : public FExternalUserInfo, public TSharedFromThis<FUserInfo>
{
public:
    FUserInfo() = default;
    UE_NONCOPYABLE(FUserInfo);
    virtual ~FUserInfo() override = default;

    /**
     * If true, this user info object has been populated with information.
     */
    virtual bool IsReady() const = 0;

    /**
     * Returns the user ID of the user.
     */
    virtual UE::Online::FAccountId GetUserId() const = 0;

    /**
     * Returns the external account that this user most recently signed in with.
     */
    virtual TOptional<EOS_EExternalAccountType> GetMostRecentlySignedInPlatform() const = 0;

    /**
     * Returns the Epic Games specific information about a user, if this user is linked to an Epic Games account.
     */
    virtual TOptional<const FUserEpicGamesAccountInfo> GetLinkedEpicGamesAccount() const = 0;
};

typedef TSharedRef<FUserInfo> FUserInfoRef;
typedef TSharedPtr<FUserInfo> FUserInfoPtr;

}

namespace Redpoint::EOS::UserCache
{
REDPOINT_EOS_FILE_NS_EXPORT(3531376250, Redpoint::EOS::UserCache, FUserInfo)
REDPOINT_EOS_FILE_NS_EXPORT(3531376250, Redpoint::EOS::UserCache, FUserInfoRef)
REDPOINT_EOS_FILE_NS_EXPORT(3531376250, Redpoint::EOS::UserCache, FUserInfoPtr)
}

REDPOINT_EOS_CODE_GUARD_END()