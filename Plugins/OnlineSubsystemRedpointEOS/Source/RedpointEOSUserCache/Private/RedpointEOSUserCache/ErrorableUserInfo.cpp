// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSUserCache/ErrorableUserInfo.h"
#include "RedpointEOSAPI/Error.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2653858584, Redpoint::EOS::UserCache)
{
using namespace ::Redpoint::EOS::API;

FErrorableUserInfo::FErrorableUserInfo()
    : UserInfo(nullptr)
    , Error(MakeShared<FError>(
          Errors::NotFound(ANSI_TO_TCHAR(__FUNCTION__), TEXT("FErrorableUserInfo initialized without value."))))
{
}

FErrorableUserInfo::FErrorableUserInfo(const FUserInfoRef &InUserInfo)
    : UserInfo(InUserInfo)
    , Error(MakeShared<FError>(Errors::Success()))
{
}

FErrorableUserInfo::FErrorableUserInfo(const FError &InError)
    : UserInfo(nullptr)
    , Error(MakeShared<FError>(InError))
{
}

bool FErrorableUserInfo::IsErrored() const
{
    return !(this->Error->WasSuccessful()) || !this->UserInfo.IsValid();
}

FUserInfoRef FErrorableUserInfo::GetUserInfo() const
{
    checkf(!this->IsErrored(), TEXT("GetUserInfo can not be called if IsErrored returns true!"));
    return this->UserInfo.ToSharedRef();
}

TSharedRef<FError> FErrorableUserInfo::GetError() const
{
    return this->Error;
}

}

REDPOINT_EOS_CODE_GUARD_END()