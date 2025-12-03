// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/Nodes/SignOutEOSAccountNode.h"

#include "RedpointEOSAPI/Connect/Logout.h"
#include "RedpointEOSAuth/AuthenticationGraphState.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3268046824, Redpoint::EOS::Auth::Nodes)
{
using namespace ::Redpoint::EOS::API::Connect;

FSignOutEOSAccountNode::FSignOutEOSAccountNode(EOS_ProductUserId InLocalUserId)
    : LocalUserId(InLocalUserId)
{
}

void FSignOutEOSAccountNode::Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone)
{
    FLogout::Execute(
        State->PlatformHandle,
        FLogout::Options{this->LocalUserId},
        FLogout::CompletionDelegate::CreateLambda([OnDone](const FLogout::Result &Result) {
            OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
        }));
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()