// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/Nodes/AuthenticationGraphNodeUntil_LoginComplete.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2014916715, Redpoint::EOS::Auth::Nodes)
{

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

bool FAuthenticationGraphNodeUntil_LoginComplete::Condition(const FAuthenticationGraphState &State)
{
    return State.ResultUserId.IsValid();
}

FAuthenticationGraphNodeUntil_LoginComplete::FAuthenticationGraphNodeUntil_LoginComplete(FString InErrorMessage)
    : FAuthenticationGraphNodeUntil(
          FAuthenticationGraphCondition::CreateStatic(&FAuthenticationGraphNodeUntil_LoginComplete::Condition),
          MoveTemp(InErrorMessage))
{
}

FString FAuthenticationGraphNodeUntil_LoginComplete::GetDebugName() const
{
    return TEXT("FAuthenticationGraphNodeUntil_LoginComplete");
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()