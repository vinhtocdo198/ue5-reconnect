// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Delegates/Delegate.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(4264127615, Redpoint::EOS::Auth)
{

/** Called by the implementor of the refresh callback to indicate that credentials have been refreshed. */
typedef TDelegate<void(bool bWasSuccessful)> FAuthenticationGraphRefreshEOSCredentialsComplete;

}

namespace Redpoint::EOS::Auth
{
REDPOINT_EOS_FILE_NS_EXPORT(4264127615, Redpoint::EOS::Auth, FAuthenticationGraphRefreshEOSCredentialsComplete)
}

REDPOINT_EOS_CODE_GUARD_END()