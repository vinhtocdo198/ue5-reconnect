// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/CrossPlatform/AutomatedTesting/EmitLogForAutomatedTestingNode.h"

#include "RedpointEOSAuth/Logging.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && (!defined(UE_BUILD_SHIPPING) || !UE_BUILD_SHIPPING)

namespace REDPOINT_EOS_FILE_NS_ID(1284975154, Redpoint::EOS::Auth::CrossPlatform::AutomatedTesting)
{
using namespace ::Redpoint::EOS::Auth;

FEmitLogForAutomatedTestingNode::FEmitLogForAutomatedTestingNode(const FString &InLog)
{
    this->Log = InLog;
}

void FEmitLogForAutomatedTestingNode::Execute(
    TSharedRef<FAuthenticationGraphState> State,
    FAuthenticationGraphNodeOnDone OnDone)
{
    // These are errors so that unexpected logs in the authentication graph flow will cause the unit tests to fail.
    UE_LOG(LogRedpointEOSAuth, Error, TEXT("%s"), *this->Log);
    OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()