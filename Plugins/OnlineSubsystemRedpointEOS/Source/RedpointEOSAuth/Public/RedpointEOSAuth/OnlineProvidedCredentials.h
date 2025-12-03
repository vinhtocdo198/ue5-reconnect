// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2920558761, Redpoint::EOS::Auth)
{

/**
 * This is effectively a copy of FOnlineAccountCredentials from OSSv1, such that we can pass this data into the
 * authentication graph system (which can't have hard dependencies on OSSv1 to ensure it can work in OSSv2 games).
 */
struct REDPOINTEOSAUTH_API FOnlineProvidedCredentials
{
public:
    FString Type;
    FString Id;
    FString Token;
};

}

namespace Redpoint::EOS::Auth
{
REDPOINT_EOS_FILE_NS_EXPORT(2920558761, Redpoint::EOS::Auth, FOnlineProvidedCredentials)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()