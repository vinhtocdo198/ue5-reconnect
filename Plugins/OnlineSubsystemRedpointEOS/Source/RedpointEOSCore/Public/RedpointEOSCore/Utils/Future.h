// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Async/Future.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Utils/FutureChainNextInfo.h"
#include "RedpointEOSCore/Utils/FutureChainThenInfo.h"
#include "Templates/EnableIf.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3660819043, Redpoint::EOS::Core::Utils)
{
using namespace ::Redpoint::EOS::API;

/**
 * Safely call a lambda that returns an immediate value, depending on whether 'this' is still a valid pointer. For
 * example:
 *
 * // Returns [](PreviousResult) -> TFuture<NextResult>
 * IfBoundValue(
 *   this,
 *   [this](PreviousResult) {
 *     return (NextResult)...;
 *   },
 *   [](PreviousResult) {
 *     return (NextResult)...;
 *   });
 *
 * If you need to return a future from the callbacks, use IfBoundAwait instead.
 */
template <typename Shared, typename Func, typename FuncNotBound>
auto IfBoundValue(TSharedRef<Shared> SP, Func ContinuationIfBound, FuncNotBound ContinuationIfNotBound)
{
    auto Wk = TWeakPtr<Shared>(SP);
    return [Wk,
            CapturedContinuationIfBound = MoveTemp(ContinuationIfBound),
            CapturedContinuationIfNotBound = MoveTemp(ContinuationIfNotBound)](auto Self) mutable {
        auto Pinned = Wk.Pin();
        if (Pinned.IsValid())
        {
            return CapturedContinuationIfBound(Self);
        }
        else
        {
            return CapturedContinuationIfNotBound(Self);
        }
    };
}

/**
 * Safely call a lambda that returns an immediate value, depending on whether 'this' is still a valid pointer. For
 * example:
 *
 * // Returns [](PreviousResult) -> TFuture<NextResult>
 * IfBoundValue(
 *   this,
 *   [this](PreviousResult) {
 *     return (NextResult)...;
 *   },
 *   [](PreviousResult) {
 *     return (NextResult)...;
 *   });
 *
 * If you need to return a future from the callbacks, use IfBoundAwait instead.
 */
template <typename Shared, typename Func, typename FuncNotBound>
auto IfBoundValue(Shared *SP, Func ContinuationIfBound, FuncNotBound ContinuationIfNotBound)
{
    static_assert(
        std::is_base_of<TSharedFromThis<Shared>, Shared>::value,
        "Expected SP to implement TSharedFromThis<>.");
    return IfBoundValue(SP->AsShared(), ContinuationIfBound, ContinuationIfNotBound);
}

/**
 * Safely call a lambda that returns another future, depending on whether 'this' is still a valid pointer. For example:
 *
 * // Returns [](PreviousResult) -> TFuture<NextResult>
 * IfBoundAwait(
 *   this,
 *   [this](PreviousResult) {
 *     return MakeFulfilledPromise<NextResult>(...).GetFuture();
 *   },
 *   [](PreviousResult) {
 *     return MakeFulfilledPromise<NextResult>(...).GetFuture();
 *   });
 */
template <typename Shared, typename Func, typename FuncNotBound>
auto IfBoundAwait(TSharedRef<Shared> SP, Func ContinuationIfBound, FuncNotBound ContinuationIfNotBound)
{
    using In = typename ::Redpoint::EOS::Core::Utils::TFutureChainNextInfo<Func>::Previous;
    using Out = typename ::Redpoint::EOS::Core::Utils::TFutureChainNextInfo<Func>::Next;

    auto Wk = TWeakPtr<Shared>(SP);
    return [Wk,
            CapturedContinuationIfBound = MoveTemp(ContinuationIfBound),
            CapturedContinuationIfNotBound = MoveTemp(ContinuationIfNotBound)](In PreviousValue) {
        auto Promise = MakeShared<TPromise<Out>>();
        auto Pinned = Wk.Pin();
        if (Pinned.IsValid())
        {
            CapturedContinuationIfBound(PreviousValue).Next([Promise](Out ResultValue) {
                Promise->SetValue(ResultValue);
            });
        }
        else
        {
            CapturedContinuationIfNotBound(PreviousValue).Next([Promise](Out ResultValue) {
                Promise->SetValue(ResultValue);
            });
        }
        return Promise->GetFuture();
    };
}

/**
 * Safely call a lambda that returns another future, depending on whether 'this' is still a valid pointer. For example:
 *
 * // Returns [](PreviousResult) -> TFuture<NextResult>
 * IfBoundAwait(
 *   this,
 *   [this](PreviousResult) {
 *     return MakeFulfilledPromise<NextResult>(...).GetFuture();
 *   },
 *   [](PreviousResult) {
 *     return MakeFulfilledPromise<NextResult>(...).GetFuture();
 *   });
 */
template <typename Shared, typename Func, typename FuncNotBound>
auto IfBoundAwait(Shared *SP, Func ContinuationIfBound, FuncNotBound ContinuationIfNotBound)
{
    static_assert(
        std::is_base_of<TSharedFromThis<Shared>, Shared>::value,
        "Expected SP to implement TSharedFromThis<>.");
    return IfBoundAwait(SP->AsShared(), ContinuationIfBound, ContinuationIfNotBound);
}

}

