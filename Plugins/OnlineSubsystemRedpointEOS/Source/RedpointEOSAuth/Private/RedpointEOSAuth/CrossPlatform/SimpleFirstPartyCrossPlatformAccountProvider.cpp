// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/CrossPlatform/SimpleFirstPartyCrossPlatformAccountProvider.h"

#include "RedpointEOSAuth/AuthenticationGraph.h"
#include "RedpointEOSAuth/CrossPlatform/SimpleFirstParty/GetJwtForSimpleFirstPartyNode.h"
#include "RedpointEOSAuth/CrossPlatform/SimpleFirstParty/PerformOpenIdLoginForCrossPlatformFPNode.h"
#include "RedpointEOSAuth/CrossPlatform/SimpleFirstPartyCrossPlatformAccountId.h"
#include "RedpointEOSAuth/Nodes/AuthenticationGraphNodeUntil_Forever.h"
#include "RedpointEOSAuth/Nodes/FailAuthenticationNode.h"
#include "RedpointEOSAuth/Nodes/NoopAuthenticationGraphNode.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3947185440, Redpoint::EOS::Auth::CrossPlatform)
{
using namespace ::Redpoint::EOS::Auth::Nodes;
using namespace ::Redpoint::EOS::Auth::CrossPlatform::SimpleFirstParty;

FName FSimpleFirstPartyCrossPlatformAccountProvider::GetName() const
{
    return SIMPLE_FIRST_PARTY_ACCOUNT_ID;
}

TSharedPtr<const FCrossPlatformAccountId> FSimpleFirstPartyCrossPlatformAccountProvider::CreateCrossPlatformAccountId(
    const FString &InStringRepresentation) const
{
    return FSimpleFirstPartyCrossPlatformAccountId::ParseFromString(InStringRepresentation);
}

TSharedPtr<const FCrossPlatformAccountId> FSimpleFirstPartyCrossPlatformAccountProvider::CreateCrossPlatformAccountId(
    uint8 *InBytes,
    int32 InSize) const
{
    FString Data = BytesToString(InBytes, InSize);
    return FSimpleFirstPartyCrossPlatformAccountId::ParseFromString(Data);
}

TSharedRef<FAuthenticationGraphNode> FSimpleFirstPartyCrossPlatformAccountProvider::
    GetInteractiveAuthenticationSequence(bool bAllowPersistentCredentials) const
{
    return MakeShared<FAuthenticationGraphNodeUntil_Forever>()
        ->Add(MakeShared<FGetJwtForSimpleFirstPartyNode>())
        ->Add(MakeShared<FPerformOpenIdLoginForCrossPlatformFPNode>());
}

TSharedRef<FAuthenticationGraphNode> FSimpleFirstPartyCrossPlatformAccountProvider::
    GetInteractiveOnlyAuthenticationSequence() const
{
    return MakeShared<FAuthenticationGraphNodeUntil_Forever>()
        ->Add(MakeShared<FGetJwtForSimpleFirstPartyNode>())
        ->Add(MakeShared<FPerformOpenIdLoginForCrossPlatformFPNode>());
}

TSharedRef<FAuthenticationGraphNode> FSimpleFirstPartyCrossPlatformAccountProvider::
    GetNonInteractiveAuthenticationSequence(bool bOnlyUseExternalCredentials) const
{
    return MakeShared<FNoopAuthenticationGraphNode>();
}

TSharedRef<FAuthenticationGraphNode> FSimpleFirstPartyCrossPlatformAccountProvider::
    GetUpgradeCurrentAccountToCrossPlatformAccountSequence() const
{
    return MakeShared<FNoopAuthenticationGraphNode>();
}

TSharedRef<FAuthenticationGraphNode> FSimpleFirstPartyCrossPlatformAccountProvider::
    GetLinkUnusedExternalCredentialsToCrossPlatformAccountSequence() const
{
    return MakeShared<FNoopAuthenticationGraphNode>();
}

TSharedRef<FAuthenticationGraphNode> FSimpleFirstPartyCrossPlatformAccountProvider::
    GetNonInteractiveDeauthenticationSequence() const
{
    return MakeShared<FNoopAuthenticationGraphNode>();
}

TSharedRef<FAuthenticationGraphNode> FSimpleFirstPartyCrossPlatformAccountProvider::
    GetAutomatedTestingAuthenticationSequence() const
{
#if !defined(UE_BUILD_SHIPPING) || !UE_BUILD_SHIPPING
    return MakeShared<FNoopAuthenticationGraphNode>();
#else
    return MakeShared<FFailAuthenticationNode>(
        TEXT("Automated testing authentication is not supported in shipping builds."));
#endif
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()