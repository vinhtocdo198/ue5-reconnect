// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API
{

struct REDPOINTEOSAPI_API FError
{
private:
    bool bWasSuccessfulValue;
    FString ErrorNamespaceValue;
    FString ErrorCodeValue;
    FText ErrorMessageValue;
    EOS_EResult ResultCodeValue;
    FString OriginalCallValue;
    FString ContextValue;

#ifdef REDPOINTEOSAPI_PACKAGE
public:
#else
private:
#endif
    /**
     * Do not use this constructor. Use the predefined error functions like Errors::AccessDenied() instead.
     */
    FError(
        bool bWasSuccessful,
        const FString &ErrorNamespace,
        const FString &ErrorCode,
        const FText &ErrorMessage,
        const EOS_EResult &ResultCode,
        const FString &OriginalCall,
        const FString &Context);

public:
    FError(const FError &Error);
    FError(FError &&);
    ~FError() = default;

    FError &operator=(const FError &Error);
    FError &operator=(FError &&);

    /** If true, the operation was successful. */
    FORCEINLINE bool WasSuccessful() const
    {
        return this->bWasSuccessfulValue;
    }

    /** The namespace for the error. This will always be "errors.com.redpoint.eos". */
    FORCEINLINE const FString &ErrorNamespace() const
    {
        return this->ErrorNamespaceValue;
    }

    /** The string-based error code, such as "errors.com.redpoint.eos.unexpected_error". */
    FORCEINLINE const FString &ErrorCode() const
    {
        return this->ErrorCodeValue;
    }

    /** The description of the error, as per the EOS SDK header files. */
    FORCEINLINE const FText &ErrorMessage() const
    {
        return this->ErrorMessageValue;
    }

    /** The original EOS_EResult error code for logical comparison. */
    FORCEINLINE const EOS_EResult &ResultCode() const
    {
        return this->ResultCodeValue;
    }

    /** The name of the function or original call this error occurred in. */
    FORCEINLINE const FString &OriginalCall() const
    {
        return this->OriginalCallValue;
    }

    /** The context in which the error occurred. */
    FORCEINLINE const FString &Context() const
    {
        return this->ContextValue;
    }

    /** Converts the error to a format suitable for logging. */
    FString ToLogString() const;

    /** Create a new FError with the additional context added. */
    FError WithContext(const FString &AdditionalContext) const;
};

}

REDPOINT_EOS_CODE_GUARD_END()