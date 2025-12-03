// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Interfaces/IHttpRequest.h"
#include "OnlineSubsystemRedpointSteam/Operation/SteamOperation.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSInterfaces/Interfaces/OnlineAvatarInterface.h"

#if defined(REDPOINT_EOS_STEAM_ENABLED)

namespace REDPOINT_EOS_FILE_NS_ID(4265612550, Redpoint::EOS::OnlineSubsystemRedpointSteam::OnlineSubsystem)
{
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam::Operation;

class FOnlineAvatarInterfaceRedpointSteam
    : public IOnlineAvatar,
      public TSharedFromThis<FOnlineAvatarInterfaceRedpointSteam, ESPMode::ThreadSafe>
{
private:
    bool bWarnedAboutEmptyApiKey;
    TSharedRef<FSteamOperation> SteamOperation;

    void OnProcessAvatarUrlRequestComplete(
        FHttpRequestPtr Request,
        FHttpResponsePtr Response,
        bool bConnectedSuccessfully,
        FString DefaultAvatarUrl,
        FOnGetAvatarUrlComplete OnComplete);

    void OnAvatarFetched(
        const FOnlineError &Error,
        uint32 Width,
        uint32 Height,
        const TArray<uint8> &RGBABuffer,
        TSoftObjectPtr<UTexture> DefaultTexture,
        FOnGetAvatarComplete OnComplete);

public:
    FOnlineAvatarInterfaceRedpointSteam(TSharedRef<FSteamOperation> InSteamOperation);
    UE_NONCOPYABLE(FOnlineAvatarInterfaceRedpointSteam);
    virtual ~FOnlineAvatarInterfaceRedpointSteam() = default;

    bool GetAvatar(
        const FUniqueNetId &LocalUserId,
        const FUniqueNetId &TargetUserId,
        TSoftObjectPtr<UTexture> DefaultTexture,
        FOnGetAvatarComplete OnComplete) override;

    bool GetAvatarUrl(
        const FUniqueNetId &LocalUserId,
        const FUniqueNetId &TargetUserId,
        FString DefaultAvatarUrl,
        FOnGetAvatarUrlComplete OnComplete) override;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointSteam::OnlineSubsystem
{
REDPOINT_EOS_FILE_NS_EXPORT(
    4265612550,
    Redpoint::EOS::OnlineSubsystemRedpointSteam::OnlineSubsystem,
    FOnlineAvatarInterfaceRedpointSteam)
}

#endif