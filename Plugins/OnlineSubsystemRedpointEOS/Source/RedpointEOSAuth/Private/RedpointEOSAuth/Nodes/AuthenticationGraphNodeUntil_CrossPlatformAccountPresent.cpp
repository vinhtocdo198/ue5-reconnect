// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/Nodes/AuthenticationGraphNodeUntil_CrossPlatformAccountPresent.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1513208775, Redpoint::EOS::Auth::Nodes)
{

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

bool FAuthenticationGraphNodeUntil_CrossPlatformAccountPresent::Condition(const FAuthenticationGraphState &State)
{
    return State.AuthenticatedCrossPlatformAccountId.IsValid();
}

bool FAuthenticationGraphNodeUntil_CrossPlatformAccountPresent::RequireConditionPass() const
{
    return !this->AllowFailureFlag;
}

FAuthenticationGraphNodeUntil_CrossPlatformAccountPresent::FAuthenticationGraphNodeUntil_CrossPlatformAccountPresent(
    FString InErrorMessage,
    FString InEditorSignalContext,
    FString InEditorSignalId)
    : FAuthenticationGraphNodeUntil(
          FAuthenticationGraphCondition::CreateStatic(
              &FAuthenticationGraphNodeUntil_CrossPlatformAccountPresent::Condition),
          MoveTemp(InErrorMessage),
          MoveTemp(InEditorSignalContext),
          MoveTemp(InEditorSignalId))
    , AllowFailureFlag(false)
{
}

TSharedRef<FAuthenticationGraphNodeUntil_CrossPlatformAccountPresent>
FAuthenticationGraphNodeUntil_CrossPlatformAccountPresent::AllowFailure(bool Allow)
{
    this->AllowFailureFlag = Allow;
    return StaticCastSharedRef<FAuthenticationGraphNodeUntil_CrossPlatformAccountPresent>(this->AsShared());
}

FString FAuthenticationGraphNodeUntil_CrossPlatformAccountPresent::GetDebugName() const
{
    return TEXT("FAuthenticationGraphNodeUntil_CrossPlatformAccountPresent");
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()