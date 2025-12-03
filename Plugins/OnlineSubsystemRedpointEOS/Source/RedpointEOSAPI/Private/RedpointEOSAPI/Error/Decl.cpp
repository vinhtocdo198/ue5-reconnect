// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Error/Decl.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API
{

FError::FError(
    bool bInWasSuccessful,
    const FString &InErrorNamespace,
    const FString &InErrorCode,
    const FText &InErrorMessage,
    const EOS_EResult &InResultCode,
    const FString &InOriginalCall,
    const FString &InContext)
    : bWasSuccessfulValue(bInWasSuccessful)
    , ErrorNamespaceValue(InErrorNamespace)
    , ErrorCodeValue(InErrorCode)
    , ErrorMessageValue(InErrorMessage)
    , ResultCodeValue(InResultCode)
    , OriginalCallValue(InOriginalCall)
    , ContextValue(InContext)
{
}

FError::FError(const FError &InError)
    : bWasSuccessfulValue(InError.bWasSuccessfulValue)
    , ErrorNamespaceValue(InError.ErrorNamespaceValue)
    , ErrorCodeValue(InError.ErrorCodeValue)
    , ErrorMessageValue(InError.ErrorMessageValue)
    , ResultCodeValue(InError.ResultCodeValue)
    , OriginalCallValue(InError.OriginalCallValue)
    , ContextValue(InError.ContextValue)
{
}

// @note: This move constructor doesn't do anything to the source value, since there's no valid way to reset it to an
// "empty" state.
FError::FError(FError &&InError)
    : bWasSuccessfulValue(InError.bWasSuccessfulValue)
    , ErrorNamespaceValue(InError.ErrorNamespaceValue)
    , ErrorCodeValue(InError.ErrorCodeValue)
    , ErrorMessageValue(InError.ErrorMessageValue)
    , ResultCodeValue(InError.ResultCodeValue)
    , OriginalCallValue(InError.OriginalCallValue)
    , ContextValue(InError.ContextValue)
{
}

FError &FError::operator=(const FError &InError)
{
    this->bWasSuccessfulValue = InError.bWasSuccessfulValue;
    this->ErrorNamespaceValue = InError.ErrorNamespaceValue;
    this->ErrorCodeValue = InError.ErrorCodeValue;
    this->ErrorMessageValue = InError.ErrorMessageValue;
    this->ResultCodeValue = InError.ResultCodeValue;
    this->OriginalCallValue = InError.OriginalCallValue;
    this->ContextValue = InError.ContextValue;
    return *this;
}

// @note: This move constructor doesn't do anything to the source value, since there's no valid way to reset it to an
// "empty" state.
FError &FError::operator=(FError &&InError)
{
    this->bWasSuccessfulValue = InError.bWasSuccessfulValue;
    this->ErrorNamespaceValue = InError.ErrorNamespaceValue;
    this->ErrorCodeValue = InError.ErrorCodeValue;
    this->ErrorMessageValue = InError.ErrorMessageValue;
    this->ResultCodeValue = InError.ResultCodeValue;
    this->OriginalCallValue = InError.OriginalCallValue;
    this->ContextValue = InError.ContextValue;
    return *this;
}

FString FError::ToLogString() const
{
    return FString::Printf(
        TEXT("ErrorNamespace=%s ErrorCode=%s ErrorMessage='%s' OriginalCall=%s Context='%s'"),
        *this->ErrorNamespaceValue,
        *this->ErrorCodeValue,
        *this->ErrorMessageValue.ToString(),
        *this->OriginalCallValue,
        *this->ContextValue);
}

FError FError::WithContext(const FString &AdditionalContext) const
{
    return FError(
        this->bWasSuccessfulValue,
        this->ErrorNamespaceValue,
        this->ErrorCodeValue,
        this->ErrorMessageValue,
        this->ResultCodeValue,
        this->OriginalCallValue,
        this->ContextValue.IsEmpty() ? AdditionalContext
                                     : FString::Printf(TEXT("%s; %s"), *AdditionalContext, *this->ContextValue));
}

}

REDPOINT_EOS_CODE_GUARD_END()