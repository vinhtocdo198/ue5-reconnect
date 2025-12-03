// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/Nodes/AuthenticationGraphNodeUntil_Forever.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2668518940, Redpoint::EOS::Auth::Nodes)
{

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

bool FAuthenticationGraphNodeUntil_Forever::Condition(const FAuthenticationGraphState &State)
{
    return false;
}

bool FAuthenticationGraphNodeUntil_Forever::RequireConditionPass() const
{
    return false;
}

FAuthenticationGraphNodeUntil_Forever::FAuthenticationGraphNodeUntil_Forever()
    : FAuthenticationGraphNodeUntil(
          FAuthenticationGraphCondition::CreateStatic(&FAuthenticationGraphNodeUntil_Forever::Condition))
{
}

FString FAuthenticationGraphNodeUntil_Forever::GetDebugName() const
{
    return TEXT("FAuthenticationGraphNodeUntil_Forever");
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()