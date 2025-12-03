// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/CrossPlatform/EpicGames/TryDefaultDeveloperAuthenticationEASCredentialsNode.h"

#include "RedpointEOSAuth/AuthenticationGraphState.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(699046853, Redpoint::EOS::Auth::CrossPlatform::EpicGames)
{

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

FString FTryDefaultDeveloperAuthenticationEASCredentialsNode::GetCredentialName(
    TSharedRef<FAuthenticationGraphState> State)
{
    return State->Config->GetDeveloperToolDefaultCredentialName();
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()