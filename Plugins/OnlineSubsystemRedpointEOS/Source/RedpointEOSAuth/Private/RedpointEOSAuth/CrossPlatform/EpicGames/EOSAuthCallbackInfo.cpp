// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/CrossPlatform/EpicGames/EOSAuthCallbackInfo.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2758850448, Redpoint::EOS::Auth::CrossPlatform::EpicGames)
{

FEOSAuthCallbackInfo::FEOSAuthCallbackInfo(const FLogin::Result &InLogin)
    : Login(InLogin)
    , LinkAccount()
    , bIsLinkAccount(false)
{
}

FEOSAuthCallbackInfo::FEOSAuthCallbackInfo(const FLinkAccount::Result &InLinkAccount)
    : Login()
    , LinkAccount(InLinkAccount)
    , bIsLinkAccount(true)
{
}

const EOS_EResult &FEOSAuthCallbackInfo::ResultCode() const
{
    return this->bIsLinkAccount ? this->LinkAccount.ResultCode : this->Login.ResultCode;
}

const TOptional<FPinGrantInfo> &FEOSAuthCallbackInfo::PinGrantInfo() const
{
    return this->bIsLinkAccount ? this->LinkAccount.PinGrantInfo : this->Login.PinGrantInfo;
}

const EOS_EpicAccountId &FEOSAuthCallbackInfo::LocalUserId() const
{
    return this->bIsLinkAccount ? this->LinkAccount.LocalUserId : this->Login.LocalUserId;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()