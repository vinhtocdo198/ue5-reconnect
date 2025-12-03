// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Delegates/Delegate.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3488434266, Redpoint::EOS::Auth)
{

typedef TDelegate<void(bool bWasSuccessful)> FOnlineExternalCredentialsRefreshComplete;

}

namespace Redpoint::EOS::Auth
{
REDPOINT_EOS_FILE_NS_EXPORT(3488434266, Redpoint::EOS::Auth, FOnlineExternalCredentialsRefreshComplete)
}

REDPOINT_EOS_CODE_GUARD_END()