namespace Redpoint::EOS::Core::Utils
{
REDPOINT_EOS_FILE_NS_EXPORT(3660819043, Redpoint::EOS::Core::Utils, IfBoundValue);
REDPOINT_EOS_FILE_NS_EXPORT(3660819043, Redpoint::EOS::Core::Utils, IfBoundAwait);
}

/**
 * Chain a future with another lambda that accepts the result of the first future and returns the next future.
 */
template <
    typename Func,
    typename PreviousType,
    typename = std::enable_if<std::conjunction_v<
        std::is_invocable<Func>,
        std::is_same<PreviousType, typename ::Redpoint::EOS::Core::Utils::TFutureChainNextInfo<Func>::Previous>>>>
TFuture<typename ::Redpoint::EOS::Core::Utils::TFutureChainNextInfo<Func>::Next> operator+(
    TFuture<PreviousType> &&PreviousFuture,
    Func Continuation)
{
    auto ChainingPromise =
        MakeShared<TPromise<typename ::Redpoint::EOS::Core::Utils::TFutureChainNextInfo<Func>::Next>>();
    PreviousFuture.Next([ChainingPromise, CapturedContinuation = MoveTemp(Continuation)](auto PreviousValue) {
        CapturedContinuation(PreviousValue).Next([ChainingPromise](auto NextValue) {
            ChainingPromise->SetValue(NextValue);
        });
    });
    return ChainingPromise->GetFuture();
}

/**
 * Chain a future with another lambda that accepts the first future and returns the next future.
 */
template <
    typename Func,
    typename PreviousType,
    typename = std::enable_if<std::conjunction_v<
        std::is_invocable<Func>,
        std::is_same<PreviousType, typename ::Redpoint::EOS::Core::Utils::TFutureChainThenInfo<Func>::Previous>>>>
TFuture<typename ::Redpoint::EOS::Core::Utils::TFutureChainThenInfo<Func>::Next> operator*(
    TFuture<PreviousType> &&PreviousFuture,
    Func Continuation)
{
    auto ChainingPromise =
        MakeShared<TPromise<typename ::Redpoint::EOS::Core::Utils::TFutureChainThenInfo<Func>::Next>>();
    PreviousFuture.Then([ChainingPromise, CapturedContinuation = MoveTemp(Continuation)](auto PreviousChainedFuture) {
        CapturedContinuation(MoveTemp(PreviousChainedFuture)).Next([ChainingPromise](auto NextValue) {
            ChainingPromise->SetValue(NextValue);
        });
    });
    return ChainingPromise->GetFuture();
}

REDPOINT_EOS_CODE_GUARD_END()
