// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationMeta/Auth/MetaExternalCredentials.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_META_ENABLED
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAuth/AuthenticationConstants.h"
#include "RedpointEOSAuth/CrossPlatform/CrossPlatformAccountId.h"
#include "RedpointEOSPlatformIntegrationMeta/Logging.h"
#include "RedpointEOSPlatformIntegrationMeta/MetaSDK.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_META_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(3113027389, Redpoint::EOS::Platform::Integration::Meta::Auth)
{
using namespace ::Redpoint::EOS::Platform::Integration::Meta;
using namespace ::Redpoint::EOS::API;

FMetaExternalCredentials::FMetaExternalCredentials(
    const FString &InProofToken,
    const FString &InDisplayName,
    const TMap<FString, FString> &InAuthAttributes,
    const FString &InUserId)
{
    this->ProofToken = InProofToken;
    this->DisplayName = InDisplayName;
    this->AuthAttributes = InAuthAttributes;
    this->UserId = InUserId;
}

FText FMetaExternalCredentials::GetProviderDisplayName() const
{
    return NSLOCTEXT("OnlineSubsystemRedpointEOS", "Platform_Meta", "Meta");
}

FString FMetaExternalCredentials::GetType() const
{
    return TEXT("OCULUS_USERID_NONCE");
}

FString FMetaExternalCredentials::GetId() const
{
    if (this->DisplayName.IsEmpty())
    {
        return TEXT("Anonymous");
    }

    return this->DisplayName;
}

FString FMetaExternalCredentials::GetToken() const
{
    return this->ProofToken;
}

ESupportedLinkType FMetaExternalCredentials::GetProviderSupportedLinkType(FName PlatformName) const
{
    if (PlatformName == FName(EPIC_GAMES_ACCOUNT_ID))
    {
        return ESupportedLinkType::PersistentAuth;
    }
    else
    {
        return ESupportedLinkType::None;
    }
}

void FMetaExternalCredentials::StoreData(const FString &Key, const FString &Data) const
{
    FString Path = FPaths::ProjectPersistentDownloadDir() / FString::Printf(TEXT("%s_%s"), *this->UserId, *Key);

    UE_LOG(LogRedpointEOSPlatformIntegrationMeta, Verbose, TEXT("Saving refresh token at path: %s"), *Path);
    FFileHelper::SaveStringToFile(Data, *Path);
}

TOptional<FString> FMetaExternalCredentials::ReadData(const FString &Key) const
{
    FString Path = FPaths::ProjectPersistentDownloadDir() / FString::Printf(TEXT("%s_%s"), *this->UserId, *Key);

    FString OutFileValue;
    if (FFileHelper::LoadFileToString(OutFileValue, *Path))
    {
        UE_LOG(LogRedpointEOSPlatformIntegrationMeta, Verbose, TEXT("Read refresh token from path: %s"), *Path);
        return OutFileValue;
    }
    else
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationMeta,
            Verbose,
            TEXT("Unable to read refresh token from path: %s"),
            *Path);
        return TOptional<FString>();
    }
}

TMap<FString, FString> FMetaExternalCredentials::GetAuthAttributes() const
{
    return this->AuthAttributes;
}

FName FMetaExternalCredentials::GetNativeSubsystemName() const
{
    return NAME_None;
}

void FMetaExternalCredentials::Refresh(
    TSoftObjectPtr<UWorld> InWorld,
    int32 LocalUserNum,
    FOnlineExternalCredentialsRefreshComplete OnComplete)
{
    FMetaSDK::GetLoggedInUser(FMetaSDK::FOnGetLoggedInUserComplete::CreateSPLambda(
        this,
        [this, OnComplete](const FError &UserErrorCode, const TOptional<ovrUserHandle> &UserHandle) {
            if (!UserErrorCode.WasSuccessful())
            {
                UE_LOG(
                    LogRedpointEOSPlatformIntegrationMeta,
                    Error,
                    TEXT("Unable to refresh Meta Quest credentials: %s"),
                    *UserErrorCode.ToLogString());
                OnComplete.ExecuteIfBound(false);
                return;
            }

            auto OvrUserId = ovr_User_GetID(UserHandle.GetValue());

            FMetaSDK::GetIsViewerEntitled(FMetaSDK::FOnGetIsViewerEntitledComplete::CreateSPLambda(
                this,
                [this, OvrUserId, OnComplete](const FError &EntitlementErrorCode) {
                    if (!EntitlementErrorCode.WasSuccessful())
                    {
                        UE_LOG(
                            LogRedpointEOSPlatformIntegrationMeta,
                            Error,
                            TEXT("The current user is not entitled to use this Meta Quest app: %s"),
                            *EntitlementErrorCode.ToLogString());
                        OnComplete.ExecuteIfBound(false);
                        return;
                    }

                    FMetaSDK::GetUserProof(FMetaSDK::FOnGetUserProofComplete::CreateSPLambda(
                        this,
                        [this,
                         OvrUserId,
                         OnComplete](const FError &ProofErrorCode, const TOptional<FString> &ProofNonce) {
                            if (!ProofErrorCode.WasSuccessful())
                            {
                                UE_LOG(
                                    LogRedpointEOSPlatformIntegrationMeta,
                                    Error,
                                    TEXT("Unable to refresh Meta Quest credentials: %s"),
                                    *ProofErrorCode.ToLogString());
                                OnComplete.ExecuteIfBound(false);
                                return;
                            }

                            TMap<FString, FString> MetaAttrs;
                            MetaAttrs.Add(EOS_AUTH_ATTRIBUTE_AUTHENTICATEDWITH, TEXT("meta"));
                            MetaAttrs.Add(TEXT("meta.userId"), FString::Printf(TEXT("%llu"), OvrUserId));
                            MetaAttrs.Add(TEXT("meta.proofNonce"), ProofNonce.GetValue());
                            // deprecated
                            MetaAttrs.Add(TEXT("oculus.userId"), FString::Printf(TEXT("%llu"), OvrUserId));
                            MetaAttrs.Add(TEXT("oculus.proofNonce"), ProofNonce.GetValue());

                            this->ProofToken = FString::Printf(TEXT("%llu|%s"), OvrUserId, *ProofNonce.GetValue());
                            this->AuthAttributes = MetaAttrs;
                            this->UserId = FString::Printf(TEXT("%llu"), OvrUserId);

                            UE_LOG(
                                LogRedpointEOSPlatformIntegrationMeta,
                                Verbose,
                                TEXT("Obtained refresh Meta user proof token: %s"),
                                *this->ProofToken);
                            OnComplete.ExecuteIfBound(true);
                        }));
                }));
        }));
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()