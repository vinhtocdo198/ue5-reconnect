// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Avatar/OnlineAvatarInterfaceSynthetic.h"

#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "OnlineSubsystemRedpointEOS/Logging.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/SubsystemNames.h"
#include "OnlineSubsystemRedpointEOS/Texture/TextureLoader.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSCore/Module.h"
#include "RedpointEOSCore/Utils/CallContextFactory.h"
#include "RedpointEOSCore/Utils/MultiOperation.h"
#include "RedpointEOSPlatform/RuntimePlatformIntegration.h"
#include "RedpointEOSPlatform/RuntimePlatformRegistry.h"
#include "RedpointEOSUserCache/UserCacheSystem.h"
#include "RedpointEOSUserCache/UserInfo.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1278425628, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Avatar)
{
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::Platform;

FOnlineAvatarInterfaceSynthetic::FOnlineAvatarInterfaceSynthetic(
    const FPlatformHandle &InPlatformHandle,
    const TSharedRef<IOnlineIdentityInterfaceEOS> &InIdentity,
    const TSharedRef<IOnlineFriends> &InFriends)
    : PlatformHandle(InPlatformHandle)
    , Identity(InIdentity)
    , Friends(InFriends)
{
}

FString FOnlineAvatarInterfaceSynthetic::GetCachedAvatarUrl(
    const FUniqueNetId &LocalUserId,
    const FUniqueNetId &TargetUserId)
{
    int32 LocalUserNum;
    if (!this->Identity->GetLocalUserNum(LocalUserId, LocalUserNum))
    {
        return TEXT("");
    }

    auto Friend = this->Friends->GetFriend(LocalUserNum, TargetUserId, TEXT(""));
    if (!Friend.IsValid())
    {
        return TEXT("");
    }

    FString AvatarUrl;
    if (Friend->GetUserAttribute(TEXT("avatarUrl"), AvatarUrl))
    {
        // This friend has a cached avatar URL, use it.
        return AvatarUrl;
    }

    return TEXT("");
}

void FOnlineAvatarInterfaceSynthetic::ResolveTargetUserIdToExternalUserInfoOrFail(
    const FUniqueNetId &LocalUserId,
    const FUniqueNetId &TargetUserId,
    const FOnResolveFailure &OnFailure,
    const FOnResolveSuccess &OnResolved)
{
    using namespace ::Redpoint::EOS::Core::Id;
    using namespace ::Redpoint::EOS::Core::Utils;
    using namespace ::Redpoint::EOS::Platform::Services;
    using namespace ::Redpoint::EOS::UserCache;
    using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS;

    int32 LocalUserNum;
    if (!this->Identity->GetLocalUserNum(LocalUserId, LocalUserNum))
    {
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Error,
            TEXT("Failed to resolve avatar for user account '%s': The local user is not signed in."),
            *TargetUserId.ToString());
        OnFailure.ExecuteIfBound();
        return;
    }

    auto LocalAccountId = GetAccountId(LocalUserId);
    auto CallContext = ConstructRuntimePlatformServiceCallContext(this->PlatformHandle, LocalUserNum);

    if (TargetUserId.GetType().IsEqual(REDPOINT_EOS_SUBSYSTEM))
    {
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            VeryVerbose,
            TEXT("%s: Resolving target EOS user ID '%s'..."),
            ANSI_TO_TCHAR(__FUNCTION__),
            *TargetUserId.ToString());

        auto TargetAccountId = GetAccountId(TargetUserId);
        this->PlatformHandle->GetSystem<IUserCacheSystem>()->GetUser(
            LocalAccountId,
            TargetAccountId,
            IUserCacheSystem::FOnGetUserComplete::CreateSPLambda(
                this,
                [
#if REDPOINT_EOS_INCLUDE_UE_LOG_ONLY_CAPTURES
                    TargetAccountId,
#endif
                    CallContext,
                    OnFailure,
                    OnResolved](FError ErrorCode, FUserInfoPtr UserInfo) {
                    if (!ErrorCode.WasSuccessful())
                    {
                        UE_LOG(
                            LogOnlineSubsystemRedpointEOS,
                            Error,
                            TEXT("Failed to resolve avatar for user account '%s': %s"),
                            *GetUserIdDebugString(TargetAccountId),
                            *ErrorCode.ToLogString());
                        OnFailure.ExecuteIfBound();
                    }
                    else
                    {
                        OnResolved.ExecuteIfBound(CallContext, UserInfo.ToSharedRef());
                    }
                }));
    }
    else
    {
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            VeryVerbose,
            TEXT("%s: Resolving target external user '%s'..."),
            ANSI_TO_TCHAR(__FUNCTION__),
            *TargetUserId.ToString());

        FMultiOperation<TSharedRef<IRuntimePlatformIntegration>, FExternalUserInfoPtr>::RunSP(
            this->AsShared(),
            Redpoint::EOS::Core::FModule::GetModuleChecked()
                .GetRuntimePlatformRegistry()
                ->GetRuntimePlatformIntegrations(),
            // @note: Passing the TargetUserId by reference here is a little sketchy, but we know the implementation of
            // RunSP will call the Initiate function for all of the runtime integrations before RunSP returns, thus
            // ensuring that TargetUserId is still valid at the time that GetExternalUserByUniqueNetId is initially
            // called. We can't do 'TargetUserId.AsShared()' as some online subsystems incorrectly allocate their
            // FUniqueNetIds on the stack.
            [&TargetUserId, TargetUserIdString = TargetUserId.ToString(), CallContext](
                TSharedRef<IRuntimePlatformIntegration> Integration,
                TFunction<void(FExternalUserInfoPtr OutValue)> OnDone) -> bool {
                Integration->GetUserService()->GetExternalUserByUniqueNetId(
                    CallContext,
                    TargetUserId,
                    IRuntimePlatformUserService::FOnGetExternalUserByUniqueNetIdComplete::CreateLambda(
                        [OnDone, TargetUserIdString](const FError &Error, const FExternalUserInfoPtr &UserInfo) {
                            if (!Error.WasSuccessful() && Error.ResultCode() == EOS_EResult::EOS_NotImplemented)
                            {
                                UE_LOG(
                                    LogOnlineSubsystemRedpointEOS,
                                    VeryVerbose,
                                    TEXT("%s: Failed to resolve external user info for '%s': %s"),
                                    ANSI_TO_TCHAR(__FUNCTION__),
                                    *TargetUserIdString,
                                    *Error.ToLogString());
                            }
                            OnDone(UserInfo);
                        }));
                return true;
            },
            [TargetUserIdString = TargetUserId.ToString(), CallContext, OnFailure, OnResolved](
                TArray<FExternalUserInfoPtr> Values) {
                // Use the first valid FExternalUserInfo for the lookup.
                for (const auto &Value : Values)
                {
                    if (Value.IsValid())
                    {
                        OnResolved.ExecuteIfBound(CallContext, Value.ToSharedRef());
                        return;
                    }
                }

                // If none was found, we can't get the avatar for this external user.
                UE_LOG(
                    LogOnlineSubsystemRedpointEOS,
                    Error,
                    TEXT("Failed to resolve avatar for user account '%s': There is no platform support for retrieving "
                         "avatars."),
                    *TargetUserIdString);
                OnFailure.ExecuteIfBound();
            });
    }
}

