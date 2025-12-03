// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Delegates/Delegate.h"
#include "RedpointEOSAuth/AuthenticationGraphRefreshEOSCredentialsInfo.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2085313665, Redpoint::EOS::Auth)
{
using namespace ::Redpoint::EOS::Auth;

/**
 * This type of delegate will be called by the identity subsystem when credentials need to be refreshed
 * for the associated EOS user. The implementation of this delegate must handle calling EOS_Connect_Login
 * for the EOS user. It's expected that the product user ID will be captured by the lambda that this event
 * is wired up to.
 *
 * It is NOT SAFE for the callback to capture any authentication graph pointers, as they will all have been
 * freed when this callback is invoked. Instead, use the information in the Info parameter.
 */
typedef TDelegate<void(const TSharedRef<FAuthenticationGraphRefreshEOSCredentialsInfo> &Info)>
    FAuthenticationGraphRefreshEOSCredentials;

}

namespace Redpoint::EOS::Auth
{
REDPOINT_EOS_FILE_NS_EXPORT(2085313665, Redpoint::EOS::Auth, FAuthenticationGraphRefreshEOSCredentials)
}

REDPOINT_EOS_CODE_GUARD_END()