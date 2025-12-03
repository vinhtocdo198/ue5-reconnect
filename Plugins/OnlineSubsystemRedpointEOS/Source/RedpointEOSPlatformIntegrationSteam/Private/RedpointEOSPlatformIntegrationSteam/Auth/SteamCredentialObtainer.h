// Copyright June Rhodes. All Rights Reserved.

#pragma once

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && defined(REDPOINT_EOS_STEAM_ENABLED)

#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineEncryptedAppTicketInterfaceSteam.h"
#include "RedpointEOSAuth/AuthenticationCredentialObtainer.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSPlatformIntegrationSteam/Auth/AuthTicketCallbackHandler.h"
#include "RedpointEOSPlatformIntegrationSteam/Auth/SteamCredentialInfo.h"
#include "RedpointEOSPlatformIntegrationSteam/ForwardDecls.h"
#include "RedpointEOSPlatformIntegrationSteam/SDK.h"

namespace REDPOINT_EOS_FILE_NS_ID(4001727958, Redpoint::EOS::Platform::Integration::Steam::Auth)
{
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::Platform::Integration::Steam::Auth;

class FSteamCredentialObtainer
    : public FAuthenticationCredentialObtainer<FSteamCredentialObtainer, FSteamCredentialInfo>
{
    friend FAuthTicketCallbackHandler;

private:
    FDelegateHandle EncryptedAppTicketDelegateHandle;
    FString UserId;
    FString SessionTicket;
    FString AppTicket;
    FString SessionTicketBase64;
    FString AppTicketBase64;
    bool bSuccess;
    bool bComplete;
    TWeakPtr<FOnlineEncryptedAppTicketSteam, ESPMode::ThreadSafe> OSSEncryptedAppTicketWk;
    TWeakPtr<IOnlineIdentity, ESPMode::ThreadSafe> OSSIdentityWk;
    HAuthTicket AuthTicketHandle;
    TSharedPtr<FAuthTicketCallbackHandler, ESPMode::ThreadSafe> CallbackHandler;
    bool bAuthTicketWaiting;

public:
    bool bAuthTicketSynced;

private:
    EOS_EResult ConvertToHexString(const TArray<uint8> &Data, FString &OutAppTicket);
    void OnEncryptedAppTicketObtained(bool bEncryptedDataAvailable, int32 ResultCode);

public:
    FSteamCredentialObtainer(const FSteamCredentialObtainer::FOnCredentialObtained &Cb);
    UE_NONCOPYABLE(FSteamCredentialObtainer);

    virtual bool Init(UWorld *World, int32 LocalUserNum) override;
    virtual bool Tick(float DeltaSeconds) override;
};

}

namespace Redpoint::EOS::Platform::Integration::Steam::Auth
{
REDPOINT_EOS_FILE_NS_EXPORT(4001727958, Redpoint::EOS::Platform::Integration::Steam::Auth, FSteamCredentialObtainer)
}

#endif