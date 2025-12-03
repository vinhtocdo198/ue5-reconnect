// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationMeta/Services/MetaRuntimePlatformRelationshipService.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_META_ENABLED
#include "RedpointEOSPlatformIntegrationMeta/MetaSDK.h"
#include "RedpointEOSPlatformIntegrationMeta/Services/MetaExternalUserInfo.h"
#include "RedpointEOSUserCache/ExternalUserInfo.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_META_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(2563615876, Redpoint::EOS::Platform::Integration::Meta::Services)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Platform::Integration::Meta;
using namespace ::Redpoint::EOS::Platform::Integration::Meta::Services;
using namespace ::Redpoint::EOS::UserCache;

FString FMetaRuntimePlatformRelationshipService::GetCacheKey() const
{
    return TEXT("Meta");
}

bool FMetaRuntimePlatformRelationshipService::AllowRelationshipCaching() const
{
    return true;
}

void FMetaRuntimePlatformRelationshipService::GetRelationships(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    ERelationshipType RelationshipTypes,
    const FOnGetRelationshipComplete &OnComplete)
{
#if REDPOINT_EOS_META_HAS_USER_GET_BLOCKED_USERS
    auto GetBlockedUsers = [OnComplete](const TOptional<TArray<FExternalUserInfoRef>> &Friends) {
        FMetaSDK::GetBlockedUsers(FMetaSDK::FOnGetBlockedUsersComplete::CreateLambda(
            [Friends, OnComplete](const FError &ErrorCode, const TOptional<TArray<ovrBlockedUserHandle>> &Blocks) {
                if (!ErrorCode.WasSuccessful())
                {
                    OnComplete.ExecuteIfBound(ErrorCode, TMap<ERelationshipType, TArray<FExternalUserInfoRef>>());
                    return;
                }

                auto MetaBlocked = TArray<FExternalUserInfoRef>();
                for (const auto Block : Blocks.GetValue())
                {
                    MetaBlocked.Add(MakeShared<FMetaExternalUserInfo>(ovr_BlockedUser_GetId(Block), TEXT("")));
                }

                auto Results = TMap<ERelationshipType, TArray<FExternalUserInfoRef>>();
                if (Friends.IsSet())
                {
                    Results.Add(ERelationshipType::MutualFriends, Friends.GetValue());
                }
                Results.Add(ERelationshipType::Blocked, MetaBlocked);

                OnComplete.ExecuteIfBound(Errors::Success(), Results);
                return;
            }));
    };
#endif

    if ((RelationshipTypes & ERelationshipType::MutualFriends) != ERelationshipType::None)
    {
        FMetaSDK::GetLoggedInUserFriends(FMetaSDK::FOnGetLoggedInUserFriendsComplete::CreateLambda(
            [
#if REDPOINT_EOS_META_HAS_USER_GET_BLOCKED_USERS
                GetBlockedUsers,
                RelationshipTypes,
#endif
                OnComplete](const FError &ErrorCode, const TOptional<TArray<ovrUserHandle>> &Friends) {
                if (!ErrorCode.WasSuccessful())
                {
                    OnComplete.ExecuteIfBound(ErrorCode, TMap<ERelationshipType, TArray<FExternalUserInfoRef>>());
                    return;
                }

                auto MetaFriends = TArray<FExternalUserInfoRef>();
                for (const auto Friend : Friends.GetValue())
                {
#if REDPOINT_EOS_META_HAS_USER_GET_DISPLAY_NAME
                    FString DisplayName = UTF8_TO_TCHAR(ovr_User_GetDisplayName(Friend));
#else
                    FString DisplayName = UTF8_TO_TCHAR(ovr_User_GetOculusID(Friend));
#endif
                    MetaFriends.Add(MakeShared<FMetaExternalUserInfo>(ovr_User_GetID(Friend), DisplayName));
                }

#if REDPOINT_EOS_META_HAS_USER_GET_BLOCKED_USERS
                if ((RelationshipTypes & ERelationshipType::Blocked) != ERelationshipType::None)
                {
                    GetBlockedUsers(MetaFriends);
                }
                else
                {
#endif
                    OnComplete.ExecuteIfBound(
                        Errors::Success(),
                        TMap<ERelationshipType, TArray<FExternalUserInfoRef>>{
                            {ERelationshipType::MutualFriends, MetaFriends}});
                    return;
#if REDPOINT_EOS_META_HAS_USER_GET_BLOCKED_USERS
                }
#endif
            }));
    }
#if REDPOINT_EOS_META_HAS_USER_GET_BLOCKED_USERS
    else if ((RelationshipTypes & ERelationshipType::Blocked) != ERelationshipType::None)
    {
        GetBlockedUsers(TOptional<TArray<FExternalUserInfoRef>>());
    }
#endif
    else
    {
        OnComplete.ExecuteIfBound(Errors::Success(), TMap<ERelationshipType, TArray<FExternalUserInfoRef>>());
        return;
    }
}

IRuntimePlatformRelationshipService::FOnRelationshipsChanged &FMetaRuntimePlatformRelationshipService::
    OnRelationshipsChanged(const FRuntimePlatformServiceCallContextRef &CallContext)
{
    // The Meta Quest SDK never notifies of changes to friends or blocked users, so this delegate never fires.
    return this->OnRelationshipsChangedDelegate;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()