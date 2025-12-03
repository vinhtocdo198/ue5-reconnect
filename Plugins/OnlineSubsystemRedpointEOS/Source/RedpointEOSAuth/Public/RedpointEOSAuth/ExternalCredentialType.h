// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1348168839, Redpoint::EOS::Auth)
{

class REDPOINTEOSAUTH_API FExternalCredentialType
{
public:
    static EOS_EExternalCredentialType FromString(const FString &InStr);
    static FString ToString(EOS_EExternalCredentialType InType);
};

}

namespace Redpoint::EOS::Auth
{
REDPOINT_EOS_FILE_NS_EXPORT(1348168839, Redpoint::EOS::Auth, FExternalCredentialType)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()