// Copyright June Rhodes. All Rights Reserved.

#pragma once

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && defined(REDPOINT_EOS_STEAM_ENABLED)

#include "RedpointEOSPlatformIntegrationSteam/Auth/SteamCredentialObtainer.h"
#include "RedpointEOSPlatformIntegrationSteam/ForwardDecls.h"
#include "RedpointEOSPlatformIntegrationSteam/SDK.h"

namespace REDPOINT_EOS_FILE_NS_ID(724733551, Redpoint::EOS::Platform::Integration::Steam::Auth)
{
using namespace ::Redpoint::EOS::Platform::Integration::Steam::Auth;

class FAuthTicketCallbackHandler
{
private:
    FSteamCredentialObtainer *Obtainer;
#if REDPOINT_EOS_UE_5_4_OR_LATER
    STEAM_CALLBACK(FAuthTicketCallbackHandler, OnGetAuthSessionTicketResponse, GetTicketForWebApiResponse_t);
#else
    STEAM_CALLBACK(FAuthTicketCallbackHandler, OnGetAuthSessionTicketResponse, GetAuthSessionTicketResponse_t);
#endif

public:
    FAuthTicketCallbackHandler(FSteamCredentialObtainer *InObtainer);
};

}

namespace Redpoint::EOS::Platform::Integration::Steam::Auth
{
REDPOINT_EOS_FILE_NS_EXPORT(724733551, Redpoint::EOS::Platform::Integration::Steam::Auth, FAuthTicketCallbackHandler)
}

#endif