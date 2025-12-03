// Copyright June Rhodes. All Rights Reserved.

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && EOS_GOG_ENABLED

#include "RedpointEOSPlatformIntegrationGOG/GetExternalCredentialsFromGOGNode.h"
#include "RedpointEOSAuth/AuthenticationConstants.h"
#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "RedpointEOSPlatformIntegrationGOG/GOGEncryptedAppTicketTask.h"
#include "RedpointEOSPlatformIntegrationGOG/GOGExternalCredentials.h"
#include "RedpointEOSPlatformIntegrationGOG/Logging.h"
#include "RedpointEOSPlatformIntegrationGOG/SDK.h"

namespace Redpoint::EOS::Platform::Integration::GOG
{

void FGetExternalCredentialsFromGOGNode::OnEncryptedAppTicketResult(
    bool bWasSuccessful,
    FString AppTicket,
    TSharedRef<FAuthenticationGraphState> InState,
    FAuthenticationGraphNodeOnDone InOnDone)
{
    if (!bWasSuccessful)
    {
        UE_LOG(LogRedpointEOSPlatformIntegrationGOG, Error, TEXT("Unable to get encrypted app ticket from GOG"));
        InOnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }

    auto UserId = galaxy::api::User()->GetGalaxyID();

    TMap<FString, FString> UserAuthAttributes;
    UserAuthAttributes.Add(EOS_AUTH_ATTRIBUTE_AUTHENTICATEDWITH, TEXT("gog"));
    UserAuthAttributes.Add(TEXT("gog.id"), FString::Printf(TEXT("%llu"), UserId.ToUint64()));
    UserAuthAttributes.Add(TEXT("gog.encryptedAppTicket"), AppTicket);

    UE_LOG(LogRedpointEOSPlatformIntegrationGOG, Verbose, TEXT("Authenticating with GOG app ticket: %s"), *AppTicket);

    InState->AvailableExternalCredentials.Add(MakeShared<FGOGExternalCredentials>(AppTicket, UserAuthAttributes));
    InOnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
    return;
}

void FGetExternalCredentialsFromGOGNode::Execute(
    TSharedRef<FAuthenticationGraphState> InState,
    FAuthenticationGraphNodeOnDone InOnDone)
{
    TSharedPtr<FGOGEncryptedAppTicketTask> Task =
        MakeShared<FGOGEncryptedAppTicketTask>(FOnGOGAppTicketResult::CreateSP(
            this,
            &FGetExternalCredentialsFromGOGNode::OnEncryptedAppTicketResult,
            InState,
            InOnDone));
    Task->Start();
}

}

#endif