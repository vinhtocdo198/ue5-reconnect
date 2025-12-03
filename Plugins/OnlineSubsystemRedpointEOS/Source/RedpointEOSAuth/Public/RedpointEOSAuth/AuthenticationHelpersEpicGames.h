// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Auth/LinkAccount.h"
#include "RedpointEOSAPI/Auth/Login.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSConfig/Config.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3204027507, Redpoint::EOS::Auth)
{

class REDPOINTEOSAUTH_API FAuthenticationHelpersEpicGames
{
public:
    typedef TDelegate<void(const Redpoint::EOS::API::Auth::FLogin::Result &, bool bThisCallIsResponsibleForLogout)>
        FLoginCompletionDelegate;
    typedef TDelegate<
        void(const Redpoint::EOS::API::Auth::FLinkAccount::Result &, bool bThisCallIsResponsibleForLogout)>
        FLinkAccountCompletionDelegate;

    static void Login(
        Redpoint::EOS::API::FPlatformHandle PlatformHandle,
        const FString &Id,
        const FString &Token,
        EOS_ELoginCredentialType Type,
        EOS_EExternalCredentialType ExternalType,
        bool bPermitInteractive,
        const Redpoint::EOS::Config::IConfig &InConfig,
        const FLoginCompletionDelegate &OnComplete);

    static void LinkAccount(
        Redpoint::EOS::API::FPlatformHandle PlatformHandle,
        EOS_ContinuanceToken ContinuanceToken,
        EOS_ELinkAccountFlags LinkAccountFlags,
        EOS_EpicAccountId LocalUserId,
        const FLinkAccountCompletionDelegate &OnComplete);
};

}

namespace Redpoint::EOS::Auth
{
REDPOINT_EOS_FILE_NS_EXPORT(3204027507, Redpoint::EOS::Auth, FAuthenticationHelpersEpicGames)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()