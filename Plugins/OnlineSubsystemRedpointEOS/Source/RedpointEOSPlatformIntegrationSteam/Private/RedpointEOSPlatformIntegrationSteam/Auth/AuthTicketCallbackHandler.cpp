// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationSteam/Auth/AuthTicketCallbackHandler.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && defined(REDPOINT_EOS_STEAM_ENABLED)

#include "Misc/Base64.h"
#include "RedpointEOSPlatformIntegrationSteam/Logging.h"

namespace REDPOINT_EOS_FILE_NS_ID(724733551, Redpoint::EOS::Platform::Integration::Steam::Auth)
{
using namespace ::Redpoint::EOS::Platform::Integration::Steam;

#if REDPOINT_EOS_UE_5_4_OR_LATER
void FAuthTicketCallbackHandler::OnGetAuthSessionTicketResponse(GetTicketForWebApiResponse_t *Callback)
{
    if (Callback->m_eResult == EResult::k_EResultOK)
    {
        FString NewSessionTicket;
        TArray<uint8> Data(Callback->m_rgubTicket, Callback->m_cubTicket);
        EOS_EResult Result = Obtainer->ConvertToHexString(Data, NewSessionTicket);
        if (Result == EOS_EResult::EOS_Success)
        {
            Obtainer->SessionTicket = NewSessionTicket;
            Obtainer->SessionTicketBase64 = FBase64::Encode(Data);
        }
    }
    else
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationSteam,
            Warning,
            TEXT("Unable to obtain Steam session ticket (got result code %d)"),
            Callback->m_eResult);
        Obtainer->SessionTicket = TEXT("");
        Obtainer->SessionTicketBase64 = TEXT("");
    }
    Obtainer->bAuthTicketSynced = true;
}
#else
void FAuthTicketCallbackHandler::OnGetAuthSessionTicketResponse(GetAuthSessionTicketResponse_t *Callback)
{
    // We don't use the auth ticket in the callback, we just need to know when it's synced to the server.
    // WARNING: This code runs on a different thread than the main authentication graph.
    Obtainer->bAuthTicketSynced = true;
}
#endif

FAuthTicketCallbackHandler::FAuthTicketCallbackHandler(FSteamCredentialObtainer *InObtainer)
    : Obtainer(InObtainer)
{
}

}

#endif