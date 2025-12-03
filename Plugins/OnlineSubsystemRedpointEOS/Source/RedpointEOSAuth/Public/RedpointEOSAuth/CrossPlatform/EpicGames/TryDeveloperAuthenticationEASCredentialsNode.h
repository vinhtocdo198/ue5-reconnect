// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Auth/Login.h"
#include "RedpointEOSAuth/AuthenticationGraph.h"
#include "RedpointEOSCore/Utils/ExclusiveAsyncOperationQueue.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2536339074, Redpoint::EOS::Auth::CrossPlatform::EpicGames)
{
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::API::Auth;
using namespace ::Redpoint::EOS::Core::Utils;

class REDPOINTEOSAUTH_API FTryDeveloperAuthenticationEASCredentialsNode : public FAuthenticationGraphNode
{
private:
    static TSharedRef<FExclusiveAsyncOperationQueue> LCTDeveloperOperationQueue;

    void ExecuteInOperationQueue(
        const FExclusiveAsyncOperationQueue::FOnDone &OnOperationQueueDone,
        TSharedRef<FAuthenticationGraphState> State,
        FAuthenticationGraphNodeOnDone OnDone,
        FString CredentialAddress,
        FString CredentialName);

    void OnLoginComplete(
        const FLogin::Result &Data,
        bool bThisCallIsResponsibleForLogout,
        FExclusiveAsyncOperationQueue::FOnDone OnOperationQueueDone,
        TSharedRef<FAuthenticationGraphState> State,
        FAuthenticationGraphNodeOnDone OnDone,
        FString CredentialAddress,
        FString CredentialName);

protected:
    virtual FString GetCredentialName(TSharedRef<FAuthenticationGraphState> State) = 0;

public:
    FTryDeveloperAuthenticationEASCredentialsNode() = default;
    UE_NONCOPYABLE(FTryDeveloperAuthenticationEASCredentialsNode);
    virtual ~FTryDeveloperAuthenticationEASCredentialsNode() override = default;

    static void ForceLCTDeveloperInProgressMutexReset();

    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;
};

}

namespace Redpoint::EOS::Auth::CrossPlatform::EpicGames
{
REDPOINT_EOS_FILE_NS_EXPORT(
    2536339074,
    Redpoint::EOS::Auth::CrossPlatform::EpicGames,
    FTryDeveloperAuthenticationEASCredentialsNode)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()