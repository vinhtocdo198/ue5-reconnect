// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Connect/Login.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSConfig/Config.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2599832381, Redpoint::EOS::Auth)
{

class REDPOINTEOSAUTH_API FAuthenticationHelpersConnect
{
public:
    typedef TDelegate<void(const Redpoint::EOS::API::Connect::FLogin::Result &, bool bThisCallIsResponsibleForLogout)>
        FLoginCompletionDelegate;

    static void Login(
        Redpoint::EOS::API::FPlatformHandle PlatformHandle,
        const FString &Id,
        const FString &Token,
        const FString &LocalDisplayName,
        EOS_EExternalCredentialType Type,
        const FLoginCompletionDelegate &OnComplete);
};

}

namespace Redpoint::EOS::Auth
{
REDPOINT_EOS_FILE_NS_EXPORT(2599832381, Redpoint::EOS::Auth, FAuthenticationHelpersConnect)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()