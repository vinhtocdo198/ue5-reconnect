// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationSteam/Auth/SteamCredentialObtainer.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && defined(REDPOINT_EOS_STEAM_ENABLED)

#include "Misc/Base64.h"
#include "OnlineSubsystemUtils.h"
#include "RedpointEOSAuth/AuthenticationConstants.h"
#include "RedpointEOSPlatformIntegrationSteam/Auth/AuthTicketCallbackHandler.h"
#include "RedpointEOSPlatformIntegrationSteam/Auth/SteamCredentialInfo.h"

namespace REDPOINT_EOS_FILE_NS_ID(4001727958, Redpoint::EOS::Platform::Integration::Steam::Auth)
{

FSteamCredentialObtainer::FSteamCredentialObtainer(const FSteamCredentialObtainer::FOnCredentialObtained &Cb)
    : FAuthenticationCredentialObtainer(Cb)
    , EncryptedAppTicketDelegateHandle()
    , SessionTicket(TEXT(""))
    , AppTicket(TEXT(""))
    , SessionTicketBase64(TEXT(""))
    , AppTicketBase64(TEXT(""))
    , bSuccess(false)
    , bComplete(false)
    , AuthTicketHandle()
    , CallbackHandler(nullptr)
    , bAuthTicketWaiting(false)
    , bAuthTicketSynced(false)
{
}

EOS_EResult FSteamCredentialObtainer::ConvertToHexString(const TArray<uint8> &Data, FString &OutHexString)
{
    uint8_t *Block = (uint8_t *)FMemory::Malloc(Data.Num());
    for (auto i = 0; i < Data.Num(); i++)
    {
        Block[i] = Data[i];
    }
    uint32_t TokenLen = 4096;
    char TokenBuffer[4096];
    auto Result = EOS_ByteArray_ToString(Block, Data.Num(), TokenBuffer, &TokenLen);
    FMemory::Free(Block);
    if (Result != EOS_EResult::EOS_Success)
    {
        OutHexString = TEXT("");
    }
    else
    {
        OutHexString = FString(ANSI_TO_TCHAR(TokenBuffer));
    }
    return Result;
}

void FSteamCredentialObtainer::OnEncryptedAppTicketObtained(bool bEncryptedDataAvailable, int32 ResultCode)
{
    auto OSSEncryptedAppTicket = this->OSSEncryptedAppTicketWk.Pin();
    auto OSSIdentity = this->OSSIdentityWk.Pin();
    if (!OSSEncryptedAppTicket.IsValid() || !OSSIdentity.IsValid())
    {
        this->EmitError(TEXT("Steam OSS is no longer available to handle encrypted app ticket response with."));
        this->bSuccess = false;
        this->bComplete = true;
        return;
    }

    OSSEncryptedAppTicket->OnEncryptedAppTicketResultDelegate.Remove(this->EncryptedAppTicketDelegateHandle);

    // When using session tickets, it's optional for the encrypted app ticket to succeed.
    this->AppTicket = TEXT("");
    this->AppTicketBase64 = TEXT("");
    if (bEncryptedDataAvailable)
    {
        TArray<uint8> Data;
        auto Success = OSSEncryptedAppTicket->GetEncryptedAppTicket(Data);
        if (Success)
        {
            FString NewAppTicket;
            EOS_EResult Result = ConvertToHexString(Data, NewAppTicket);
            if (Result == EOS_EResult::EOS_Success)
            {
                this->AppTicket = NewAppTicket;
                this->AppTicketBase64 = FBase64::Encode(Data);
            }
        }
    }

    this->UserId = OSSIdentity->GetUniquePlayerId(0)->ToString();

    // Get the SteamUser interface directly.
    auto SteamUserPtr = SteamUser();
    if (SteamUserPtr == nullptr)
    {
        this->EmitError(TEXT("SteamUser() interface not available to get session ticket"));
        this->bSuccess = false;
        this->bComplete = true;
        return;
    }

    // Now start obtaining the session ticket.
#if REDPOINT_EOS_UE_5_4_OR_LATER
    this->bAuthTicketWaiting = true;
    this->bAuthTicketSynced = false;
    this->CallbackHandler = MakeShareable(new FAuthTicketCallbackHandler(this));
    this->AuthTicketHandle = SteamUserPtr->GetAuthTicketForWebApi("epiconlineservices");
    if (this->AuthTicketHandle == k_HAuthTicketInvalid)
    {
        this->bAuthTicketWaiting = false;
        this->bAuthTicketSynced = false;
        this->CallbackHandler.Reset();

        this->EmitError(TEXT("Unable to start obtainment of session ticket from SteamUser() interface"));
        this->bSuccess = false;
        this->bComplete = true;
    }
    else
    {
        // We're already in waiting mode due to setting bAuthTicketWaiting above.
    }
#else
    const int SteamAuthMaxTicketLengthInBytes = 1024;
    uint8 AuthTicket[SteamAuthMaxTicketLengthInBytes];
    uint32 AuthTicketSize = 0;
    this->bAuthTicketWaiting = true;
    this->bAuthTicketSynced = false;
    this->CallbackHandler = MakeShareable(new FAuthTicketCallbackHandler(this));
    this->AuthTicketHandle =
        SteamUserPtr->GetAuthSessionTicket(AuthTicket, UE_ARRAY_COUNT(AuthTicket), &AuthTicketSize);
    if (this->AuthTicketHandle != k_HAuthTicketInvalid && AuthTicketSize > 0)
    {
        FString NewSessionTicket;
        EOS_EResult SessionTicketResult =
            ConvertToHexString(TArray<uint8>(AuthTicket, AuthTicketSize), NewSessionTicket);
        if (SessionTicketResult != EOS_EResult::EOS_Success)
        {
            this->EmitError(TEXT("EOS_ByteArray_ToString call failed for session ticket"));
            this->bSuccess = false;
            this->bComplete = true;
            return;
        }

        this->SessionTicket = NewSessionTicket;
        this->SessionTicketBase64 = FBase64::Encode(AuthTicket, AuthTicketSize);

        // Now we wait for the callback to fire.
    }
    else
    {
        this->bAuthTicketWaiting = false;
        this->bAuthTicketSynced = false;
        this->CallbackHandler.Reset();

        this->EmitError(TEXT("Unable to start obtainment of session ticket from SteamUser() interface"));
        this->bSuccess = false;
        this->bComplete = true;
    }
#endif
}

bool FSteamCredentialObtainer::Init(UWorld *World, int32 LocalUserNum)
{
    if (World == nullptr)
    {
        this->EmitError(TEXT("Could not authenticate with Steam, because the UWorld* pointer was null."));
        this->Done(false, FSteamCredentialInfo());
        return false;
    }

    FOnlineSubsystemSteam *OSSSubsystem = (FOnlineSubsystemSteam *)Online::GetSubsystem(World, STEAM_SUBSYSTEM);
    if (OSSSubsystem == nullptr)
    {
        this->EmitError(TEXT("Could not authenticate with online subsystem, because the subsystem was not "
                             "available. Check that Steam OSS is enabled in your DefaultEngine.ini file."));
        this->Done(false, FSteamCredentialInfo());
        return false;
    }

    FOnlineEncryptedAppTicketSteamPtr OSSEncryptedAppTicket = OSSSubsystem->GetEncryptedAppTicketInterface();
    if (OSSEncryptedAppTicket == nullptr)
    {
        this->EmitError(
            TEXT("Could not authenticate with Steam, because the encrypted app ticket interface was not available."));
        this->Done(false, FSteamCredentialInfo());
        return false;
    }

    TSharedPtr<IOnlineIdentity, ESPMode::ThreadSafe> OSSIdentity =
        StaticCastSharedPtr<IOnlineIdentity>(OSSSubsystem->GetIdentityInterface());
    if (!OSSIdentity.IsValid())
    {
        this->EmitError(TEXT("Could not authenticate with Steam, because the identity interface was not available."));
        this->Done(false, FSteamCredentialInfo());
        return false;
    }

    this->OSSEncryptedAppTicketWk = OSSEncryptedAppTicket;
    this->OSSIdentityWk = OSSIdentity;

    this->EncryptedAppTicketDelegateHandle = OSSEncryptedAppTicket->OnEncryptedAppTicketResultDelegate.AddSP(
        this,
        &FSteamCredentialObtainer::OnEncryptedAppTicketObtained);
    OSSEncryptedAppTicket->RequestEncryptedAppTicket(nullptr, 0);
    return true;
}

bool FSteamCredentialObtainer::Tick(float DeltaSeconds)
{
    if (!this->bComplete)
    {
        if (this->bAuthTicketWaiting && this->bAuthTicketSynced)
        {
            // Auth ticket has been synced.
            this->bAuthTicketWaiting = false;

            // This is the last thing to happen.
            this->bSuccess = true;
            this->bComplete = true;

            // Fallthrough to success handler below.
        }
        else
        {
            return true;
        }
    }

    if (this->bSuccess)
    {
        TMap<FString, FString> UserAuthAttributes;
        UserAuthAttributes.Add(EOS_AUTH_ATTRIBUTE_AUTHENTICATEDWITH, TEXT("steam"));
        UserAuthAttributes.Add(TEXT("steam.id"), this->UserId);
        UserAuthAttributes.Add(TEXT("steam.encryptedAppTicket"), this->AppTicket);
        UserAuthAttributes.Add(TEXT("steam.encryptedAppTicket.base64"), this->AppTicketBase64);
        UserAuthAttributes.Add(TEXT("steam.sessionTicket"), this->SessionTicket);
        UserAuthAttributes.Add(TEXT("steam.sessionTicket.base64"), this->SessionTicketBase64);

        FSteamCredentialInfo Info;
        Info.UserId = this->UserId;
        Info.EncryptedAppTicket = this->AppTicket;
        Info.SessionTicket = this->SessionTicket;
        Info.AuthAttributes = UserAuthAttributes;
        this->Done(true, Info);
    }
    else
    {
        this->Done(false, FSteamCredentialInfo());
    }
    return false;
}

}

#endif