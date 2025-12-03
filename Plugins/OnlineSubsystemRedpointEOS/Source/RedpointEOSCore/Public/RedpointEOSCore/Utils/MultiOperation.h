// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Containers/Array.h"
#include "Delegates/Delegate.h"
#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Logging.h"
#include "Templates/Function.h"
#include "Templates/SharedPointer.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3792310572, Redpoint::EOS::Core::Utils)
{
using namespace ::Redpoint::EOS::Core;

template <typename... Dummy> class FMultiOperation;

/**
 * Runs multiple asynchronous operations in parallel, where those asynchronous operations have result data that needs to
 * be aggregated.
 */
template <typename TIn, typename TOut> class FMultiOperation<TIn, TOut>
{
public:
    typedef TDelegate<void(TArray<TOut> OutValues)> FOnDone;
    typedef TDelegate<void(TOut OutValue)> FOnEntryDone;
    typedef TDelegate<bool(TIn Value, FOnEntryDone OnDone)> FInitiate;
    typedef TDelegate<bool(TArray<TIn> &InBatch, FOnDone OnDone)> FBatchedInitiate;

private:
    FMultiOperation()
        : DoneMarkers()
        , OutValues()
        , bDoneCalled(false)
        , bDisallowFinalize(false) {};

    TArray<bool> DoneMarkers;
    TArray<TOut> OutValues;
    bool bDoneCalled;
    bool bDisallowFinalize;

    void HandleDone(TSharedPtr<FMultiOperation<TIn, TOut>> Op, int i, FOnDone OnDone)
    {
        REDPOINT_EOS_USE_LLM_TAG(Core);

        Op->DoneMarkers[i] = true;
        this->CheckDone(Op, OnDone);
    };

    void CheckDone(TSharedPtr<FMultiOperation<TIn, TOut>> Op, FOnDone OnDone)
    {
        REDPOINT_EOS_USE_LLM_TAG(Core);

        if (Op->bDoneCalled)
        {
            UE_LOG(
                LogRedpointEOSCore,
                Error,
                TEXT("A call to FMultiOperation::Run had incorrect semantics. This usually means another online "
                     "subsystem is not correctly returning 'true' when it plans to later call a delegate, preventing "
                     "FMultiOperation from correctly detecting when the work is done."));
            return;
        }
        if (Op->bDisallowFinalize)
        {
            // Some OSS implementations are very poor and return false even when they have fired the delegate, which
            // results in a double call to HandleDone. bDisallowFinalize prevents CheckDone from clearing memory while
            // the initial calls are being made which makes double invocation in the case of an immediate failure safe.
            return;
        }
        bool AllDone = true;
        for (auto Marker : Op->DoneMarkers)
        {
            if (!Marker)
            {
                AllDone = false;
                break;
            }
        }
        if (AllDone)
        {
            Op->bDoneCalled = true;
            OnDone.ExecuteIfBound(Op->OutValues);
        }
    };

    static void HandleInitiateDone(TOut OutValue, TSharedPtr<FMultiOperation<TIn, TOut>> Op, int i, FOnDone OnDone)
    {
        REDPOINT_EOS_USE_LLM_TAG(Core);

        Op->OutValues[i] = OutValue;
        Op->HandleDone(Op, i, OnDone);
    }

    static void HandleBatchedInitiateDone(
        TArray<TOut> OutBatch,
        TSharedPtr<FMultiOperation<TIn, TOut>> Op,
        int i,
        FOnDone OnDone,
        int ExpectedOutSize)
    {
        REDPOINT_EOS_USE_LLM_TAG(Core);

        check(OutBatch.Num() == ExpectedOutSize /* result batch did not match expected size */);
        for (auto BOut = 0; BOut < ExpectedOutSize; BOut++)
        {
            Op->OutValues[i + BOut] = OutBatch[BOut];
            Op->DoneMarkers[i + BOut] = true;
        }
        Op->CheckDone(Op, OnDone);
    }

public:
    /**
     * For a given array of inputs, run each Initiate delegate in parallel over all of them. Once all Initiate delegates
     * either return false or invoke the OnEntryDone callback passed to them, runs the OnDone callback with the results
     * of all of the Initiate delegates.
     */
    static void Run(TArray<TIn> Inputs, FInitiate Initiate, FOnDone OnDone, TOut DefaultValue = TOut())
    {
        REDPOINT_EOS_USE_LLM_TAG(Core);

        if (Inputs.Num() == 0)
        {
            // If there are no inputs, fire the OnDone immediately.
            OnDone.ExecuteIfBound(TArray<TOut>());
            return;
        }

        TSharedPtr<FMultiOperation<TIn, TOut>> Op = MakeShareable(new FMultiOperation<TIn, TOut>());

        Op->DoneMarkers = TArray<bool>();
        Op->OutValues = TArray<TOut>();
        Op->bDisallowFinalize = true;
        for (auto i = 0; i < Inputs.Num(); i++)
        {
            Op->DoneMarkers.Add(false);
            Op->OutValues.Add(DefaultValue);
        }

        for (auto i = 0; i < Inputs.Num(); i++)
        {
            bool ShouldWait = false;
            if (Initiate.IsBound())
            {
                ShouldWait = Initiate.Execute(
                    Inputs[i],
                    FOnEntryDone::CreateStatic(&FMultiOperation::HandleInitiateDone, Op, i, OnDone));
            }
            if (!ShouldWait)
            {
                // Didn't start successfully.
                Op->HandleDone(Op, i, OnDone);
            }
        }
        Op->bDisallowFinalize = false;
        Op->CheckDone(Op, OnDone);
    }

    /**
     * For a given array of inputs, run each Initiate delegate in parallel over all of them, ensuring only BatchMaxSize
     * tasks are ever run concurrently. Once all Initiate delegates either return false or invoke the OnEntryDone
     * callback passed to them, runs the OnDone callback with the results of all of the Initiate delegates.
     */
    static void RunBatched(
        TArray<TIn> Inputs,
        int BatchMaxSize,
        FBatchedInitiate Initiate,
        FOnDone OnDone,
        TOut DefaultValue = TOut())
    {
        REDPOINT_EOS_USE_LLM_TAG(Core);

        if (Inputs.Num() == 0)
        {
            // If there are no inputs, fire the OnDone immediately.
            OnDone.ExecuteIfBound(TArray<TOut>());
            return;
        }

        TSharedPtr<FMultiOperation<TIn, TOut>> Op = MakeShareable(new FMultiOperation<TIn, TOut>());

        Op->DoneMarkers = TArray<bool>();
        Op->OutValues = TArray<TOut>();
        Op->bDisallowFinalize = true;
        for (auto i = 0; i < Inputs.Num(); i++)
        {
            Op->DoneMarkers.Add(false);
            Op->OutValues.Add(DefaultValue);
        }

        for (auto i = 0; i < Inputs.Num(); i += BatchMaxSize)
        {
            TArray<TIn> Batch;
            for (auto BIn = i; BIn < Inputs.Num() && BIn < i + BatchMaxSize; BIn++)
            {
                Batch.Add(Inputs[BIn]);
            }
            int ExpectedOutSize = Batch.Num();
            check(ExpectedOutSize > 0 && ExpectedOutSize <= BatchMaxSize);
            bool ShouldWait = false;
            if (Initiate.IsBound())
            {
                ShouldWait = Initiate.Execute(
                    Batch,
                    FOnDone::CreateStatic(&FMultiOperation::HandleBatchedInitiateDone, Op, i, OnDone, ExpectedOutSize));
            }
            if (!ShouldWait)
            {
                // Didn't start successfully.
                for (auto BOut = 0; BOut < ExpectedOutSize; BOut++)
                {
                    Op->DoneMarkers[i + BOut] = true;
                }
                Op->CheckDone(Op, OnDone);
            }
        }
        Op->bDisallowFinalize = false;
        Op->CheckDone(Op, OnDone);
    }

    /**
     * The same as Run, except that it takes in a TSharedRef as the first argument and binds the passed functions as a
     * CreateSPLambda delegate automatically.
     */
    template <typename TBinder>
    static void RunSP(
        TSharedRef<TBinder> BoundTo,
        TArray<TIn> Inputs,
        TFunction<bool(TIn Value, TFunction<void(TOut OutValue)> OnDone)> Initiate,
        TFunction<void(TArray<TOut> OutValues)> OnDone,
        TOut DefaultValue = TOut())
    {
        REDPOINT_EOS_USE_LLM_TAG(Core);

        Run(Inputs,
            FInitiate::CreateSPLambda(
                BoundTo,
                [WeakPtr = TWeakPtr<TBinder>(BoundTo),
                 Initiate](TIn Value, TDelegate<void(TOut OutValue)> CallbackOnDone) -> bool {
                    auto StrongPtr = WeakPtr.Pin();
                    if (StrongPtr.IsValid())
                    {
                        return Initiate(Value, [WeakPtr, CallbackOnDone](TOut OutValue) {
                            auto StrongPtr2 = WeakPtr.Pin();
                            if (StrongPtr2.IsValid())
                            {
                                CallbackOnDone.ExecuteIfBound(OutValue);
                            }
                        });
                    }
                    return false;
                }),
            FOnDone::CreateSPLambda(BoundTo, OnDone),
            DefaultValue);
    }

    /**
     * The same as Run, except that it takes in a TSharedFromThis as the first argument and binds the passed functions
     * as a CreateSPLambda delegate automatically.
     */
    template <typename TBinder>
    static void RunSP(
        TSharedFromThis<TBinder> *BoundTo,
        TArray<TIn> Inputs,
        TFunction<bool(TIn Value, TFunction<void(TOut OutValue)> OnDone)> Initiate,
        TFunction<void(TArray<TOut> OutValues)> OnDone,
        TOut DefaultValue = TOut())
    {
        REDPOINT_EOS_USE_LLM_TAG(Core);

        return RunSP<TBinder>(BoundTo->AsShared(), Inputs, Initiate, OnDone, DefaultValue);
    }

    /**
     * The same as Run, except that it takes in a TSharedRef as the first argument and binds the passed functions as a
     * CreateSPLambda delegate automatically.
     */
    template <typename TBinder>
    static void RunBatchedSP(
        TSharedRef<TBinder> BoundTo,
        TArray<TIn> Inputs,
        int BatchMaxSize,
        TFunction<bool(const TArray<TIn> &Values, TFunction<void(TArray<TOut> OutValue)> OnDone)> Initiate,
        TFunction<void(TArray<TOut> OutValues)> OnDone,
        TOut DefaultValue = TOut())
    {
        REDPOINT_EOS_USE_LLM_TAG(Core);

        RunBatched(
            Inputs,
            BatchMaxSize,
            FBatchedInitiate::CreateSPLambda(
                BoundTo,
                [WeakPtr = TWeakPtr<TBinder>(BoundTo),
                 Initiate](const TArray<TIn> &Values, TDelegate<void(TArray<TOut> OutValues)> CallbackOnDone) -> bool {
                    auto StrongPtr = WeakPtr.Pin();
                    if (StrongPtr.IsValid())
                    {
                        return Initiate(Values, [WeakPtr, CallbackOnDone](TArray<TOut> NestedOutValues) {
                            auto StrongPtr2 = WeakPtr.Pin();
                            if (StrongPtr2.IsValid())
                            {
                                CallbackOnDone.ExecuteIfBound(NestedOutValues);
                            }
                        });
                    }
                    return false;
                }),
            FOnDone::CreateSPLambda(BoundTo, OnDone),
            DefaultValue);
    }

    /**
     * The same as Run, except that it takes in a TSharedFromThis as the first argument and binds the passed functions
     * as a CreateSPLambda delegate automatically.
     */
    template <typename TBinder>
    static void RunBatchedSP(
        TSharedFromThis<TBinder> *BoundTo,
        TArray<TIn> Inputs,
        int BatchMaxSize,
        TFunction<bool(const TArray<TIn> &Values, TFunction<void(TArray<TOut> OutValues)> OnDone)> Initiate,
        TFunction<void(TArray<TOut> OutValues)> OnDone,
        TOut DefaultValue = TOut())
    {
        REDPOINT_EOS_USE_LLM_TAG(Core);

        return RunBatchedSP<TBinder>(BoundTo->AsShared(), Inputs, BatchMaxSize, Initiate, OnDone, DefaultValue);
    }
};

/**
 * Runs multiple asynchronous operations in parallel, where those asynchronous operations have no result (other than
 * notifying completion).
 */
template <typename TIn> class FMultiOperation<TIn>
{
public:
    typedef TDelegate<bool(TIn Value, FSimpleDelegate OnDone)> FInitiate;
    typedef TDelegate<bool(TArray<TIn> &InBatch, FSimpleDelegate OnDone)> FBatchedInitiate;

    /**
     * For a given array of inputs, run each Initiate delegate in parallel over all of them. Once all Initiate delegates
     * either return false or invoke the OnEntryDone callback passed to them, runs the OnDone callback with the results
     * of all of the Initiate delegates.
     */
    static void Run(TArray<TIn> Inputs, FInitiate Initiate, FSimpleDelegate OnDone)
    {
        REDPOINT_EOS_USE_LLM_TAG(Core);

        FMultiOperation<TIn, bool>::Run(
            Inputs,
            FMultiOperation<TIn, bool>::FInitiate::CreateLambda(
                [Initiate](TIn Value, typename FMultiOperation<TIn, bool>::FOnEntryDone OnEntryDone) -> bool {
                    if (Initiate.IsBound())
                    {
                        return Initiate.Execute(Value, FSimpleDelegate::CreateLambda([OnEntryDone]() {
                                                    OnEntryDone.ExecuteIfBound(true);
                                                }));
                    }
                    else
                    {
                        return false;
                    }
                }),
            FMultiOperation<TIn, bool>::FOnDone::CreateLambda([OnDone](TArray<bool> Unused) {
                OnDone.ExecuteIfBound();
            }));
    }

    /**
     * For a given array of inputs, run each Initiate delegate in parallel over all of them, ensuring only BatchMaxSize
     * tasks are ever run concurrently. Once all Initiate delegates either return false or invoke the OnEntryDone
     * callback passed to them, runs the OnDone callback with the results of all of the Initiate delegates.
     */
    static void RunBatched(TArray<TIn> Inputs, int BatchMaxSize, FBatchedInitiate Initiate, FSimpleDelegate OnDone)
    {
        REDPOINT_EOS_USE_LLM_TAG(Core);

        FMultiOperation<TIn, bool>::RunBatched(
            Inputs,
            BatchMaxSize,
            FMultiOperation<TIn, bool>::FBatchedInitiate::CreateLambda(
                [Initiate](TArray<TIn> &Values, typename FMultiOperation<TIn, bool>::FOnEntryDone OnEntryDone) -> bool {
                    if (Initiate.IsBound())
                    {
                        return Initiate.Execute(Values, FSimpleDelegate::CreateLambda([OnEntryDone]() {
                                                    OnEntryDone.ExecuteIfBound(true);
                                                }));
                    }
                    else
                    {
                        return false;
                    }
                }),
            FMultiOperation<TIn, bool>::FOnDone::CreateLambda([OnDone](TArray<bool> Unused) {
                OnDone.ExecuteIfBound();
            }));
    }

    /**
     * The same as Run, except that it takes in a TSharedRef as the first argument and binds the passed functions as a
     * CreateSPLambda delegate automatically.
     */
    template <typename TBinder>
    static void RunSP(
        TSharedRef<TBinder> BoundTo,
        TArray<TIn> Inputs,
        TFunction<bool(TIn Value, TFunction<void()> OnDone)> Initiate,
        TFunction<void()> OnDone)
    {
        REDPOINT_EOS_USE_LLM_TAG(Core);

        FMultiOperation<TIn, bool>::RunSP(
            BoundTo,
            Inputs,
            [Initiate](TIn Value, TFunction<void(bool)> OnEntryDone) {
                return Initiate(Value, [OnEntryDone]() {
                    OnEntryDone(true);
                });
            },
            [OnDone](TArray<bool>) {
                OnDone();
            },
            true);
    }

    /**
     * The same as Run, except that it takes in a TSharedFromThis as the first argument and binds the passed functions
     * as a CreateSPLambda delegate automatically.
     */
    template <typename TBinder>
    static void RunSP(
        TSharedFromThis<TBinder> *BoundTo,
        TArray<TIn> Inputs,
        TFunction<bool(TIn Value, TFunction<void()> OnDone)> Initiate,
        TFunction<void()> OnDone)
    {
        REDPOINT_EOS_USE_LLM_TAG(Core);

        return RunSP<TBinder>(BoundTo->AsShared(), Inputs, Initiate, OnDone);
    }

    /**
     * The same as Run, except that it takes in a TSharedRef as the first argument and binds the passed functions as a
     * CreateSPLambda delegate automatically.
     */
    template <typename TBinder>
    static void RunBatchedSP(
        TSharedRef<TBinder> BoundTo,
        TArray<TIn> Inputs,
        int BatchMaxSize,
        TFunction<bool(const TArray<TIn> &Values, TFunction<void()> OnDone)> Initiate,
        TFunction<void()> OnDone)
    {
        REDPOINT_EOS_USE_LLM_TAG(Core);

        FMultiOperation<TIn, bool>::RunBatchedSP(
            BoundTo,
            Inputs,
            BatchMaxSize,
            [Initiate](TArray<TIn> &Values, TFunction<void(bool)> OnEntryDone) {
                return Initiate(Values, [OnEntryDone]() {
                    OnEntryDone(true);
                });
            },
            [OnDone](TArray<bool>) {
                OnDone();
            },
            true);
    }

    /**
     * The same as Run, except that it takes in a TSharedFromThis as the first argument and binds the passed functions
     * as a CreateSPLambda delegate automatically.
     */
    template <typename TBinder>
    static void RunBatchedSP(
        TSharedFromThis<TBinder> *BoundTo,
        TArray<TIn> Inputs,
        int BatchMaxSize,
        TFunction<bool(const TArray<TIn> &Values, TFunction<void()> OnDone)> Initiate,
        TFunction<void()> OnDone)
    {
        REDPOINT_EOS_USE_LLM_TAG(Core);

        return RunBatchedSP<TBinder>(BoundTo->AsShared(), Inputs, BatchMaxSize, Initiate, OnDone);
    }
};

}

namespace Redpoint::EOS::Core::Utils
{
REDPOINT_EOS_FILE_NS_EXPORT(3792310572, Redpoint::EOS::Core::Utils, FMultiOperation)
}

REDPOINT_EOS_CODE_GUARD_END()