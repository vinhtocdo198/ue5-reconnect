// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationOSSv1/Services/OSSv1RuntimePlatformAvatarService.h"

#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "RedpointEOSInterfaces/Interfaces/OnlineAvatarInterface.h"
#include "RedpointEOSPlatformIntegrationOSSv1/Caller/SafeOSSv1Caller.h"
#include "RedpointEOSPlatformIntegrationOSSv1/Logging.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2821691268, Redpoint::EOS::Platform::Integration::OSSv1::Services)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Platform::Integration::OSSv1;
using namespace ::Redpoint::EOS::Platform::Integration::OSSv1::Caller;

FOSSv1RuntimePlatformAvatarService::FOSSv1RuntimePlatformAvatarService(
    const EOS_EExternalAccountType &InExternalAccountTypeToHandle,
    const FName &InSubsystemName)
    : ExternalAccountTypeToHandle(InExternalAccountTypeToHandle)
    , SubsystemName(InSubsystemName)
{
}

void FOSSv1RuntimePlatformAvatarService::GetAvatar(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FExternalUserInfoRef &UserInfo,
    const FOnGetAvatarComplete &OnComplete)
{
    // Locate the external account information, if this user has it linked.
    bool bExternalAccountFound = false;
    FUserExternalAccountInfo ExternalAccount;
    for (const auto &ExternalAccountKV : UserInfo->GetLinkedExternalAccounts())
    {
        if (ExternalAccountKV.Key == this->ExternalAccountTypeToHandle)
        {
            bExternalAccountFound = true;
            ExternalAccount = ExternalAccountKV.Value;
            break;
        }
    }
    if (!bExternalAccountFound)
    {
        OnComplete.ExecuteIfBound(
            UserInfo,
            Errors::NotFound(
                ANSI_TO_TCHAR(__FUNCTION__),
                FString::Printf(
                    TEXT("The user account does not have an external '%s' account linked to retrieve the avatar for."),
                    *this->SubsystemName.ToString())),
            nullptr);
        return;
    }

    // Get the target online subsystem, and the relevant interfaces, falling back to 'Redpoint<name>' when it's not
    // natively available.
    TSharedPtr<IOnlineAvatar> AvatarInterface = nullptr;
    TSharedPtr<IOnlineIdentity> IdentityInterface = nullptr;
    {
        auto *OSSInstance = Online::GetSubsystem(CallContext->World.Get(), this->SubsystemName);
        if (OSSInstance != nullptr)
        {
            if (!AvatarInterface.IsValid())
            {
                AvatarInterface = Online::GetAvatarInterface(OSSInstance);
            }
            if (!IdentityInterface.IsValid())
            {
                IdentityInterface = OSSInstance->GetIdentityInterface();
            }
        }
    }
    {
        auto *OSSInstance = Online::GetSubsystem(
            CallContext->World.Get(),
            FName(*FString::Printf(TEXT("Redpoint%s"), *this->SubsystemName.ToString())));
        if (OSSInstance != nullptr)
        {
            if (!AvatarInterface.IsValid())
            {
                AvatarInterface = Online::GetAvatarInterface(OSSInstance);
            }
            if (!IdentityInterface.IsValid())
            {
                IdentityInterface = OSSInstance->GetIdentityInterface();
            }
        }
    }

    // If we still don't have an avatar interface, this subsystem doesn't support avatars.
    if (!IdentityInterface.IsValid() || !AvatarInterface.IsValid())
    {
        OnComplete.ExecuteIfBound(
            UserInfo,
            Errors::NotImplemented(
                ANSI_TO_TCHAR(__FUNCTION__),
                FString::Printf(TEXT("The subsystem '%s' does not support avatars."), *this->SubsystemName.ToString())),
            nullptr);
        return;
    }

    // Ask the target online subsystem to get the local user's FUniqueNetId for use with the avatar interface.
    auto LocalUserId = IdentityInterface->GetUniquePlayerId(CallContext->LocalUserNum);
    if (!LocalUserId.IsValid())
    {
        OnComplete.ExecuteIfBound(
            UserInfo,
            Errors::AccessDenied(
                ANSI_TO_TCHAR(__FUNCTION__),
                FString::Printf(
                    TEXT("Subsystem '%s' is not signed in for local user %d, so avatars can not be retrieved on their "
                         "behalf."),
                    *this->SubsystemName.ToString(),
                    CallContext->LocalUserNum)),
            nullptr);
        return;
    }

    // Ask the target online subsystem to convert the user ID into an FUniqueNetId for use with the avatar interface.
    auto TargetUserId = IdentityInterface->CreateUniquePlayerId(ExternalAccount.AccountId);
    if (!TargetUserId.IsValid())
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationOSSv1,
            Error,
            TEXT("Subsystem '%s' could not convert user ID '%s' into an FUniqueNetId."),
            *this->SubsystemName.ToString(),
            *ExternalAccount.AccountId);
        OnComplete.ExecuteIfBound(
            UserInfo,
            Errors::UnexpectedError(
                ANSI_TO_TCHAR(__FUNCTION__),
                FString::Printf(
                    TEXT("Subsystem '%s' could not convert user ID '%s' into an FUniqueNetId."),
                    *this->SubsystemName.ToString(),
                    *ExternalAccount.AccountId)),
            nullptr);
        return;
    }

    // Now call the real avatar implementation.
    FSafeOSSv1Caller::GetAvatar(
        AvatarInterface.ToSharedRef(),
        *LocalUserId,
        *TargetUserId,
        nullptr,
        ::FOnGetAvatarComplete::CreateSPLambda(
            this,
            [this, UserInfo, OnComplete](bool bWasSuccessful, TSoftObjectPtr<UTexture> ResultTexture) {
                OnComplete.ExecuteIfBound(
                    UserInfo,
                    bWasSuccessful ? Errors::Success()
                                   : Errors::UnexpectedError(
                                         ANSI_TO_TCHAR(__FUNCTION__),
                                         FString::Printf(
                                             TEXT("The GetAvatar call on the delegated '%s' subsystem failed."),
                                             *this->SubsystemName.ToString())),
                    ResultTexture);
            }));
}

