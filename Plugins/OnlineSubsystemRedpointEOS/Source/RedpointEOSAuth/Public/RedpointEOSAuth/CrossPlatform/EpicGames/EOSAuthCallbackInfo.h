// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraph.h"

#include "EOSUserInterface_EnterDevicePinCode.h"
#include "RedpointEOSAPI/Auth/LinkAccount.h"
#include "RedpointEOSAPI/Auth/Login.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2758850448, Redpoint::EOS::Auth::CrossPlatform::EpicGames)
{
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::Auth::UserInterface;
using namespace ::Redpoint::EOS::API::Auth;

struct REDPOINTEOSAUTH_API FEOSAuthCallbackInfo
{
private:
    const FLogin::Result Login;
    const FLinkAccount::Result LinkAccount;
    const bool bIsLinkAccount;

public:
    FEOSAuthCallbackInfo(const FLogin::Result &InLogin);
    FEOSAuthCallbackInfo(const FLinkAccount::Result &InLinkAccount);
    UE_NONCOPYABLE(FEOSAuthCallbackInfo);

    const EOS_EResult &ResultCode() const;
    const TOptional<FPinGrantInfo> &PinGrantInfo() const;
    const EOS_EpicAccountId &LocalUserId() const;
};

}

namespace Redpoint::EOS::Auth::CrossPlatform::EpicGames
{
REDPOINT_EOS_FILE_NS_EXPORT(2758850448, Redpoint::EOS::Auth::CrossPlatform::EpicGames, FEOSAuthCallbackInfo)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()