void FOnlineAvatarInterfaceSynthetic::OnUserInfoResolvedForGetAvatarUrl(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FExternalUserInfoRef &UserInfo,
    FString DefaultAvatarUrl,
    ::FOnGetAvatarUrlComplete OnComplete)
{
    FMultiOperation<TSharedRef<IRuntimePlatformIntegration>, TOptional<FString>>::RunSP(
        this->AsShared(),
        Redpoint::EOS::Core::FModule::GetModuleChecked().GetRuntimePlatformRegistry()->GetRuntimePlatformIntegrations(),
        [CallContext, UserInfo](
            TSharedRef<IRuntimePlatformIntegration> Integration,
            TFunction<void(TOptional<FString> OutValue)> OnDone) -> bool {
            Integration->GetAvatarService()->GetAvatarUrl(
                CallContext,
                UserInfo,
                IRuntimePlatformAvatarService::FOnGetAvatarUrlComplete::CreateLambda(
                    [OnDone](const FExternalUserInfoRef &, const FError &Error, const FString &AvatarTexture) {
                        if (Error.WasSuccessful())
                        {
                            OnDone(AvatarTexture);
                        }
                        else
                        {
                            OnDone(TOptional<FString>());
                        }
                    }));
            return true;
        },
        [OnComplete, DefaultAvatarUrl](TArray<TOptional<FString>> AvatarUrls) {
            for (const auto &AvatarUrl : AvatarUrls)
            {
                if (AvatarUrl.IsSet())
                {
                    OnComplete.ExecuteIfBound(true, AvatarUrl.GetValue());
                    return;
                }
            }
            OnComplete.ExecuteIfBound(false, DefaultAvatarUrl);
        });
}

