// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Online/CoreOnline.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Identity/OnlineIdentityInterfaceEOS.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSInterfaces/Interfaces/OnlineAvatarInterface.h"
#include "RedpointEOSPlatform/Services/RuntimePlatformServiceCallContext.h"
#include "RedpointEOSUserCache/ExternalUserInfo.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1278425628, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Avatar)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::UserCache;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Identity;
using namespace ::Redpoint::EOS::Platform::Services;

class FOnlineAvatarInterfaceSynthetic : public IOnlineAvatar, public TSharedFromThis<FOnlineAvatarInterfaceSynthetic>
{
private:
    FPlatformHandle PlatformHandle;
    TSharedRef<IOnlineIdentityInterfaceEOS> Identity;
    TSharedRef<IOnlineFriends> Friends;

    typedef TDelegate<void()> FOnResolveFailure;
    typedef TDelegate<
        void(const FRuntimePlatformServiceCallContextRef &CallContext, const FExternalUserInfoRef &UserInfo)>
        FOnResolveSuccess;

    /**
     * As a fallback, we check the 'avatarUrl' attribute if the target user is on our friend list, in case we can
     * fetch avatars cross-platform by downloading from the URL. The returned URL is only used as a last-resort if
     * everything else fails.
     */
    FString GetCachedAvatarUrl(const FUniqueNetId &LocalUserId, const FUniqueNetId &TargetUserId);

    /**
     * The TargetUserId provided to IOnlineAvatar functions could either be an EOS user ID, or a user ID directly from a
     * local online subsystem like Steam. This function resolves the TargetUserId using different paths depending on
     * what type it, and returns the loaded FExternalUserInfoRef so that it can be used with
     * IRuntimePlatformAvatarService.
     */
    void ResolveTargetUserIdToExternalUserInfoOrFail(
        const FUniqueNetId &LocalUserId,
        const FUniqueNetId &TargetUserId,
        const FOnResolveFailure &OnFailure,
        const FOnResolveSuccess &OnResolved);

    /**
     * When the target user resolves in ResolveTargetUserIdToExternalUserInfoOrFail, but the
     * IRuntimePlatformAvatarService::GetAvatar does not return an avatar, we fallback to trying
     * IRuntimePlatformAvatarService::GetAvatarUrl. This function contains the logic shared between that fallback path
     * and IOnlineAvatar::GetAvatarUrl.
     */
    void OnUserInfoResolvedForGetAvatarUrl(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FExternalUserInfoRef &UserInfo,
        FString DefaultAvatarUrl,
        ::FOnGetAvatarUrlComplete OnComplete);

    /**
     * When we've successfully retrieved an avatar URL via OnUserInfoResolvedForGetAvatarUrl, or via cross-platform
     * friend attributes, and the original call is IOnlineAvatar::GetAvatar, this function downloads the avatar texture
     * from the URL and loads it into memory.
     */
    void DownloadAvatarTextureFromUrl(
        const FString &ResultAvatarUrl,
        const ::FOnGetAvatarComplete &OnComplete,
        const TSoftObjectPtr<UTexture> &DefaultTexture);

public:
    FOnlineAvatarInterfaceSynthetic(
        const FPlatformHandle &InPlatformHandle,
        const TSharedRef<IOnlineIdentityInterfaceEOS> &InIdentity,
        const TSharedRef<IOnlineFriends> &InFriends);
    UE_NONCOPYABLE(FOnlineAvatarInterfaceSynthetic);
    virtual ~FOnlineAvatarInterfaceSynthetic() override = default;

    bool GetAvatar(
        const FUniqueNetId &LocalUserId,
        const FUniqueNetId &TargetUserId,
        TSoftObjectPtr<UTexture> DefaultTexture,
        ::FOnGetAvatarComplete OnComplete) override;

    bool GetAvatarUrl(
        const FUniqueNetId &LocalUserId,
        const FUniqueNetId &TargetUserId,
        FString DefaultAvatarUrl,
        ::FOnGetAvatarUrlComplete OnComplete) override;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Avatar
{
REDPOINT_EOS_FILE_NS_EXPORT(
    1278425628,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Avatar,
    FOnlineAvatarInterfaceSynthetic)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()