// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Containers/Ticker.h"
#include "Delegates/Delegate.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Utils/DelegateInterceptor.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(606199311, Redpoint::EOS::Core::Utils)
{
using namespace ::Redpoint::EOS::Core::Utils;

template <typename DelegateSignature, typename UserPolicy> class TFireWithinInterceptor;

template <typename RetValType, typename... ParamTypes, typename UserPolicy>
class TFireWithinInterceptor<RetValType(ParamTypes...), UserPolicy>
    : public TSharedFromThis<TFireWithinInterceptor<RetValType(ParamTypes...), UserPolicy>>
{
    static_assert(std::is_void_v<RetValType>, "The return value must be void to use FireWithin.");

private:
    bool bFired;
    FTSTicker::FDelegateHandle TimerHandle;

public:
    TFireWithinInterceptor()
        : bFired()
        , TimerHandle()
    {
    }

    void Initialize(
        const TDelegate<RetValType(ParamTypes...), UserPolicy> &OriginalDelegate,
        float TimeoutSeconds,
        typename std::remove_reference<ParamTypes>::type... TimeoutParameters)
    {
        this->TimerHandle = FTSTicker::GetCoreTicker().AddTicker(
            FTickerDelegate::CreateSP(this, &TFireWithinInterceptor::OnTimeout, OriginalDelegate, TimeoutParameters...),
            TimeoutSeconds);
    }

private:
    bool OnTimeout(
        float,
        TDelegate<RetValType(ParamTypes...), UserPolicy> OriginalDelegate,
        typename std::remove_reference<ParamTypes>::type... TimeoutParameters)
    {
        if (!this->bFired)
        {
            this->bFired = true;
            OriginalDelegate.ExecuteIfBound(TimeoutParameters...);
        }
        return false;
    }

public:
    bool IsSafeToExecute() const
    {
        return true;
    }

    RetValType Execute(const TDelegate<RetValType(ParamTypes...), UserPolicy> &OriginalDelegate, ParamTypes... Params)
    {
        if (!this->bFired)
        {
            this->bFired = true;
            FTSTicker::GetCoreTicker().RemoveTicker(this->TimerHandle);
            OriginalDelegate.Execute(Params...);
        }
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
            FTSTicker::GetCoreTicker().RemoveTicker(this->TimerHandle);
            OriginalDelegate.Execute(Params...);
        }

        return true;
    }
};

/**
 * Wraps the specified delegate and calls the delegate if it hasn't been called within the specified timeout.
 */
template <typename... ParamTypes, typename UserPolicy>
TDelegate<void(ParamTypes...), UserPolicy> FireWithin(
    float TimeoutSeconds,
    const TDelegate<void(ParamTypes...), UserPolicy> &Delegate,
    typename std::remove_reference<ParamTypes>::type... TimeoutParameters)
{
    return TDelegateInterceptor<TFireWithinInterceptor<void(ParamTypes...), UserPolicy>>::Intercept(
        Delegate,
        TimeoutSeconds,
        TimeoutParameters...);
}

}

namespace Redpoint::EOS::Core::Utils
{
REDPOINT_EOS_FILE_NS_EXPORT(606199311, Redpoint::EOS::Core::Utils, FireWithin)
}

REDPOINT_EOS_CODE_GUARD_END()