void FOSSv1RuntimePlatformAvatarService::GetAvatarUrl(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FExternalUserInfoRef &UserInfo,
    const FOnGetAvatarUrlComplete &OnComplete)
{
    // Locate the external account information, if this user has it linked.
    bool bExternalAccountFound = false;
    FUserExternalAccountInfo ExternalAccount;
    for (const auto &ExternalAccountKV : UserInfo->GetLinkedExternalAccounts())
    {
        if (ExternalAccountKV.Key == this->ExternalAccountTypeToHandle)
        {
            bExternalAccountFound = true;
            ExternalAccount = ExternalAccountKV.Value;
            break;
        }
    }
    if (!bExternalAccountFound)
    {
        OnComplete.ExecuteIfBound(
            UserInfo,
            Errors::NotFound(
                ANSI_TO_TCHAR(__FUNCTION__),
                FString::Printf(
                    TEXT("The user account does not have an external '%s' account linked to retrieve the avatar for."),
                    *this->SubsystemName.ToString())),
            TEXT(""));
        return;
    }

    // Get the target online subsystem, and the relevant interfaces, falling back to 'Redpoint<name>' when it's not
    // natively available.
    TSharedPtr<IOnlineAvatar> AvatarInterface = nullptr;
    TSharedPtr<IOnlineIdentity> IdentityInterface = nullptr;
    {
        auto *OSSInstance = Online::GetSubsystem(CallContext->World.Get(), this->SubsystemName);
        if (OSSInstance != nullptr)
        {
            if (!AvatarInterface.IsValid())
            {
                AvatarInterface = Online::GetAvatarInterface(OSSInstance);
            }
            if (!IdentityInterface.IsValid())
            {
                IdentityInterface = OSSInstance->GetIdentityInterface();
            }
        }
    }
    {
        auto *OSSInstance = Online::GetSubsystem(
            CallContext->World.Get(),
            FName(*FString::Printf(TEXT("Redpoint%s"), *this->SubsystemName.ToString())));
        if (OSSInstance != nullptr)
        {
            if (!AvatarInterface.IsValid())
            {
                AvatarInterface = Online::GetAvatarInterface(OSSInstance);
            }
            if (!IdentityInterface.IsValid())
            {
                IdentityInterface = OSSInstance->GetIdentityInterface();
            }
        }
    }

    // If we still don't have an avatar interface, this subsystem doesn't support avatars.
    if (!IdentityInterface.IsValid() || !AvatarInterface.IsValid())
    {
        OnComplete.ExecuteIfBound(
            UserInfo,
            Errors::NotImplemented(
                ANSI_TO_TCHAR(__FUNCTION__),
                FString::Printf(TEXT("The subsystem '%s' does not support avatars."), *this->SubsystemName.ToString())),
            TEXT(""));
        return;
    }

    // Ask the target online subsystem to get the local user's FUniqueNetId for use with the avatar interface.
    auto LocalUserId = IdentityInterface->GetUniquePlayerId(CallContext->LocalUserNum);
    if (!LocalUserId.IsValid())
    {
        OnComplete.ExecuteIfBound(
            UserInfo,
            Errors::AccessDenied(
                ANSI_TO_TCHAR(__FUNCTION__),
                FString::Printf(
                    TEXT("Subsystem '%s' is not signed in for local user %d, so avatars can not be retrieved on their "
                         "behalf."),
                    *this->SubsystemName.ToString(),
                    CallContext->LocalUserNum)),
            TEXT(""));
        return;
    }

    // Ask the target online subsystem to convert the user ID into an FUniqueNetId for use with the avatar interface.
    auto TargetUserId = IdentityInterface->CreateUniquePlayerId(ExternalAccount.AccountId);
    if (!TargetUserId.IsValid())
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationOSSv1,
            Error,
            TEXT("Subsystem '%s' could not convert user ID '%s' into an FUniqueNetId."),
            *this->SubsystemName.ToString(),
            *ExternalAccount.AccountId);
        OnComplete.ExecuteIfBound(
            UserInfo,
            Errors::UnexpectedError(
                ANSI_TO_TCHAR(__FUNCTION__),
                FString::Printf(
                    TEXT("Subsystem '%s' could not convert user ID '%s' into an FUniqueNetId."),
                    *this->SubsystemName.ToString(),
                    *ExternalAccount.AccountId)),
            TEXT(""));
        return;
    }

    // Now call the real avatar implementation.
    FSafeOSSv1Caller::GetAvatarUrl(
        AvatarInterface.ToSharedRef(),
        *LocalUserId,
        *TargetUserId,
        TEXT(""),
        ::FOnGetAvatarUrlComplete::CreateSPLambda(
            this,
            [this, UserInfo, OnComplete](bool bWasSuccessful, FString ResultAvatarUrl) {
                OnComplete.ExecuteIfBound(
                    UserInfo,
                    bWasSuccessful ? Errors::Success()
                                   : Errors::UnexpectedError(
                                         ANSI_TO_TCHAR(__FUNCTION__),
                                         FString::Printf(
                                             TEXT("The GetAvatarUrl call on the delegated '%s' subsystem failed."),
                                             *this->SubsystemName.ToString())),
                    ResultAvatarUrl);
            }));
}

}; // namespace REDPOINT_EOS_FILE_NS_ID(2821691268,Redpoint::EOS::Platform::Integration::OSSv1::Services)

REDPOINT_EOS_CODE_GUARD_END()