void FOnlineAvatarInterfaceSynthetic::DownloadAvatarTextureFromUrl(
    const FString &ResultAvatarUrl,
    const ::FOnGetAvatarComplete &OnComplete,
    const TSoftObjectPtr<UTexture> &DefaultTexture)
{
    using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS;

    UE_LOG(
        LogOnlineSubsystemRedpointEOS,
        VeryVerbose,
        TEXT("%s: Downloading avatar from URL '%s'..."),
        ANSI_TO_TCHAR(__FUNCTION__),
        *ResultAvatarUrl);
    auto Request = FHttpModule::Get().CreateRequest();
    Request->SetVerb("GET");
    Request->SetURL(ResultAvatarUrl);
    Request->OnProcessRequestComplete() = FHttpRequestCompleteDelegate::CreateSPLambda(
        this,
        [OnComplete, DefaultTexture](FHttpRequestPtr, FHttpResponsePtr Response, bool bConnectedSuccessfully) {
            // If the request failed, return the default texture.
            if (!bConnectedSuccessfully || !Response.IsValid() ||
                !EHttpResponseCodes::IsOk(Response->GetResponseCode()))
            {
                UE_LOG(
                    LogOnlineSubsystemRedpointEOS,
                    VeryVerbose,
                    TEXT("%s: Returning default texture for avatar load, as the avatar could not be downloaded from "
                         "the URL."),
                    ANSI_TO_TCHAR(__FUNCTION__));
                OnComplete.ExecuteIfBound(false, DefaultTexture);
                return;
            }

            // Attempt to load the texture, and return it if successful.
            UTexture *LoadedTexture = Texture::FTextureLoader::LoadTextureFromHttpResponse(Response);
            if (IsValid(LoadedTexture))
            {
                UE_LOG(
                    LogOnlineSubsystemRedpointEOS,
                    VeryVerbose,
                    TEXT("%s: Returning avatar texture downloaded from URL."),
                    ANSI_TO_TCHAR(__FUNCTION__));
                OnComplete.ExecuteIfBound(true, LoadedTexture);
            }
            else
            {
                UE_LOG(
                    LogOnlineSubsystemRedpointEOS,
                    VeryVerbose,
                    TEXT("%s: Returning default texture for avatar load, as the downloaded avatar could not be "
                         "understood as a graphics image."),
                    ANSI_TO_TCHAR(__FUNCTION__));
                OnComplete.ExecuteIfBound(false, DefaultTexture);
            }
        });
    Request->ProcessRequest();
}

