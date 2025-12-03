// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationMeta/Services/MetaRuntimePlatformAvatarService.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_META_ENABLED
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSCore/Utils/TextureLoader.h"
#include "RedpointEOSPlatformIntegrationMeta/Logging.h"
#include "RedpointEOSPlatformIntegrationMeta/MetaSDK.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_META_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(3278636343, Redpoint::EOS::Platform::Integration::Meta::Services)
{
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::Platform::Integration::Meta;
using namespace ::Redpoint::EOS::Platform::Integration::Meta::Services;
using namespace ::Redpoint::EOS::API;

void FMetaRuntimePlatformAvatarService::GetAvatar(
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

void FMetaRuntimePlatformAvatarService::GetAvatarUrl(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FExternalUserInfoRef &UserInfo,
    const FOnGetAvatarUrlComplete &OnComplete)
{
    auto *MetaAccount = UserInfo->GetLinkedExternalAccounts().Find(EOS_EExternalAccountType::EOS_EAT_OCULUS);
    if (MetaAccount == nullptr)
    {
        OnComplete.ExecuteIfBound(
            UserInfo,
            Errors::NotFound(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("The target user account does not have a Meta account linked to retrieve the avatar for.")),
            TEXT(""));
        return;
    }

    ovrID MetaAccountID;
    if (!ovrID_FromString(&MetaAccountID, TCHAR_TO_ANSI(*MetaAccount->AccountId)))
    {
        OnComplete.ExecuteIfBound(
            UserInfo,
            Errors::NotFound(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("The target user account has a Meta account, but the associated Meta account ID is not valid.")),
            TEXT(""));
        return;
    }

    FMetaSDK::GetUser(
        MetaAccountID,
        FMetaSDK::FOnGetUserComplete::CreateSPLambda(
            this,
            [UserInfo, OnComplete](const FError &ErrorCode, const TOptional<ovrUserHandle> &User) {
                if (!ErrorCode.WasSuccessful())
                {
                    OnComplete.ExecuteIfBound(UserInfo, ErrorCode, TEXT(""));
                    return;
                }

                OnComplete.ExecuteIfBound(
                    UserInfo,
                    Errors::Success(),
                    ANSI_TO_TCHAR(ovr_User_GetImageUrl(User.GetValue())));
            }));
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()