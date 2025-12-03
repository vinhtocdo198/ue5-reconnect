// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointSteam/OnlineSubsystem/OnlineAvatarInterfaceRedpointSteam.h"

#include "Containers/Ticker.h"
#include "Dom/JsonObject.h"
#include "Engine/Texture2D.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Misc/ConfigCacheIni.h"
#include "OnlineSubsystemRedpointSteam/Logging.h"
#include "OnlineSubsystemRedpointSteam/OnlineSubsystem/OnlineSubsystemRedpointSteam.h"
#include "OnlineSubsystemRedpointSteam/SDK.h"
#include "RedpointEOSCore/Legacy/WeakPtrHelpers.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "TextureResource.h"
#include <limits>

#if defined(REDPOINT_EOS_STEAM_ENABLED)

namespace REDPOINT_EOS_FILE_NS_ID(4265612550, Redpoint::EOS::OnlineSubsystemRedpointSteam::OnlineSubsystem)
{
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam::OnlineSubsystem;

void FOnlineAvatarInterfaceRedpointSteam::OnProcessAvatarUrlRequestComplete(
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    FHttpRequestPtr Request,
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    FHttpResponsePtr Response,
    bool bConnectedSuccessfully,
    FString DefaultAvatarUrl,
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    FOnGetAvatarUrlComplete OnComplete)
{
    if (!Response.IsValid())
    {
        OnComplete.ExecuteIfBound(false, MoveTemp(DefaultAvatarUrl));
        return;
    }

    const FString Content = Response->GetContentAsString();

    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
    TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Content);

    if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
    {
        TSharedPtr<FJsonObject> JsonResponse = JsonObject->GetObjectField(TEXT("response"));
        if (JsonResponse.IsValid())
        {
            TArray<TSharedPtr<FJsonValue>> JsonPlayers = JsonResponse->GetArrayField(TEXT("players"));
            if (JsonPlayers.Num() > 0)
            {
                TSharedPtr<FJsonObject> JsonPlayer = JsonPlayers[0]->AsObject();
                OnComplete.ExecuteIfBound(true, JsonPlayer->GetStringField(TEXT("avatarfull")));
                return;
            }
        }
    }

    OnComplete.ExecuteIfBound(false, MoveTemp(DefaultAvatarUrl));
}

void FOnlineAvatarInterfaceRedpointSteam::OnAvatarFetched(
    const FOnlineError &Error,
    uint32 Width,
    uint32 Height,
    const TArray<uint8> &RGBABuffer,
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    TSoftObjectPtr<UTexture> DefaultTexture,
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    FOnGetAvatarComplete OnComplete)
{
    if (!Error.bSucceeded)
    {
        OnComplete.ExecuteIfBound(false, MoveTemp(DefaultTexture));
        return;
    }

    UTexture2D *Avatar = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
    uint8 *MipData = (uint8 *)Avatar->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
    FMemory::Memcpy(MipData, (void *)RGBABuffer.GetData(), RGBABuffer.Num());
    Avatar->GetPlatformData()->Mips[0].BulkData.Unlock();
    Avatar->GetPlatformData()->SetNumSlices(1);
    Avatar->NeverStream = true;
    Avatar->UpdateResource();

    OnComplete.ExecuteIfBound(true, Avatar);
}

FOnlineAvatarInterfaceRedpointSteam::FOnlineAvatarInterfaceRedpointSteam(TSharedRef<FSteamOperation> InSteamOperation)
    : bWarnedAboutEmptyApiKey(false)
    , SteamOperation(InSteamOperation)
{
}