bool FOnlineAvatarInterfaceSynthetic::GetAvatar(
    const FUniqueNetId &LocalUserId,
    const FUniqueNetId &TargetUserId,
    TSoftObjectPtr<UTexture> DefaultTexture,
    ::FOnGetAvatarComplete OnComplete)
{
    using namespace ::Redpoint::EOS::Platform::Services;
    using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS;

    // If this is an EOS user, check the synthetic friends system to see if we have a cached avatar URL to fallback on
    // if everything else fails.
    auto CachedAvatarUrl = this->GetCachedAvatarUrl(LocalUserId, TargetUserId);
    auto FallbackToCachedAvatarUrlOnFailure =
        TDelegate<void()>::CreateSPLambda(this, [this, CachedAvatarUrl, OnComplete, DefaultTexture]() {
            if (CachedAvatarUrl.IsEmpty())
            {
                OnComplete.ExecuteIfBound(false, DefaultTexture);
                return;
            }

            UE_LOG(
                LogOnlineSubsystemRedpointEOS,
                VeryVerbose,
                TEXT("%s: Falling back to cached avatar URL for friend as the local platform can not obtain the "
                     "avatar."),
                ANSI_TO_TCHAR(__FUNCTION__));
            this->DownloadAvatarTextureFromUrl(CachedAvatarUrl, OnComplete, DefaultTexture);
        });

    // Use the platform services to get the avatar.
    this->ResolveTargetUserIdToExternalUserInfoOrFail(
        LocalUserId,
        TargetUserId,
        FOnResolveFailure::CreateSPLambda(
            this,
            [FallbackToCachedAvatarUrlOnFailure]() {
                FallbackToCachedAvatarUrlOnFailure.ExecuteIfBound();
            }),
        FOnResolveSuccess::CreateSPLambda(
            this,
            [this, OnComplete, FallbackToCachedAvatarUrlOnFailure](
                const FRuntimePlatformServiceCallContextRef &CallContext,
                const FExternalUserInfoRef &UserInfo) {
                FMultiOperation<TSharedRef<IRuntimePlatformIntegration>, TSoftObjectPtr<UTexture>>::RunSP(
                    this->AsShared(),
                    Redpoint::EOS::Core::FModule::GetModuleChecked()
                        .GetRuntimePlatformRegistry()
                        ->GetRuntimePlatformIntegrations(),
                    [CallContext, UserInfo](
                        TSharedRef<IRuntimePlatformIntegration> Integration,
                        TFunction<void(TSoftObjectPtr<UTexture> OutValue)> OnDone) -> bool {
                        Integration->GetAvatarService()->GetAvatar(
                            CallContext,
                            UserInfo,
                            IRuntimePlatformAvatarService::FOnGetAvatarComplete::CreateLambda(
                                [OnDone](
                                    const FExternalUserInfoRef &,
                                    const FError &Error,
                                    const TSoftObjectPtr<UTexture> &AvatarTexture) {
                                    OnDone(AvatarTexture);
                                }));
                        return true;
                    },
                    [this, CallContext, UserInfo, OnComplete, FallbackToCachedAvatarUrlOnFailure](
                        TArray<TSoftObjectPtr<UTexture>> Textures) {
                        // Use the first loaded texture, if one is available.
                        for (const auto &Texture : Textures)
                        {
                            if (Texture.IsValid())
                            {
                                UE_LOG(
                                    LogOnlineSubsystemRedpointEOS,
                                    VeryVerbose,
                                    TEXT("%s: Returning texture from runtime platform GetAvatar call."),
                                    ANSI_TO_TCHAR(__FUNCTION__));
                                OnComplete.ExecuteIfBound(true, Texture);
                                return;
                            }
                        }

                        // Attempt to fallback to GetAvatarUrl, in case this avatar system only provides URLs.
                        this->OnUserInfoResolvedForGetAvatarUrl(
                            CallContext,
                            UserInfo,
                            TEXT(""),
                            ::FOnGetAvatarUrlComplete::CreateSPLambda(
                                this,
                                [this,
                                 OnComplete,
                                 FallbackToCachedAvatarUrlOnFailure](bool bWasSuccessful, FString ResultAvatarUrl) {
                                    if (bWasSuccessful && !ResultAvatarUrl.IsEmpty())
                                    {
                                        UE_LOG(
                                            LogOnlineSubsystemRedpointEOS,
                                            VeryVerbose,
                                            TEXT("%s: Forwarding GetAvatar call to download operation."),
                                            ANSI_TO_TCHAR(__FUNCTION__));
                                        this->DownloadAvatarTextureFromUrl(
                                            ResultAvatarUrl,
                                            ::FOnGetAvatarComplete::CreateSPLambda(
                                                this,
                                                [OnComplete, FallbackToCachedAvatarUrlOnFailure](
                                                    bool bDownloadWasSuccessful,
                                                    TSoftObjectPtr<UTexture> ResultTexture) {
                                                    if (bDownloadWasSuccessful)
                                                    {
                                                        OnComplete.ExecuteIfBound(
                                                            bDownloadWasSuccessful,
                                                            ResultTexture);
                                                    }
                                                    else
                                                    {
                                                        FallbackToCachedAvatarUrlOnFailure.ExecuteIfBound();
                                                    }
                                                }),
                                            nullptr);
                                    }
                                    else
                                    {
                                        UE_LOG(
                                            LogOnlineSubsystemRedpointEOS,
                                            VeryVerbose,
                                            TEXT("%s: Returning default texture for GetAvatar as the avatar could not "
                                                 "be loaded via texture or URL."),
                                            ANSI_TO_TCHAR(__FUNCTION__));
                                        FallbackToCachedAvatarUrlOnFailure.ExecuteIfBound();
                                    }
                                }));
                    });
            }));
    return true;
}

