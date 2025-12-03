// Copyright June Rhodes. All Rights Reserved.

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && defined(REDPOINT_EOS_STEAM_ENABLED)

#include "RedpointEOSPlatformIntegrationSteam/Auth/SteamCredentialInfo.h"

namespace REDPOINT_EOS_FILE_NS_ID(4123366627, Redpoint::EOS::Platform::Integration::Steam::Auth)
{

FSteamCredentialInfo::FSteamCredentialInfo()
    : UserId(TEXT(""))
    , EncryptedAppTicket(TEXT(""))
    , SessionTicket(TEXT(""))
{
}

}

#endif