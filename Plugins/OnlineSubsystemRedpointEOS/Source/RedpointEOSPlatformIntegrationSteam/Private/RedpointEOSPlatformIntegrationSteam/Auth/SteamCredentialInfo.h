// Copyright June Rhodes. All Rights Reserved.

#pragma once

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && defined(REDPOINT_EOS_STEAM_ENABLED)

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSPlatformIntegrationSteam/ForwardDecls.h"

namespace REDPOINT_EOS_FILE_NS_ID(4123366627, Redpoint::EOS::Platform::Integration::Steam::Auth)
{

struct FSteamCredentialInfo
{
public:
    FSteamCredentialInfo();

    FString UserId;
    FString EncryptedAppTicket;
    FString SessionTicket;
    TMap<FString, FString> AuthAttributes;
};

}

namespace Redpoint::EOS::Platform::Integration::Steam::Auth
{
REDPOINT_EOS_FILE_NS_EXPORT(4123366627, Redpoint::EOS::Platform::Integration::Steam::Auth, FSteamCredentialInfo)
}

#endif