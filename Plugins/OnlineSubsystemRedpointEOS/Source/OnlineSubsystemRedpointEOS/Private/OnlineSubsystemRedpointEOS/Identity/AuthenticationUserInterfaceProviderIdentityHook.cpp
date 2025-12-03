// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/Identity/AuthenticationUserInterfaceProviderIdentityHook.h"

#include "OnlineSubsystemRedpointEOS/Authentication/EOSSubsystemAuthenticationGraphUserInterface.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(19988241, Redpoint::EOS::OnlineSubsystemRedpointEOS::Identity)
{
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::Auth;

TSharedPtr<IAuthenticationGraphUserInterface> FAuthenticationUserInterfaceProviderIdentityHook::
    OnAuthenticationUserInterfaceRequired(const FIdentityHookContext &InContext)
{
    return MakeShared<
        Redpoint::EOS::OnlineSubsystemRedpointEOS::Authentication::FEOSSubsystemAuthenticationGraphUserInterface>();
}

}

#endif