bool FOnlineAvatarInterfaceSynthetic::GetAvatarUrl(
    const FUniqueNetId &LocalUserId,
    const FUniqueNetId &TargetUserId,
    FString DefaultAvatarUrl,
    ::FOnGetAvatarUrlComplete OnComplete)
{
    using namespace ::Redpoint::EOS::Platform::Services;
    using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS;

    // If this is an EOS user, check the synthetic friends system to see if we have a cached avatar URL to fallback on
    // if everything else fails.
    auto CachedAvatarUrl = this->GetCachedAvatarUrl(LocalUserId, TargetUserId);
    auto FallbackToCachedAvatarUrlOnFailure =
        TDelegate<void()>::CreateSPLambda(this, [OnComplete, CachedAvatarUrl, DefaultAvatarUrl]() {
            if (CachedAvatarUrl.IsEmpty())
            {
                OnComplete.ExecuteIfBound(false, DefaultAvatarUrl);
                return;
            }
            else
            {
                UE_LOG(
                    LogOnlineSubsystemRedpointEOS,
                    VeryVerbose,
                    TEXT("%s: Falling back to cached avatar URL for friend as the local platform can not obtain the "
                         "avatar."),
                    ANSI_TO_TCHAR(__FUNCTION__));
                OnComplete.ExecuteIfBound(true, CachedAvatarUrl);
                return;
            }
        });

    // Use the platform services to get the avatar URL.
    this->ResolveTargetUserIdToExternalUserInfoOrFail(
        LocalUserId,
        TargetUserId,
        FOnResolveFailure::CreateSPLambda(
            this,
            [FallbackToCachedAvatarUrlOnFailure]() {
                FallbackToCachedAvatarUrlOnFailure.ExecuteIfBound();
            }),
        FOnResolveSuccess::CreateSP(
            this,
            &FOnlineAvatarInterfaceSynthetic::OnUserInfoResolvedForGetAvatarUrl,
            DefaultAvatarUrl,
            ::FOnGetAvatarUrlComplete::CreateSPLambda(
                this,
                [FallbackToCachedAvatarUrlOnFailure, OnComplete](bool bWasSuccessful, FString ResultAvatarUrl) {
                    if (bWasSuccessful)
                    {
                        OnComplete.ExecuteIfBound(bWasSuccessful, ResultAvatarUrl);
                    }
                    else
                    {
                        FallbackToCachedAvatarUrlOnFailure.ExecuteIfBound();
                    }
                })));
    return true;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()