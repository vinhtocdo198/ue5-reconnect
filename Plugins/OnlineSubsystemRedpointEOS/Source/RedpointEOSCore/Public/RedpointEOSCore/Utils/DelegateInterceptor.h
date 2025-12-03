// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Delegates/Delegate.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2193491572, Redpoint::EOS::Core::Utils)
{

namespace Private
{

#if !REDPOINT_EOS_UE_5_5_OR_LATER

template <typename DelegateBase, typename DelegateInstanceType> class TDelegateProtectedAccessor : public DelegateBase
{
public:
    template <typename... DelegateInstanceParams>
    static void AccessCreateDelegateInstance(DelegateBase &Delegate, DelegateInstanceParams &&...Params)
    {
        auto CreatePtr = &TDelegateProtectedAccessor<DelegateBase, DelegateInstanceType>::
                             template CreateDelegateInstance<DelegateInstanceType, DelegateInstanceParams...>;
        (Delegate.*CreatePtr)(Params...);
    }
};

#endif

template <typename InterceptorType, typename FuncType, typename UserPolicy, typename... VarTypes>
class TInterceptingDelegateInstance;

template <
    typename InterceptorType,
    typename RetValType,
    typename... ParamTypes,
    typename UserPolicy,
    typename... VarTypes>
class TInterceptingDelegateInstance<InterceptorType, RetValType(ParamTypes...), UserPolicy, VarTypes...>
    : public TCommonDelegateInstanceState<RetValType(ParamTypes...), UserPolicy, VarTypes...>
{
private:
    using Super = TCommonDelegateInstanceState<RetValType(ParamTypes...), UserPolicy, VarTypes...>;
    using DelegateBaseType = typename UserPolicy::FDelegateExtras;

    TSharedRef<InterceptorType> InterceptorState;
    TDelegate<RetValType(ParamTypes...)> OriginalDelegate;

public:
    template <typename... InVarTypes>
    explicit TInterceptingDelegateInstance(
        const TSharedRef<InterceptorType> &InInterceptorState,
        const TDelegate<RetValType(ParamTypes...)> &InOriginalDelegate,
        InVarTypes &&...Vars)
        : Super(Forward<InVarTypes>(Vars)...)
        , InterceptorState(InInterceptorState)
        , OriginalDelegate(InOriginalDelegate)
    {
    }

    // IDelegateInstance interface

#if USE_DELEGATE_TRYGETBOUNDFUNCTIONNAME

    FName TryGetBoundFunctionName() const final
    {
        return OriginalDelegate.TryGetBoundFunctionName();
    }

#endif

    UObject *GetUObject() const final
    {
        return OriginalDelegate.GetUObject();
    }

    const void *GetObjectForTimerManager() const final
    {
        return OriginalDelegate.GetObjectForTimerManager();
    }

    uint64 GetBoundProgramCounterForTimerManager() const final
    {
        return OriginalDelegate.GetBoundProgramCounterForTimerManager();
    }

    bool HasSameObject(const void *InContextObject) const final
    {
        return false;
    }

    bool IsCompactable() const final
    {
        return OriginalDelegate.IsCompactable();
    }

    bool IsSafeToExecute() const final
    {
        return OriginalDelegate.IsBound() && this->InterceptorState->IsSafeToExecute();
    }

public:
#if !REDPOINT_EOS_UE_5_5_OR_LATER
    void CreateCopy(TDelegateBase<FThreadSafeDelegateMode> &Base) const final
    {
        TDelegateProtectedAccessor<TDelegateBase<FThreadSafeDelegateMode>, TInterceptingDelegateInstance>::
            AccessCreateDelegateInstance(Base, *this);
    }

    void CreateCopy(TDelegateBase<FNotThreadSafeDelegateMode> &Base) const final
    {
        TDelegateProtectedAccessor<TDelegateBase<FNotThreadSafeDelegateMode>, TInterceptingDelegateInstance>::
            AccessCreateDelegateInstance(Base, *this);
    }

    void CreateCopy(TDelegateBase<FNotThreadSafeNotCheckedDelegateMode> &Base) const final
    {
        TDelegateProtectedAccessor<TDelegateBase<FNotThreadSafeNotCheckedDelegateMode>, TInterceptingDelegateInstance>::
            AccessCreateDelegateInstance(Base, *this);
    }
#else
    void CreateCopy(TDelegateBase<FThreadSafeDelegateMode> &Base) const final
    {
        new (TWriteLockedDelegateAllocation<FThreadSafeDelegateMode>{Base}) TInterceptingDelegateInstance(*this);
    }

    void CreateCopy(TDelegateBase<FNotThreadSafeDelegateMode> &Base) const final
    {
        new (TWriteLockedDelegateAllocation<FNotThreadSafeDelegateMode>{Base}) TInterceptingDelegateInstance(*this);
    }

    void CreateCopy(TDelegateBase<FNotThreadSafeNotCheckedDelegateMode> &Base) const final
    {
        new (TWriteLockedDelegateAllocation<FNotThreadSafeNotCheckedDelegateMode>{Base})
            TInterceptingDelegateInstance(*this);
    }
#endif

    RetValType Execute(ParamTypes... Params) const final
    {
        return this->InterceptorState->Execute(this->OriginalDelegate, Params...);
    }

    bool ExecuteIfSafe(ParamTypes... Params) const final
    {
        return this->InterceptorState->ExecuteIfSafe(this->OriginalDelegate, Params...);
    }
};

}

template <typename InterceptorType> class TDelegateInterceptor
{
public:
    template <typename DelegateSignature, typename UserPolicy, typename... InterceptorInitializeParameterTypes>
    static TDelegate<DelegateSignature, UserPolicy> Intercept(
        const TDelegate<DelegateSignature, UserPolicy> &Delegate,
        InterceptorInitializeParameterTypes &&...InitializeArgs);

    template <
        typename RetValType,
        typename... ParamTypes,
        typename UserPolicy,
        typename... InterceptorInitializeParameterTypes>
    static TDelegate<RetValType(ParamTypes...), UserPolicy> Intercept(
        const TDelegate<RetValType(ParamTypes...), UserPolicy> &Delegate,
        InterceptorInitializeParameterTypes &&...InitializeArgs)
    {
        auto State = MakeShared<InterceptorType>();
        State->Initialize(Delegate, Forward<InterceptorInitializeParameterTypes>(InitializeArgs)...);

#if !REDPOINT_EOS_UE_5_5_OR_LATER
        using DelegateType = TDelegate<RetValType(ParamTypes...)>;
        DelegateType Wrapper;
        Private::TDelegateProtectedAccessor<
            DelegateType,
            Private::TInterceptingDelegateInstance<InterceptorType, RetValType(ParamTypes...), UserPolicy>>::
            AccessCreateDelegateInstance(Wrapper, State, Delegate);
        return Wrapper;
#else
        TDelegate<RetValType(ParamTypes...), UserPolicy> Wrapper;
        new (TWriteLockedDelegateAllocation<typename UserPolicy::FThreadSafetyMode>{Wrapper})
            Private::TInterceptingDelegateInstance<InterceptorType, RetValType(ParamTypes...), UserPolicy>(
                State,
                Delegate);
        return Wrapper;
#endif
    }
};

}

namespace Redpoint::EOS::Core::Utils
{
REDPOINT_EOS_FILE_NS_EXPORT(2193491572, Redpoint::EOS::Core::Utils, TDelegateInterceptor)
}

REDPOINT_EOS_CODE_GUARD_END()