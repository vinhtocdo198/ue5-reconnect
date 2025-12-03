// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Auth/Login.h"
#include "RedpointEOSAuth/AuthenticationGraph.h"
#include "RedpointEOSAuth/OnlineExternalCredentials.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3364425848, Redpoint::EOS::Auth::CrossPlatform::EpicGames)
{
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::API::Auth;

class REDPOINTEOSAUTH_API FGatherEASAccountsWithExternalCredentialsNode : public FAuthenticationGraphNode
{
    typedef TTuple<EOS_EpicAccountId, EOS_ContinuanceToken> FGatherEASAccountResponse;

private:
    void OnResultCallback(
        const FLogin::Result &Data,
        bool bThisCallIsResponsibleForLogout,
        TSharedRef<FAuthenticationGraphState> State,
        TSharedRef<IOnlineExternalCredentials> ExternalCredentials,
        TFunction<void(FGatherEASAccountResponse)> OnPlatformDone);

public:
    UE_NONCOPYABLE(FGatherEASAccountsWithExternalCredentialsNode);
    FGatherEASAccountsWithExternalCredentialsNode() = default;
    virtual ~FGatherEASAccountsWithExternalCredentialsNode() override = default;

    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

    virtual FString GetDebugName() const override
    {
        return TEXT("FGatherEASAccountsWithExternalCredentialsNode");
    }
};

}

namespace Redpoint::EOS::Auth::CrossPlatform::EpicGames
{
REDPOINT_EOS_FILE_NS_EXPORT(
    3364425848,
    Redpoint::EOS::Auth::CrossPlatform::EpicGames,
    FGatherEASAccountsWithExternalCredentialsNode)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()