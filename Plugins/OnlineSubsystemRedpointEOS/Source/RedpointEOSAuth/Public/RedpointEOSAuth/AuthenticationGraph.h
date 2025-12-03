// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraphNode.h"
#include "RedpointEOSAuth/AuthenticationGraphOnDone.h"
#include "RedpointEOSAuth/ForwardDecls.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Utils/AsyncTaskGraph.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(591988451, Redpoint::EOS::Auth)
{
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::Core::Utils;

class REDPOINTEOSAUTH_API FAuthenticationGraph : public TSharedFromThis<FAuthenticationGraph>
{
    friend FAuthenticationGraphNode;

protected:
    virtual TSharedRef<FAuthenticationGraphNode> CreateGraph(
        const TSharedRef<FAuthenticationGraphState> &InitialState) const = 0;

public:
    static bool Condition_Unauthenticated(const FAuthenticationGraphState &State);
    static bool Condition_HasCrossPlatformAccountProvider(const FAuthenticationGraphState &State);
    static bool Condition_RequireCrossPlatformAccount(const FAuthenticationGraphState &State);
    static bool Condition_CrossPlatformAccountIsValid(const FAuthenticationGraphState &State);
    static bool Condition_CrossPlatformAccountIsValidWithPUID(const FAuthenticationGraphState &State);
    static bool Condition_OneSuccessfulCandidate(const FAuthenticationGraphState &State);
    static bool Condition_MoreThanOneSuccessfulCandidate(const FAuthenticationGraphState &State);
    static bool Condition_NoSuccessfulCandidate_AtLeastOneContinuanceToken(const FAuthenticationGraphState &State);
    static bool Condition_NoSuccessfulCandidate_NoContinuanceToken(const FAuthenticationGraphState &State);
    static bool Condition_CrossPlatformProvidedContinuanceToken(const FAuthenticationGraphState &State);

    void Execute(const TSharedRef<FAuthenticationGraphState> &State, const FAuthenticationGraphOnDone &OnDone) const;

    FAuthenticationGraph() = default;
    UE_NONCOPYABLE(FAuthenticationGraph);
    virtual ~FAuthenticationGraph() = default;

private:
    TSharedRef<FAsyncTaskGraph> ConstructExecutor(
        TStatId StatId,
        const TSharedRef<FAuthenticationGraphState> &ResultingState,
        bool bAddLogoutTasks) const;

    void OnExecuteDone(
        EAuthenticationGraphNodeResult Result,
        TSharedRef<FAuthenticationGraphState> ResultingState,
        TSharedRef<FAuthenticationGraphNodeCollector> Collector,
        TSharedRef<FAuthenticationGraphNode> StartingNode,
        FAuthenticationGraphOnDone ExecuteOnDone) const;
};

}

namespace Redpoint::EOS::Auth
{
REDPOINT_EOS_FILE_NS_EXPORT(591988451, Redpoint::EOS::Auth, FAuthenticationGraph)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()