// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "EOSUserInterface_SignInOrCreateAccount_Choice.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && (!defined(UE_BUILD_SHIPPING) || !UE_BUILD_SHIPPING)

namespace REDPOINT_EOS_FILE_NS_ID(3738989823, Redpoint::EOS::Auth::Nodes)
{

class REDPOINTEOSAUTH_API FPromptToSignInOrCreateAccountNode_AutomationSetting
{
public:
    static EEOSUserInterface_SignInOrCreateAccount_Choice UserChoice;
};

}

namespace Redpoint::EOS::Auth::Nodes
{
REDPOINT_EOS_FILE_NS_EXPORT(
    3738989823,
    Redpoint::EOS::Auth::Nodes,
    FPromptToSignInOrCreateAccountNode_AutomationSetting)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()