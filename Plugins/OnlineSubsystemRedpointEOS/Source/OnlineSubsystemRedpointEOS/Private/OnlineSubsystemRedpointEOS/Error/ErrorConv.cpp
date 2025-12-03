// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/Error/ErrorConv.h"

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::Error
{

FOnlineError ONLINESUBSYSTEMREDPOINTEOS_API AsOSSError(const Redpoint::EOS::API::FError &InError)
{
    if (InError.WasSuccessful())
    {
        return FOnlineError::Success();
    }
    else
    {
        EOnlineErrorResult ResultCode = EOnlineErrorResult::FailExtended;
        switch (InError.ResultCode())
        {
        case EOS_EResult::EOS_NoConnection:
            ResultCode = EOnlineErrorResult::NoConnection;
            break;
        case EOS_EResult::EOS_InvalidCredentials:
            ResultCode = EOnlineErrorResult::InvalidCreds;
            break;
        case EOS_EResult::EOS_InvalidUser:
            ResultCode = EOnlineErrorResult::InvalidUser;
            break;
        case EOS_EResult::EOS_InvalidAuth:
            ResultCode = EOnlineErrorResult::InvalidAuth;
            break;
        case EOS_EResult::EOS_AccessDenied:
            ResultCode = EOnlineErrorResult::AccessDenied;
            break;
        case EOS_EResult::EOS_TooManyRequests:
            ResultCode = EOnlineErrorResult::TooManyRequests;
            break;
        case EOS_EResult::EOS_AlreadyPending:
            ResultCode = EOnlineErrorResult::AlreadyPending;
            break;
        case EOS_EResult::EOS_InvalidParameters:
            ResultCode = EOnlineErrorResult::InvalidParams;
            break;
        case EOS_EResult::EOS_InvalidRequest:
            ResultCode = EOnlineErrorResult::RequestFailure;
            break;
        case EOS_EResult::EOS_IncompatibleVersion:
            ResultCode = EOnlineErrorResult::IncompatibleVersion;
            break;
        case EOS_EResult::EOS_NotConfigured:
            ResultCode = EOnlineErrorResult::NotConfigured;
            break;
        case EOS_EResult::EOS_NotImplemented:
            ResultCode = EOnlineErrorResult::NotImplemented;
            break;
        case EOS_EResult::EOS_Canceled:
            ResultCode = EOnlineErrorResult::Canceled;
            break;
        }

        return FOnlineError::CreateError(
            InError.ErrorNamespace(),
            ResultCode,
            InError.ErrorCode(),
            FText::FromString(FString::Printf(
                TEXT("ErrorMessage='%s' OriginalCall=%s Context='%s'"),
                *InError.ErrorMessage().ToString(),
                *InError.OriginalCall(),
                *InError.Context())));
    }
}

}