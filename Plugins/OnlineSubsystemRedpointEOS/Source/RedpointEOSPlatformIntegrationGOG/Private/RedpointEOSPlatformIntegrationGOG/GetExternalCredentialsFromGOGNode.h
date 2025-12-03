// Copyright June Rhodes. All Rights Reserved.

#pragma once

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && EOS_GOG_ENABLED

#include "RedpointEOSAuth/AuthenticationGraphNode.h"

namespace Redpoint::EOS::Platform::Integration::GOG
{
using namespace ::Redpoint::EOS::Auth;

class FGetExternalCredentialsFromGOGNode : public FAuthenticationGraphNode
{
private:
    void OnEncryptedAppTicketResult(
        bool bWasSuccessful,
        FString EncodedAppTicket,
        TSharedRef<FAuthenticationGraphState> InState,
        FAuthenticationGraphNodeOnDone InOnDone);

public:
    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

    virtual FString GetDebugName() const override
    {
        return TEXT("FGetExternalCredentialsFromGOGNode");
    }
};

}

#endif // #if EOS_HAS_AUTHENTICATION && EOS_GOG_ENABLED