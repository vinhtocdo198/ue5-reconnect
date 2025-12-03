// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Delegates/Delegate.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Utils/DelegateInterceptor.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(141053841, Redpoint::EOS::Core::Utils)
{
using namespace ::Redpoint::EOS::Core::Utils;

#if UE_BUILD_SHIPPING

template <typename T> T FireOnce(const T &Delegate)
{
    return Delegate;
}

#else

template <typename DelegateSignature, typename UserPolicy> class TFireOnceInterceptor;

template <typename RetValType, typename... ParamTypes, typename UserPolicy>
class TFireOnceInterceptor<RetValType(ParamTypes...), UserPolicy>
{
private:
    bool bFired;
    FString FirstCallStack;

public:
    TFireOnceInterceptor()
        : bFired()
        , FirstCallStack()
    {
    }

    void Initialize(const TDelegate<RetValType(ParamTypes...), UserPolicy> &)
    {
    }

    bool IsSafeToExecute() const
    {
        return true;
    }

    RetValType Execute(const TDelegate<RetValType(ParamTypes...), UserPolicy> &OriginalDelegate, ParamTypes... Params)
    {
        if (!this->bFired)
        {
            this->bFired = true;

            // Capture the stack trace so we can report it later if the delegate fires a second time.
            const SIZE_T StackTraceSize = 65536;
            ANSICHAR StackTrace[StackTraceSize] = {0};
            FPlatformStackWalk::StackWalkAndDump(StackTrace, StackTraceSize, nullptr, nullptr);

            this->FirstCallStack = ANSI_TO_TCHAR(StackTrace);
        }
        else
        {
            checkf(false, TEXT("Delegate already fired with callstack:\n%s"), *this->FirstCallStack);
        }

        return OriginalDelegate.Execute(Params...);
    }

    bool ExecuteIfSafe(const TDelegate<RetValType(ParamTypes...), UserPolicy> &OriginalDelegate, ParamTypes... Params)
    {
        if (!OriginalDelegate.IsBound())
        {
            return false;
        }

        if (!this->bFired)
        {
            this->bFired = true;

            // Capture the stack trace so we can report it later if the delegate fires a second time.
            const SIZE_T StackTraceSize = 65536;
            ANSICHAR StackTrace[StackTraceSize] = {0};
            FPlatformStackWalk::StackWalkAndDump(StackTrace, StackTraceSize, nullptr, nullptr);

            this->FirstCallStack = ANSI_TO_TCHAR(StackTrace);
        }
        else
        {
            checkf(false, TEXT("Delegate already fired with callstack:\n%s"), *this->FirstCallStack);
        }

        OriginalDelegate.Execute(Params...);
        return true;
    }
};

/**
 * Wraps the specified delegate and asserts if the delegate is invoked more than once, keeping track of the callstack
 * the first time it is invoked so that the callstacks can be compared in the debugger. This should be used to
 * temporarily isolate duplicate invocations of callbacks, and removed when no longer necessary as walking the stack
 * frame on the first invocation is probably not fast.
 *
 * (FireOnce is also compiled out in Shipping builds).
 */
template <typename DelegateSignature, typename UserPolicy>
TDelegate<DelegateSignature, UserPolicy> FireOnce(const TDelegate<DelegateSignature, UserPolicy> &Delegate)
{
    return TDelegateInterceptor<TFireOnceInterceptor<DelegateSignature, UserPolicy>>::Intercept(Delegate);
}

#endif

}

namespace Redpoint::EOS::Core::Utils
{
REDPOINT_EOS_FILE_NS_EXPORT(141053841, Redpoint::EOS::Core::Utils, FireOnce)
}

REDPOINT_EOS_CODE_GUARD_END()