// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationItchIo/Services/ItchIoRuntimePlatformAvatarService.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_ITCH_IO_ENABLED
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSCore/Utils/TextureLoader.h"
#include "RedpointEOSPlatformIntegrationItchIo/ItchIoSystem.h"
#include "RedpointEOSPlatformIntegrationItchIo/Logging.h"
#include "RedpointEOSUserCache/ExternalUserInfo.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_ITCH_IO_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(280458675, Redpoint::EOS::Platform::Integration::ItchIo::Services)
{
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::Platform::Integration::ItchIo;
using namespace ::Redpoint::EOS::Platform::Integration::ItchIo::Services;
using namespace ::Redpoint::EOS::API;

void FItchIoRuntimePlatformAvatarService::GetAvatar(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FExternalUserInfoRef &UserInfo,
    const FOnGetAvatarComplete &OnComplete)
{
    this->GetAvatarUrl(
        CallContext,
        UserInfo,
        FOnGetAvatarUrlComplete::CreateSPLambda(
            this,
            [this, UserInfo, OnComplete](const FExternalUserInfoRef &, const FError &Error, const FString &AvatarUrl) {
                if (!Error.WasSuccessful())
                {
                    OnComplete.ExecuteIfBound(UserInfo, Error, nullptr);
                    return;
                }

                if (AvatarUrl.IsEmpty())
                {
                    OnComplete.ExecuteIfBound(UserInfo, Errors::Success(), nullptr);
                    return;
                }

                FTextureLoader::LoadTextureFromHttpUrl(
                    AvatarUrl,
                    FTextureLoader::FOnTextureLoadComplete::CreateSPLambda(
                        this,
                        [UserInfo, OnComplete](const TSoftObjectPtr<UTexture> &Texture) {
                            OnComplete.ExecuteIfBound(UserInfo, Errors::Success(), Texture);
                        }));
            }));
}

void FItchIoRuntimePlatformAvatarService::GetAvatarUrl(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FExternalUserInfoRef &UserInfo,
    const FOnGetAvatarUrlComplete &OnComplete)
{
    auto *ItchIoAccount = UserInfo->GetLinkedExternalAccounts().Find(EOS_EExternalAccountType::EOS_EAT_ITCHIO);
    if (ItchIoAccount == nullptr)
    {
        OnComplete.ExecuteIfBound(
            UserInfo,
            Errors::NotFound(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("The target user account does not have a itch.io account linked to retrieve the avatar for.")),
            TEXT(""));
        return;
    }

    auto LocalUser = CallContext->PlatformHandle->GetSystem<FItchIoSystem>()->GetLocalUser();
    if (!LocalUser.IsSet() || FString::Printf(TEXT("%d"), LocalUser->Id) != ItchIoAccount->AccountId)
    {
        OnComplete.ExecuteIfBound(
            UserInfo,
            Errors::NotFound(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("Avatars can only be retrieved for the locally signed in user.")),
            TEXT(""));
        return;
    }

    if (LocalUser->CoverUrl.IsEmpty())
    {
        OnComplete.ExecuteIfBound(
            UserInfo,
            Errors::NotFound(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("The local user does not have an avatar set on itch.io.")),
            TEXT(""));
        return;
    }

    OnComplete.ExecuteIfBound(UserInfo, Errors::Success(), LocalUser->CoverUrl);
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()