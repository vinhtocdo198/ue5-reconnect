// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationOSSv1/RuntimePlatformIntegrationOSSv1.h"

#include "RedpointEOSCore/Utils/MakeSharedWithEvents.h"
#include "RedpointEOSPlatformIntegrationOSSv1/Services/OSSv1RuntimePlatformAuthService.h"
#include "RedpointEOSPlatformIntegrationOSSv1/Services/OSSv1RuntimePlatformAvatarService.h"
#include "RedpointEOSPlatformIntegrationOSSv1/Services/OSSv1RuntimePlatformPresenceService.h"
#include "RedpointEOSPlatformIntegrationOSSv1/Services/OSSv1RuntimePlatformRelationshipService.h"
#include "RedpointEOSPlatformIntegrationOSSv1/Services/OSSv1RuntimePlatformUserService.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(764385463, Redpoint::EOS::Platform::Integration::OSSv1)
{
using namespace ::Redpoint::EOS::Platform::Integration::OSSv1::Services;
using namespace ::Redpoint::EOS::Core::Utils;

FRuntimePlatformIntegrationOSSv1::FRuntimePlatformIntegrationOSSv1(
    const EOS_EExternalAccountType &InExternalAccountTypeToHandle,
    const FName &InSubsystemName,
    const FResolveUserIdToExternalAccountIdInfo &InExternalInfoResolver,
    const FString &InFriendsListName,
    const FString &InRecentPlayersNamespace,
    EOS_EExternalCredentialType InObtainedCredentialType,
    const FString &InAuthenticatedWithValue,
    const FString &InTokenAuthAttributeName)
    : AvatarService(MakeShared<FOSSv1RuntimePlatformAvatarService>(InExternalAccountTypeToHandle, InSubsystemName))
    , UserService(MakeShared<FOSSv1RuntimePlatformUserService>(
          InExternalAccountTypeToHandle,
          InSubsystemName,
          InExternalInfoResolver))
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    , AuthService(MakeShared<FOSSv1RuntimePlatformAuthService>(
          InSubsystemName,
          InObtainedCredentialType,
          InAuthenticatedWithValue,
          InTokenAuthAttributeName))
    , RelationshipService(MakeShared<FOSSv1RuntimePlatformRelationshipService>(
          InExternalAccountTypeToHandle,
          InSubsystemName,
          InExternalInfoResolver,
          InFriendsListName,
          InRecentPlayersNamespace))
    , PresenceService(MakeSharedWithEvents<FOSSv1RuntimePlatformPresenceService>(
          InExternalAccountTypeToHandle,
          InSubsystemName,
          InExternalInfoResolver,
          InFriendsListName))
#endif
{
}

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

TSharedRef<FOSSv1RuntimePlatformPresenceService> FRuntimePlatformIntegrationOSSv1::GetOSSv1PresenceService() const
{
    return this->PresenceService;
}

TSharedRef<FOSSv1RuntimePlatformRelationshipService> FRuntimePlatformIntegrationOSSv1::GetOSSv1RelationshipService()
    const
{
    return this->RelationshipService;
}

#endif

FRuntimePlatformIntegrationOSSv1::FRuntimePlatformIntegrationOSSv1(
    const EOS_EExternalAccountType &InExternalAccountTypeToHandle,
    const FName &InSubsystemName,
    const FResolveUserIdToExternalAccountIdInfo &InExternalInfoResolver,
    const FString &InFriendsListName,
    const FString &InRecentPlayersNamespace
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    ,
    const TSharedRef<IRuntimePlatformAuthService> &InAuthService
#endif
    )
    : AvatarService(MakeShared<FOSSv1RuntimePlatformAvatarService>(InExternalAccountTypeToHandle, InSubsystemName))
    , UserService(MakeShared<FOSSv1RuntimePlatformUserService>(
          InExternalAccountTypeToHandle,
          InSubsystemName,
          InExternalInfoResolver))
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    , AuthService(InAuthService)
    , RelationshipService(MakeShared<FOSSv1RuntimePlatformRelationshipService>(
          InExternalAccountTypeToHandle,
          InSubsystemName,
          InExternalInfoResolver,
          InFriendsListName,
          InRecentPlayersNamespace))
    , PresenceService(MakeSharedWithEvents<FOSSv1RuntimePlatformPresenceService>(
          InExternalAccountTypeToHandle,
          InSubsystemName,
          InExternalInfoResolver,
          InFriendsListName))
#endif
{
}

TSharedRef<IRuntimePlatformAvatarService> FRuntimePlatformIntegrationOSSv1::GetAvatarService() const
{
    return this->AvatarService;
}

TSharedRef<IRuntimePlatformUserService> FRuntimePlatformIntegrationOSSv1::GetUserService() const
{
    return this->UserService;
}

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

TSharedRef<IRuntimePlatformAuthService> FRuntimePlatformIntegrationOSSv1::GetAuthService() const
{
    return this->AuthService;
}

#endif

}; // namespace REDPOINT_EOS_FILE_NS_ID(764385463,Redpoint::EOS::Platform::Integration::OSSv1)

REDPOINT_EOS_CODE_GUARD_END()