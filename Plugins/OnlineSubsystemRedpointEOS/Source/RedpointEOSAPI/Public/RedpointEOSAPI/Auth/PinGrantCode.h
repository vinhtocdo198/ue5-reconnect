// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Auth
{

class REDPOINTEOSAPI_API FPinGrantInfo
{
public:
    FString UserCode;
    FString VerificationURI;
    int32 ExpiresIn;
    FString VerificationURIComplete;
};

}

REDPOINT_EOS_CODE_GUARD_END()