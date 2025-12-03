// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/Nodes/PromptToSignInOrCreateAccountNode_AutomationSetting.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && (!defined(UE_BUILD_SHIPPING) || !UE_BUILD_SHIPPING)

namespace REDPOINT_EOS_FILE_NS_ID(3738989823, Redpoint::EOS::Auth::Nodes)
{

EEOSUserInterface_SignInOrCreateAccount_Choice FPromptToSignInOrCreateAccountNode_AutomationSetting::UserChoice =
    EEOSUserInterface_SignInOrCreateAccount_Choice::CreateAccount;

}

#endif

REDPOINT_EOS_CODE_GUARD_END()