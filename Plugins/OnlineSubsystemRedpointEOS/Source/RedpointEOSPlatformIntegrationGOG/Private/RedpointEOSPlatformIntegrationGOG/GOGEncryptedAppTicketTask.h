// Copyright June Rhodes. All Rights Reserved.

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && EOS_GOG_ENABLED

#include "RedpointEOSPlatformIntegrationGOG/Logging.h"
#include "RedpointEOSPlatformIntegrationGOG/SDK.h"

namespace Redpoint::EOS::Platform::Integration::GOG
{

typedef TDelegate<void(bool bWasSuccessful, FString AppTicket)> FOnGOGAppTicketResult;

class FGOGEncryptedAppTicketTask : public galaxy::api::IEncryptedAppTicketListener,
                                   public TSharedFromThis<FGOGEncryptedAppTicketTask>
{
private:
    TSharedPtr<FGOGEncryptedAppTicketTask> SelfReference;

    FOnGOGAppTicketResult Callback;

public:
    FGOGEncryptedAppTicketTask(const FOnGOGAppTicketResult &InCallback);

    virtual void Start();

    virtual void OnEncryptedAppTicketRetrieveSuccess() override;
    virtual void OnEncryptedAppTicketRetrieveFailure(FailureReason failureReason) override;
};

}

#endif