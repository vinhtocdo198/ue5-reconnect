// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationGOG/GOGEncryptedAppTicketTask.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && EOS_GOG_ENABLED

#include "RedpointEOSAPI/SDK.h"

namespace Redpoint::EOS::Platform::Integration::GOG
{

// There is no documentation on how long this ticket is.
#define ENCRYPTED_APP_TICKET_LENGTH 2048

FGOGEncryptedAppTicketTask::FGOGEncryptedAppTicketTask(const FOnGOGAppTicketResult &InCallback)
{
    this->Callback = InCallback;
}

void FGOGEncryptedAppTicketTask::Start()
{
    checkf(!this->SelfReference.IsValid(), TEXT("GOG encrypted app ticket task already started!"));

    galaxy::api::User()->RequestEncryptedAppTicket(nullptr, 0, this);
    this->SelfReference = this->AsShared(); // Keep ourselves alive until the async task completes.
}

void FGOGEncryptedAppTicketTask::OnEncryptedAppTicketRetrieveSuccess()
{
    void *EncryptedAppTicketBuffer = FMemory::Malloc(ENCRYPTED_APP_TICKET_LENGTH);
    uint32 EncryptedAppTicketSize = 0;

    galaxy::api::User()->GetEncryptedAppTicket(
        EncryptedAppTicketBuffer,
        ENCRYPTED_APP_TICKET_LENGTH,
        EncryptedAppTicketSize);

    uint32_t TokenLen = 4096;
    char TokenBuffer[4096];
    auto Result = EOS_ByteArray_ToString(
        (const uint8_t *)EncryptedAppTicketBuffer,
        EncryptedAppTicketSize,
        TokenBuffer,
        &TokenLen);
    FMemory::Free(EncryptedAppTicketBuffer);
    if (Result != EOS_EResult::EOS_Success)
    {
        this->Callback.ExecuteIfBound(false, TEXT(""));
    }
    else
    {
        this->Callback.ExecuteIfBound(true, ANSI_TO_TCHAR(TokenBuffer));
    }
    this->SelfReference = nullptr;
}

void FGOGEncryptedAppTicketTask::OnEncryptedAppTicketRetrieveFailure(
    galaxy::api::IEncryptedAppTicketListener::FailureReason failureReason)
{
    this->Callback.ExecuteIfBound(false, TEXT(""));
    this->SelfReference = nullptr;
}

}

#endif