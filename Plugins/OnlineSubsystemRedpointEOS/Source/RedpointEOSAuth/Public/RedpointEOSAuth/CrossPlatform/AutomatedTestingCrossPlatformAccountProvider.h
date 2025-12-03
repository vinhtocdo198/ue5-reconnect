// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/CrossPlatform/CrossPlatformAccountId.h"
#include "RedpointEOSAuth/CrossPlatform/CrossPlatformAccountProvider.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && (!defined(UE_BUILD_SHIPPING) || !UE_BUILD_SHIPPING)

namespace REDPOINT_EOS_FILE_NS_ID(2701208506, Redpoint::EOS::Auth::CrossPlatform)
{
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::Auth::CrossPlatform;

class REDPOINTEOSAUTH_API FAutomatedTestingCrossPlatformAccountProvider : public ICrossPlatformAccountProvider
{
public:
    FAutomatedTestingCrossPlatformAccountProvider() = default;

    virtual FName GetName() const override;
    virtual TSharedPtr<const FCrossPlatformAccountId> CreateCrossPlatformAccountId(
        const FString &InStringRepresentation) const override;
    virtual TSharedPtr<const FCrossPlatformAccountId> CreateCrossPlatformAccountId(uint8 *InBytes, int32 InSize)
        const override;
    virtual TSharedRef<FAuthenticationGraphNode> GetInteractiveAuthenticationSequence(
        bool bAllowPersistentCredentials) const override;
    virtual TSharedRef<FAuthenticationGraphNode> GetInteractiveOnlyAuthenticationSequence() const override;
    virtual TSharedRef<FAuthenticationGraphNode> GetNonInteractiveAuthenticationSequence(
        bool bOnlyUseExternalCredentials) const override;
    virtual TSharedRef<FAuthenticationGraphNode> GetUpgradeCurrentAccountToCrossPlatformAccountSequence()
        const override;
    virtual TSharedRef<FAuthenticationGraphNode> GetLinkUnusedExternalCredentialsToCrossPlatformAccountSequence()
        const override;
    virtual TSharedRef<FAuthenticationGraphNode> GetNonInteractiveDeauthenticationSequence() const override;
    virtual TSharedRef<FAuthenticationGraphNode> GetAutomatedTestingAuthenticationSequence() const override;
};

}

namespace Redpoint::EOS::Auth::CrossPlatform
{
REDPOINT_EOS_FILE_NS_EXPORT(
    2701208506,
    Redpoint::EOS::Auth::CrossPlatform,
    FAutomatedTestingCrossPlatformAccountProvider)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()