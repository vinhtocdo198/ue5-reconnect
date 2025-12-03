// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Containers/UnrealString.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSAuth/AuthenticationGraphRefreshEOSCredentialsComplete.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "UObject/SoftObjectPtr.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3669677228, Redpoint::EOS::Auth)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Auth;

/**
 * Information provided to the refresh callback such as whose credentials are being refreshed, and references to the EOS
 * APIs.
 *
 * It is NOT SAFE for refresh callbacks to capture references to the authentication graph or it's state, because the
 * graph and state will have been released by the time the refresh callback runs. Refresh callbacks are only permitted
 * to reference data that they were passed by FAuthenticationGraphRefreshEOSCredentialsInfo.
 */
struct REDPOINTEOSAUTH_API FAuthenticationGraphRefreshEOSCredentialsInfo
    : public TSharedFromThis<FAuthenticationGraphRefreshEOSCredentialsInfo>
{
public:
    FAuthenticationGraphRefreshEOSCredentialsInfo(
        TSoftObjectPtr<UWorld> InWorld,
        int32 InLocalUserNum,
        FPlatformHandle InPlatformHandle,
        const TMap<FString, FString> &InExistingUserAuthAttributes)
        : World(MoveTemp(InWorld))
        , LocalUserNum(InLocalUserNum)
        , PlatformHandle(InPlatformHandle)
        , ExistingUserAuthAttributes(InExistingUserAuthAttributes)
        , SetUserAuthAttributes()
        , DeleteUserAuthAttributes()
        , OnComplete(){};
    UE_NONCOPYABLE(FAuthenticationGraphRefreshEOSCredentialsInfo);

    /** The current world. */
    TSoftObjectPtr<UWorld> World;

    /** The local user whose credential is being refreshed. */
    int32 LocalUserNum;

    /** The EOS platform handle. */
    FPlatformHandle PlatformHandle;

    /** Entries in this map will be the existing auth attributes. If you e.g. need to refresh with a refresh token, you
     * can store the refresh token originally and then pull it out here (and then put the new refresh token in by
     * setting it into SetUserAuthAttributes. */
    TMap<FString, FString> ExistingUserAuthAttributes;

    /** Entries in this map will be added (or overwritten) in the auth attributes of the FOnlineUserAccountEOS. */
    TMap<FString, FString> SetUserAuthAttributes;

    /** Entries in this set will be deleted from the auth attributes of the FOnlineUserAccountEOS. */
    TSet<FString> DeleteUserAuthAttributes;

    /** After the refresh is successfully and the attribute maps have been filled, call this method to apply the changes
     * to the FOnlineUserAccountEOS. */
    FAuthenticationGraphRefreshEOSCredentialsComplete OnComplete;
};

}

namespace Redpoint::EOS::Auth
{
REDPOINT_EOS_FILE_NS_EXPORT(3669677228, Redpoint::EOS::Auth, FAuthenticationGraphRefreshEOSCredentialsInfo)
}

REDPOINT_EOS_CODE_GUARD_END()