bool FOnlineAvatarInterfaceRedpointSteam::GetAvatar(
    const FUniqueNetId &LocalUserId,
    const FUniqueNetId &TargetUserId,
    TSoftObjectPtr<UTexture> DefaultTexture,
    FOnGetAvatarComplete OnComplete)
{
    if (TargetUserId.GetType() != STEAM_SUBSYSTEM)
    {
        UE_LOG(LogRedpointSteam, Error, TEXT("GetAvatar: TargetUserId is non-Steam user."));
        OnComplete.ExecuteIfBound(false, DefaultTexture);
        return true;
    }
    if (!LocalUserId.DoesSharedInstanceExist())
    {
        UE_LOG(LogRedpointSteam, Error, TEXT("GetAvatar: LocalUserId is not a shareable FUniqueNetId."));
        OnComplete.ExecuteIfBound(false, DefaultTexture);
        return true;
    }
    if (!TargetUserId.DoesSharedInstanceExist())
    {
        UE_LOG(LogRedpointSteam, Error, TEXT("GetAvatar: TargetUserId is not a shareable FUniqueNetId."));
        OnComplete.ExecuteIfBound(false, DefaultTexture);
        return true;
    }

    // Cheat. We can't access FUniqueNetIdSteam directly, but we do know it returns
    // the CSteamID as a uint64 from GetBytes :)
    uint64 SteamID = *(uint64 *)TargetUserId.GetBytes();

    this->SteamOperation->FetchAvatar(
        SteamID,
        FSteamOperation::FOnFetchAvatarComplete::CreateSP(
            this,
            &FOnlineAvatarInterfaceRedpointSteam::OnAvatarFetched,
            DefaultTexture,
            OnComplete));
    return true;
}

bool FOnlineAvatarInterfaceRedpointSteam::GetAvatarUrl(
    const FUniqueNetId &LocalUserId,
    const FUniqueNetId &TargetUserId,
    FString DefaultAvatarUrl,
    FOnGetAvatarUrlComplete OnComplete)
{
    if (TargetUserId.GetType() != STEAM_SUBSYSTEM)
    {
        UE_LOG(LogRedpointSteam, Error, TEXT("GetAvatarUrl: TargetUserId is non-Steam user."));
        OnComplete.ExecuteIfBound(false, DefaultAvatarUrl);
        return true;
    }
    if (!LocalUserId.DoesSharedInstanceExist())
    {
        UE_LOG(LogRedpointSteam, Error, TEXT("GetAvatarUrl: LocalUserId is not a shareable FUniqueNetId."));
        OnComplete.ExecuteIfBound(false, DefaultAvatarUrl);
        return true;
    }
    if (!TargetUserId.DoesSharedInstanceExist())
    {
        UE_LOG(LogRedpointSteam, Error, TEXT("GetAvatarUrl: TargetUserId is not a shareable FUniqueNetId."));
        OnComplete.ExecuteIfBound(false, DefaultAvatarUrl);
        return true;
    }

    // Cheat. We can't access FUniqueNetIdSteam directly, but we do know it returns
    // the CSteamID as a uint64 from GetBytes :)
    uint64 SteamID = *(uint64 *)TargetUserId.GetBytes();

    FOnlineSubsystemRedpointSteam *OnlineSubsystemSteam =
        // NOLINTNEXTLINE(unreal-ionlinesubsystem-get)
        static_cast<FOnlineSubsystemRedpointSteam *>(IOnlineSubsystem::Get(SubsystemName));
    if (OnlineSubsystemSteam)
    {
        const FString WebApiKey = OnlineSubsystemSteam->GetWebApiKey();
        if (!WebApiKey.IsEmpty())
        {
            const FString PlayerUrl = FString::Printf(
                TEXT("https://api.steampowered.com/ISteamUser/GetPlayerSummaries/v0002/?key=%s&steamids=%llu"),
                *WebApiKey,
                SteamID);

            auto Request = FHttpModule::Get().CreateRequest();
            Request->SetVerb("GET");
            Request->SetURL(PlayerUrl);
            Request->OnProcessRequestComplete().BindThreadSafeSP(
                AsShared(),
                &FOnlineAvatarInterfaceRedpointSteam::OnProcessAvatarUrlRequestComplete,
                DefaultAvatarUrl,
                OnComplete);

            Request->ProcessRequest();
        }
        else
        {
            if (!this->bWarnedAboutEmptyApiKey)
            {
                UE_LOG(
                    LogRedpointSteam,
                    Error,
                    TEXT("GetAvatarUrl: Web API Key is empty (this error will only be reported once)."));
                this->bWarnedAboutEmptyApiKey = true;
            }
            OnComplete.ExecuteIfBound(false, DefaultAvatarUrl);
        }
    }
    else
    {
        UE_LOG(LogRedpointSteam, Error, TEXT("GetAvatarUrl: FOnlineSubsystemRedpointSteam not valid."));
        OnComplete.ExecuteIfBound(false, DefaultAvatarUrl);
    }

    return true;
}

}

#endif