// Copyright June Rhodes. All Rights Reserved.

#pragma once

// MSVC requires extra coroutine_traits for lambdas that don't capture, where the lambda type is not passed as a
// parameter to the coroutine traits type. From testing however, lambdas that do capture still route to the full
// coroutine traits that receive the lambda type, allowing us to verify lack of captures via static_assert.
#if !defined(__clang__) && defined(_MSC_VER)
#define REDPOINT_EOS_ASYNC_INCLUDE_EXTRA_MSVC_LAMBDA_TRAITS 1
#else
#define REDPOINT_EOS_ASYNC_INCLUDE_EXTRA_MSVC_LAMBDA_TRAITS 0
#endif

#include "Async/Future.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#if __has_include(<coroutine>)
#include <coroutine>
#define REDPOINT_EOS_ASYNC_COROUTINE_TRAITS_NAMESPACE std
#elif __has_include(<experimental/coroutine>)
#include <experimental/coroutine>
namespace std
{
using namespace std::experimental;
}
#define REDPOINT_EOS_ASYNC_COROUTINE_TRAITS_NAMESPACE std::experimental
#elif defined(__clang__) && defined(__clang_major__) && __clang_major__ >= 18
#include "RedpointEOSAsync/ClangPolyfill_coroutine.h"
#define REDPOINT_EOS_ASYNC_COROUTINE_TRAITS_NAMESPACE std
#else
#error You're not compiling with Clang 18.x or later and the standard library include path is missing the <coroutine> header.
#endif
#include <type_traits>

REDPOINT_EOS_CODE_GUARD_BEGIN()

class FAutomationTestBase;

namespace REDPOINT_EOS_FILE_NS_ID(4071280369, Redpoint::EOS::Async)
{

enum class ETaskBinding : uint8
{
    // The task is not bound and is the return value of a static member or global function.
    Static,

    // The task is returned from a lambda which must not capture.
    Lambda
};

struct __DO_NOT_USE_EXTERNALLY_PrivateToken
{
    explicit __DO_NOT_USE_EXTERNALLY_PrivateToken() = default;
};

template <typename ResultType> struct TCallbackFunctionType
{
    using FunctionType = TFunction<void(ResultType)>;
};

template <> struct TCallbackFunctionType<void>
{
    using FunctionType = TFunction<void()>;
};

template <typename ResultType> class TTask;

template <typename ResultType>
void AsCallback(TTask<ResultType> &&Task, typename TCallbackFunctionType<ResultType>::FunctionType Callback);

template <typename ResultType> class TTaskState
{
public:
    TPromise<ResultType> Promise;
    TFuture<ResultType> Future;
    bool bIsFutureLinkedToPromise;

    explicit TTaskState()
        : Promise()
        , Future(this->Promise.GetFuture())
        , bIsFutureLinkedToPromise(true)
    {
    }
    UE_NONCOPYABLE(TTaskState);
    virtual ~TTaskState() = default;

    virtual bool IsBound()
    {
        return true;
    }

    void ReplaceFuture(TFuture<ResultType> &&NewFuture)
    {
        this->Future = MoveTemp(NewFuture);
        this->bIsFutureLinkedToPromise = false;
    }
};

template <class ClassType>
concept IsUObject = std::is_base_of<UObject, ClassType>::value;

template <class ClassType>
concept IsSharedFromThis = std::is_base_of<TSharedFromThis<ClassType>, ClassType>::value;

template <typename ResultType, typename ClassType> class TTaskStateObjectBound;

template <typename ResultType, IsSharedFromThis ClassType>
class TTaskStateObjectBound<ResultType, ClassType> : public TTaskState<ResultType>
{
public:
    TWeakPtr<ClassType> Object;

    explicit TTaskStateObjectBound(const TSharedRef<ClassType> &InObject)
        : TTaskState<ResultType>()
        , Object(InObject)
    {
    }
    UE_NONCOPYABLE(TTaskStateObjectBound);
    virtual ~TTaskStateObjectBound() override = default;

    virtual bool IsBound() override
    {
        auto Pinned = Object.Pin();
        return Pinned.IsValid();
    }
};

template <typename ResultType, IsUObject ClassType>
class TTaskStateObjectBound<ResultType, ClassType> : public TTaskState<ResultType>
{
public:
    TWeakObjectPtr<const ClassType> Object;

    TTaskStateObjectBound(const ClassType *InObject)
        : TTaskState<ResultType>()
        , Object(InObject)
    {
    }
    UE_NONCOPYABLE(TTaskStateObjectBound);
    virtual ~TTaskStateObjectBound() override = default;

    virtual bool IsBound() override
    {
        return Object.IsValid();
    }
};

template <typename ResultType> class TDeferredTask;

template <typename ResultType, ETaskBinding Binding> class TTaskWith;

template <typename ResultType> class TTask
{
protected:
    TSharedPtr<TTaskState<ResultType>> State;

public:
    explicit TTask(__DO_NOT_USE_EXTERNALLY_PrivateToken, const TSharedRef<TTaskState<ResultType>> &InState)
        : State(InState)
    {
    }
    TTask(const TTask &) = default;
    TTask(TTask &&) = default;
    ~TTask() = default;

    static TDeferredTask<ResultType> Deferred();

    static TTask<ResultType> FromResult(const ResultType &Value);

    static TTask<ResultType> FromFuture(TFuture<ResultType> &&Future);

    bool await_ready() const
    {
        // @todo: Can we eliminate the heap allocation if the coroutine immediately has a result with no suspension?
        // From the C++20 docs it sounds like the coroutine state is allocated on the heap regardless, so maybe it's not
        // worth doing at this level.
        checkf(this->State.IsValid(), TEXT("Expected TTask state object to be valid when await_ready is called!"));
        checkf(this->State->Future.IsValid(), TEXT("Expected future to be valid when await_ready is called!"));
        return this->State->Future.IsReady();
    }

    void await_suspend(std::coroutine_handle<> h) const
    {
        checkf(this->State->Future.IsValid(), TEXT("Expected future to be valid when await_suspend is called!"));
        this->State->Future.Then([StatePtr = this->State, h](TFuture<ResultType> &&ReturningFuture)
#if PLATFORM_ANDROID
                                     mutable
#endif
                                 {
                                     StatePtr->ReplaceFuture(MoveTemp(ReturningFuture));
                                     if (StatePtr->IsBound())
                                     {
                                         // @note: Compatibility with macOS toolchain on Unreal Engine 5.4 and earlier,
                                         // where resume() is not declared const.
                                         (*const_cast<std::coroutine_handle<> *>(&h)).resume();
                                     }
                                 });
    }

    auto await_resume() const
    {
        checkf(this->State->Future.IsValid(), TEXT("Expected future to be valid when await_resume is called!"));
#if REDPOINT_EOS_UE_5_4_OR_LATER
        return this->State->Future.Consume();
#else
        return this->State->Future.Get();
#endif
    }

    bool IsBound() const
    {
        return this->State.IsValid() && this->State->IsBound();
    }

    bool IsReady() const
    {
        return this->State.IsValid() && this->State->IsBound() && this->State->Future.IsValid() &&
               this->State->Future.IsReady();
    }

    template <typename RR>
    friend void AsCallback(TTask<RR> &&Task, typename TCallbackFunctionType<RR>::FunctionType Callback);

    template <ETaskBinding Binding> using With = TTaskWith<ResultType, Binding>;
};

template <typename ResultType> class TDeferredTask : public TTask<ResultType>
{
    friend TTask<ResultType>;

private:
    TDeferredTask()
        : TTask<ResultType>(__DO_NOT_USE_EXTERNALLY_PrivateToken{}, MakeShareable(new TTaskState<ResultType>()))
    {
    }

public:
    TDeferredTask(const TDeferredTask &) = default;
    TDeferredTask(TDeferredTask &&) = default;
    ~TDeferredTask() = default;

    void SetValue(ResultType Result) const
    {
        checkf(
            this->State->bIsFutureLinkedToPromise,
            TEXT("Expected future to still be linked to promise when SetValue is called on a TDeferredTask."));
        this->State->Promise.SetValue(Result);
    }
};

template <typename ResultType> TDeferredTask<ResultType> TTask<ResultType>::Deferred()
{
    return TDeferredTask<ResultType>();
}

template <typename ResultType> TTask<ResultType> TTask<ResultType>::FromResult(const ResultType &Value)
{
    auto Task = TTask<ResultType>(__DO_NOT_USE_EXTERNALLY_PrivateToken{}, MakeShareable(new TTaskState<ResultType>()));
    Task.State->Promise.SetValue(Value);
    return Task;
}

template <typename ResultType> TTask<ResultType> TTask<ResultType>::FromFuture(TFuture<ResultType> &&Future)
{
    auto Deferred = TTask<ResultType>::Deferred();
    Future.Then([Deferred](TFuture<ResultType> &&ReturningFuture)
#if PLATFORM_ANDROID
                    mutable
#endif
                {
#if REDPOINT_EOS_UE_5_4_OR_LATER
                    Deferred.SetValue(ReturningFuture.Consume());
#else
                    Deferred.SetValue(ReturningFuture.Get());
#endif
                });
    return Deferred;
}

template <> class TDeferredTask<void>;

template <> class TTask<void>
{
protected:
    TSharedPtr<TTaskState<void>> State;

public:
    explicit TTask(__DO_NOT_USE_EXTERNALLY_PrivateToken, const TSharedRef<TTaskState<void>> &InState)
        : State(InState)
    {
    }
    TTask(const TTask &) = default;
    TTask(TTask &&) = default;
    ~TTask() = default;

    FORCEINLINE static TDeferredTask<void> Deferred();

    FORCEINLINE static TTask<void> FromResult();

    FORCEINLINE static TTask<void> FromFuture(TFuture<void> &&Future);

    FORCEINLINE bool await_ready() const
    {
        // @todo: Can we eliminate the heap allocation if the coroutine immediately has a result with no suspension?
        // From the C++20 docs it sounds like the coroutine state is allocated on the heap regardless, so maybe it's not
        // worth doing at this level.
        checkf(this->State.IsValid(), TEXT("Expected TTask state object to be valid when await_ready is called!"));
        checkf(this->State->Future.IsValid(), TEXT("Expected future to be valid when await_ready is called!"));
        return this->State->Future.IsReady();
    }

    FORCEINLINE void await_suspend(std::coroutine_handle<> h) const
    {
        checkf(this->State->Future.IsValid(), TEXT("Expected future to be valid when await_suspend is called!"));
#if REDPOINT_EOS_UE_5_6_OR_LATER
        this->State->Future.Then([StatePtr = this->State, h](TFuture<void> &&ReturningFuture)
#else
        this->State->Future.Then([StatePtr = this->State, h](TFuture<int> &&ReturningFuture)
#endif
#if PLATFORM_ANDROID
                                     mutable
#endif
                                 {
                                     TPromise<void> P;
                                     P.SetValue();
                                     StatePtr->ReplaceFuture(P.GetFuture());
                                     if (StatePtr->IsBound())
                                     {
                                         // @note: Compatibility with macOS toolchain on Unreal Engine 5.4 and earlier,
                                         // where resume() is not declared const.
                                         (*const_cast<std::coroutine_handle<> *>(&h)).resume();
                                     }
                                 });
    }

    FORCEINLINE void await_resume() const
    {
        checkf(this->State->Future.IsValid(), TEXT("Expected future to be valid when await_resume is called!"));
    }

    bool IsBound() const
    {
        return this->State.IsValid() && this->State->IsBound();
    }

    bool IsReady() const
    {
        return this->State.IsValid() && this->State->IsBound() && this->State->Future.IsValid() &&
               this->State->Future.IsReady();
    }

    template <typename RR>
    friend void AsCallback(TTask<RR> &&Task, typename TCallbackFunctionType<RR>::FunctionType Callback);

    template <ETaskBinding Binding> using With = TTaskWith<void, Binding>;
};

template <typename ResultType, ETaskBinding Binding> class TTaskWith : public TTask<ResultType>
{
    friend TTask<ResultType>;

public:
    TTaskWith(__DO_NOT_USE_EXTERNALLY_PrivateToken, const TSharedRef<TTaskState<ResultType>> &InState)
        : TTask<ResultType>(__DO_NOT_USE_EXTERNALLY_PrivateToken{}, InState)
    {
    }
    TTaskWith(const TTaskWith &) = default;
    TTaskWith(TTaskWith &&) = default;
    ~TTaskWith() = default;
};

template <> class TDeferredTask<void> : public TTask<void>
{
    friend TTask<void>;

private:
    TDeferredTask()
        : TTask<void>(__DO_NOT_USE_EXTERNALLY_PrivateToken{}, MakeShareable(new TTaskState<void>()))
    {
    }

public:
    TDeferredTask(const TDeferredTask &) = default;
    TDeferredTask(TDeferredTask &&) = default;
    ~TDeferredTask() = default;

    FORCEINLINE void SetValue() const
    {
        checkf(
            this->State->bIsFutureLinkedToPromise,
            TEXT("Expected future to still be linked to promise when SetValue is called on a TDeferredTask."));
        this->State->Promise.SetValue();
    }
};

FORCEINLINE TDeferredTask<void> TTask<void>::Deferred()
{
    return TDeferredTask<void>();
}

FORCEINLINE TTask<void> TTask<void>::FromResult()
{
    auto Task = TTask<void>(__DO_NOT_USE_EXTERNALLY_PrivateToken{}, MakeShareable(new TTaskState<void>()));
    Task.State->Promise.SetValue();
    return Task;
}

FORCEINLINE TTask<void> TTask<void>::FromFuture(TFuture<void> &&Future)
{
    auto Deferred = TTask<void>::Deferred();
#if REDPOINT_EOS_UE_5_6_OR_LATER
    Future.Then([Deferred](TFuture<void> &&ReturningFuture)
#else
    Future.Then([Deferred](TFuture<int> &&ReturningFuture)
#endif
#if PLATFORM_ANDROID
                    mutable
#endif
                {
#if REDPOINT_EOS_UE_5_4_OR_LATER
                    ReturningFuture.Consume();
#else
                    ReturningFuture.Get();
#endif
                    Deferred.SetValue();
                });
    return Deferred;
}

template <typename TaskType, typename ReturnType, typename... Args> struct TCoroutinePromiseReturnBase
{
private:
    TSharedPtr<TTaskState<ReturnType>> State;

public:
    explicit TCoroutinePromiseReturnBase(const TSharedRef<TTaskState<ReturnType>> &InState)
        : State(InState)
    {
    }
    TCoroutinePromiseReturnBase(const TCoroutinePromiseReturnBase &) = default;
    TCoroutinePromiseReturnBase(TCoroutinePromiseReturnBase &&) = default;
    ~TCoroutinePromiseReturnBase() = default;

    TaskType get_return_object()
    {
        checkf(
            this->State.IsValid(),
            TEXT("Expected to have a valid task state object when get_return_object is called."));
        return TaskType(__DO_NOT_USE_EXTERNALLY_PrivateToken{}, this->State.ToSharedRef());
    }

    std::suspend_never initial_suspend() noexcept
    {
        return {};
    }

    std::suspend_never final_suspend() noexcept
    {
        return {};
    }

    void return_value(const ReturnType &value)
    {
        checkf(
            this->State.IsValid(),
            TEXT("Expected promise task state to still be valid at the time of return_value being called!"));
        checkf(
            this->State->bIsFutureLinkedToPromise,
            TEXT("Expected future to still be linked to promise when return_value is being called!"));
        this->State->Promise.SetValue(value);
    }

    void unhandled_exception()
    {
        checkf(
            false,
            TEXT("TTask promise threw an unhandled exception. Exceptions should not be used in Unreal Engine C++ "
                 "code."));
    }
};

template <typename TaskType, typename... Args> struct TCoroutinePromiseVoidBase
{
private:
    TSharedPtr<TTaskState<void>> State;

public:
    explicit TCoroutinePromiseVoidBase(const TSharedRef<TTaskState<void>> &InState)
        : State(InState)
    {
    }
    TCoroutinePromiseVoidBase(const TCoroutinePromiseVoidBase &) = default;
    TCoroutinePromiseVoidBase(TCoroutinePromiseVoidBase &&) = default;
    ~TCoroutinePromiseVoidBase() = default;

    TaskType get_return_object()
    {
        checkf(
            this->State.IsValid(),
            TEXT("Expected to have a valid task state object when get_return_object is called."));
        return TaskType(__DO_NOT_USE_EXTERNALLY_PrivateToken{}, this->State.ToSharedRef());
    }

    std::suspend_never initial_suspend() noexcept
    {
        return {};
    }

    std::suspend_never final_suspend() noexcept
    {
        return {};
    }

    void return_void()
    {
        checkf(
            this->State.IsValid(),
            TEXT("Expected promise task state to still be valid at the time of return_void being called!"));
        checkf(
            this->State->bIsFutureLinkedToPromise,
            TEXT("Expected future to still be linked to promise when return_value is being called!"));
        this->State->Promise.SetValue();
    }

    void unhandled_exception()
    {
        checkf(
            false,
            TEXT("TTask promise threw an unhandled exception. Exceptions should not be used in Unreal Engine C++ "
                 "code."));
    }
};

template <typename TaskType, typename...> struct TCoroutineTraits;
template <typename TaskType, typename...> struct TCoroutineClassTraits;
template <typename TaskType, typename...> struct TCoroutineLambdaTraits;

// Task<ResultType> returning global functions.

template <typename ReturnType, typename... Args>
struct TCoroutineTraits<TTaskWith<ReturnType, ETaskBinding::Static>, Args...>
{
    struct promise_type
        : public TCoroutinePromiseReturnBase<TTaskWith<ReturnType, ETaskBinding::Static>, ReturnType, Args...>
    {
    public:
        promise_type()
            : TCoroutinePromiseReturnBase<TTaskWith<ReturnType, ETaskBinding::Static>, ReturnType, Args...>(
                  MakeShareable(new TTaskState<ReturnType>()))
        {
        }
    };
};

// Task<void> returning global functions.

template <typename... Args> struct TCoroutineTraits<TTaskWith<void, ETaskBinding::Static>, Args...>
{
    struct promise_type : public TCoroutinePromiseVoidBase<TTaskWith<void, ETaskBinding::Static>, Args...>
    {
    public:
        promise_type()
            : TCoroutinePromiseVoidBase<TTaskWith<void, ETaskBinding::Static>, Args...>(
                  MakeShareable(new TTaskState<void>()))
        {
        }
    };
};

// Task<ResultType> returning member functions on UObject and TSharedFromThis.

template <typename ReturnType, IsUObject ClassType, typename... Args>
struct TCoroutineClassTraits<TTask<ReturnType>, ClassType &, Args...>
{
    struct promise_type : public TCoroutinePromiseReturnBase<TTask<ReturnType>, ReturnType, Args...>
    {
    public:
        promise_type(ClassType &Object, Args...)
            : TCoroutinePromiseReturnBase<TTask<ReturnType>, ReturnType, Args...>(
                  MakeShareable(new TTaskStateObjectBound<ReturnType, ClassType>(&Object)))
        {
        }
    };
};
template <typename ReturnType, IsUObject ClassType, typename... Args>
struct TCoroutineClassTraits<TTask<ReturnType>, ClassType &&, Args...>
{
    struct promise_type : public TCoroutinePromiseReturnBase<TTask<ReturnType>, ReturnType, Args...>
    {
    public:
        promise_type(ClassType &&Object, Args...)
            : TCoroutinePromiseReturnBase<TTask<ReturnType>, ReturnType, Args...>(
                  MakeShareable(new TTaskStateObjectBound<ReturnType, ClassType>(&Object)))
        {
        }
    };
};
template <typename ReturnType, IsSharedFromThis ClassType, typename... Args>
struct TCoroutineClassTraits<TTask<ReturnType>, ClassType &, Args...>
{
    struct promise_type : public TCoroutinePromiseReturnBase<TTask<ReturnType>, ReturnType, Args...>
    {
    public:
        promise_type(ClassType &Object, Args...)
            : TCoroutinePromiseReturnBase<TTask<ReturnType>, ReturnType, Args...>(
                  MakeShareable(new TTaskStateObjectBound<ReturnType, ClassType>(Object.AsShared())))
        {
        }
    };
};
template <typename ReturnType, IsSharedFromThis ClassType, typename... Args>
struct TCoroutineClassTraits<TTask<ReturnType>, ClassType &&, Args...>
{
    struct promise_type : public TCoroutinePromiseReturnBase<TTask<ReturnType>, ReturnType, Args...>
    {
    public:
        promise_type(ClassType &&Object, Args...)
            : TCoroutinePromiseReturnBase<TTask<ReturnType>, ReturnType, Args...>(
                  MakeShareable(new TTaskStateObjectBound<ReturnType, ClassType>(Object.AsShared())))
        {
        }
    };
};

// Task<void> returning member functions on UObject and TSharedFromThis.

template <IsUObject ClassType, typename... Args> struct TCoroutineClassTraits<TTask<void>, ClassType &, Args...>
{
    struct promise_type : public TCoroutinePromiseVoidBase<TTask<void>, Args...>
    {
    public:
        promise_type(ClassType &Object, Args...)
            : TCoroutinePromiseVoidBase<TTask<void>, Args...>(
                  MakeShareable(new TTaskStateObjectBound<void, ClassType>(&Object)))
        {
        }
    };
};
template <IsUObject ClassType, typename... Args> struct TCoroutineClassTraits<TTask<void>, ClassType &&, Args...>
{
    struct promise_type : public TCoroutinePromiseVoidBase<TTask<void>, Args...>
    {
    public:
        promise_type(ClassType &&Object, Args...)
            : TCoroutinePromiseVoidBase<TTask<void>, Args...>(
                  MakeShareable(new TTaskStateObjectBound<void, ClassType>(&Object)))
        {
        }
    };
};
template <IsSharedFromThis ClassType, typename... Args> struct TCoroutineClassTraits<TTask<void>, ClassType &, Args...>
{
    struct promise_type : public TCoroutinePromiseVoidBase<TTask<void>, Args...>
    {
    public:
        promise_type(ClassType &Object, Args...)
            : TCoroutinePromiseVoidBase<TTask<void>, Args...>(
                  MakeShareable(new TTaskStateObjectBound<void, ClassType>(Object.AsShared())))
        {
        }
    };
};
template <IsSharedFromThis ClassType, typename... Args> struct TCoroutineClassTraits<TTask<void>, ClassType &&, Args...>
{
    struct promise_type : public TCoroutinePromiseVoidBase<TTask<void>, Args...>
    {
    public:
        promise_type(ClassType &&Object, Args...)
            : TCoroutinePromiseVoidBase<TTask<void>, Args...>(
                  MakeShareable(new TTaskStateObjectBound<void, ClassType>(Object.AsShared())))
        {
        }
    };
};

// LambdaTask<Result> on lambda functions.

template <typename ReturnType, typename LambdaType, typename... Args>
struct TCoroutineLambdaTraits<TTaskWith<ReturnType, ETaskBinding::Lambda>, LambdaType &, Args...>
{
    struct promise_type : public TCoroutinePromiseReturnBase<
                              TTaskWith<ReturnType, ETaskBinding::Lambda>,
                              ReturnType,
                              LambdaType &,
                              Args...>
    {
    public:
        promise_type()
            : TCoroutinePromiseReturnBase<
                  TTaskWith<ReturnType, ETaskBinding::Lambda>,
                  ReturnType,
                  LambdaType &,
                  Args...>(MakeShareable(new TTaskState<ReturnType>()))
        {
            static_assert(
                std::is_convertible_v<LambdaType, TTaskWith<ReturnType, ETaskBinding::Lambda> (*)(Args...)>,
                "Lambdas that return TTask<>::With<ETaskBinding::Lambda> must not have any captures!");
        }
    };
};

#if REDPOINT_EOS_ASYNC_INCLUDE_EXTRA_MSVC_LAMBDA_TRAITS
template <typename ReturnType, typename... Args>
struct TCoroutineLambdaTraits<TTaskWith<ReturnType, ETaskBinding::Lambda>, Args...>
{
    struct promise_type
        : public TCoroutinePromiseReturnBase<TTaskWith<ReturnType, ETaskBinding::Lambda>, ReturnType, Args...>
    {
    public:
        promise_type()
            : TCoroutinePromiseReturnBase<TTaskWith<ReturnType, ETaskBinding::Lambda>, ReturnType, Args...>(
                  MakeShareable(new TTaskState<ReturnType>()))
        {
        }
    };
};
#endif

// LambdaTask<void> on lambda functions.

template <typename LambdaType, typename... Args>
struct TCoroutineLambdaTraits<TTaskWith<void, ETaskBinding::Lambda>, LambdaType &, Args...>
{
    struct promise_type : public TCoroutinePromiseVoidBase<TTaskWith<void, ETaskBinding::Lambda>, LambdaType &, Args...>
    {
    public:
        promise_type()
            : TCoroutinePromiseVoidBase<TTaskWith<void, ETaskBinding::Lambda>, LambdaType &, Args...>(
                  MakeShareable(new TTaskState<void>()))
        {
            static_assert(
                std::is_convertible_v<LambdaType, TTaskWith<void, ETaskBinding::Lambda> (*)(Args...)>,
                "Lambdas that return TTask<>::With<ETaskBinding::Lambda> must not have any captures!");
        }
    };
};

#if REDPOINT_EOS_ASYNC_INCLUDE_EXTRA_MSVC_LAMBDA_TRAITS
template <typename... Args> struct TCoroutineLambdaTraits<TTaskWith<void, ETaskBinding::Lambda>, Args...>
{
    struct promise_type : public TCoroutinePromiseVoidBase<TTaskWith<void, ETaskBinding::Lambda>, Args...>
    {
    public:
        promise_type()
            : TCoroutinePromiseVoidBase<TTaskWith<void, ETaskBinding::Lambda>, Args...>(
                  MakeShareable(new TTaskState<void>()))
        {
        }
    };
};
#endif

template <typename ResultType>
void AsCallback(TTask<ResultType> &&Task, typename TCallbackFunctionType<ResultType>::FunctionType Callback)
{
    checkf(Task.State.IsValid(), TEXT("Expected task to contain a valid state object when AsCallback is called!"));
    Task.State->Future.Then([Callback](auto F) {
#if REDPOINT_EOS_UE_5_4_OR_LATER
        Callback(F.Consume());
#else
        Callback(F.Get());
#endif
    });
}

template <> FORCEINLINE void AsCallback(TTask<void> &&Task, typename TCallbackFunctionType<void>::FunctionType Callback)
{
    checkf(Task.State.IsValid(), TEXT("Expected task to contain a valid state object when AsCallback is called!"));
    Task.State->Future.Then([Callback](auto F) {
        Callback();
    });
}

}

namespace Redpoint::EOS::Async
{
REDPOINT_EOS_FILE_NS_EXPORT(4071280369, Redpoint::EOS::Async, ETaskBinding)
REDPOINT_EOS_FILE_NS_EXPORT(4071280369, Redpoint::EOS::Async, TTask)
REDPOINT_EOS_FILE_NS_EXPORT(4071280369, Redpoint::EOS::Async, AsCallback)
}

namespace REDPOINT_EOS_ASYNC_COROUTINE_TRAITS_NAMESPACE
{

#define REDPOINT_EOS_ASYNC_STATIC_ASSERTS                                                                              \
    static_assert(                                                                                                     \
        (negation_v<is_reference<Args>> && ...),                                                                       \
        "Detected pass-by-reference for one or more arguments to this coroutine. Pass-by-reference is not permitted "  \
        "as the references may become invalid when the coroutine is suspending. This error also occurs if you try to " \
        "define a member function returning TTask<> when the class it is defined in does not inherit from "            \
        "TSharedFromThis<> or UObject. Coroutines can only be used on global functions and inside classes "            \
        "implementing UObject or TSharedFromThis<>.");                                                                 \
    static_assert(                                                                                                     \
        (disjunction_v<negation<is_pointer<Args>>, is_base_of<UObject, decay<Args>>> && ...),                          \
        "Detected pass-by-pointer for one or more arguments to this coroutine. Pass-by-pointer in coroutines is only " \
        "permitted if the argument is a UObject-based pointer. Other pointers are not permitted as they may become "   \
        "invalid when the coroutine is suspending.");                                                                  \
    static_assert(                                                                                                     \
        (negation_v<is_member_object_pointer<Args>> && ...),                                                           \
        "Detected one or more arguments to this coroutine are a member object pointer. This is not supported.");       \
    static_assert(                                                                                                     \
        (negation_v<is_member_function_pointer<Args>> && ...),                                                         \
        "Detected one or more arguments to this coroutine are a member function pointer. This is not supported.");

template <typename ReturnType, typename... Args>
struct coroutine_traits<
    ::REDPOINT_EOS_FILE_NS_ID(
        4071280369,
        Redpoint::EOS::Async)::TTaskWith<ReturnType, ::Redpoint::EOS::Async::ETaskBinding::Static>,
    Args...>
{
    REDPOINT_EOS_ASYNC_STATIC_ASSERTS

    using promise_type = typename ::REDPOINT_EOS_FILE_NS_ID(4071280369, Redpoint::EOS::Async)::TCoroutineTraits<
        ::REDPOINT_EOS_FILE_NS_ID(
            4071280369,
            Redpoint::EOS::Async)::TTaskWith<ReturnType, ::Redpoint::EOS::Async::ETaskBinding::Static>,
        Args...>::promise_type;
};
template <typename... Args>
struct coroutine_traits<
    ::REDPOINT_EOS_FILE_NS_ID(
        4071280369,
        Redpoint::EOS::Async)::TTaskWith<void, ::Redpoint::EOS::Async::ETaskBinding::Static>,
    Args...>
{
    REDPOINT_EOS_ASYNC_STATIC_ASSERTS

    using promise_type = typename ::REDPOINT_EOS_FILE_NS_ID(4071280369, Redpoint::EOS::Async)::TCoroutineTraits<
        ::REDPOINT_EOS_FILE_NS_ID(
            4071280369,
            Redpoint::EOS::Async)::TTaskWith<void, ::Redpoint::EOS::Async::ETaskBinding::Static>,
        Args...>::promise_type;
};

template <typename ReturnType, typename LambdaType, typename... Args>
struct coroutine_traits<
    ::REDPOINT_EOS_FILE_NS_ID(
        4071280369,
        Redpoint::EOS::Async)::TTaskWith<ReturnType, ::Redpoint::EOS::Async::ETaskBinding::Lambda>,
    const LambdaType &,
    Args...>
{
    REDPOINT_EOS_ASYNC_STATIC_ASSERTS

    using promise_type = typename ::REDPOINT_EOS_FILE_NS_ID(4071280369, Redpoint::EOS::Async)::TCoroutineLambdaTraits<
        ::REDPOINT_EOS_FILE_NS_ID(
            4071280369,
            Redpoint::EOS::Async)::TTaskWith<ReturnType, ::Redpoint::EOS::Async::ETaskBinding::Lambda>,
        const LambdaType &,
        Args...>::promise_type;
};
template <typename LambdaType, typename... Args>
struct coroutine_traits<
    ::REDPOINT_EOS_FILE_NS_ID(
        4071280369,
        Redpoint::EOS::Async)::TTaskWith<void, ::Redpoint::EOS::Async::ETaskBinding::Lambda>,
    const LambdaType &,
    Args...>
{
    REDPOINT_EOS_ASYNC_STATIC_ASSERTS

    using promise_type = typename ::REDPOINT_EOS_FILE_NS_ID(4071280369, Redpoint::EOS::Async)::TCoroutineLambdaTraits<
        ::REDPOINT_EOS_FILE_NS_ID(
            4071280369,
            Redpoint::EOS::Async)::TTaskWith<void, ::Redpoint::EOS::Async::ETaskBinding::Lambda>,
        const LambdaType &,
        Args...>::promise_type;
};
#if REDPOINT_EOS_ASYNC_INCLUDE_EXTRA_MSVC_LAMBDA_TRAITS
template <typename ReturnType, typename... Args>
struct coroutine_traits<
    ::REDPOINT_EOS_FILE_NS_ID(
        4071280369,
        Redpoint::EOS::Async)::TTaskWith<ReturnType, ::Redpoint::EOS::Async::ETaskBinding::Lambda>,
    Args...>
{
    REDPOINT_EOS_ASYNC_STATIC_ASSERTS

    using promise_type = typename ::REDPOINT_EOS_FILE_NS_ID(4071280369, Redpoint::EOS::Async)::TCoroutineLambdaTraits<
        ::REDPOINT_EOS_FILE_NS_ID(
            4071280369,
            Redpoint::EOS::Async)::TTaskWith<ReturnType, ::Redpoint::EOS::Async::ETaskBinding::Lambda>,
        Args...>::promise_type;
};
template <typename... Args>
struct coroutine_traits<
    ::REDPOINT_EOS_FILE_NS_ID(
        4071280369,
        Redpoint::EOS::Async)::TTaskWith<void, ::Redpoint::EOS::Async::ETaskBinding::Lambda>,
    Args...>
{
    REDPOINT_EOS_ASYNC_STATIC_ASSERTS

    using promise_type = typename ::REDPOINT_EOS_FILE_NS_ID(4071280369, Redpoint::EOS::Async)::TCoroutineLambdaTraits<
        ::REDPOINT_EOS_FILE_NS_ID(
            4071280369,
            Redpoint::EOS::Async)::TTaskWith<void, ::Redpoint::EOS::Async::ETaskBinding::Lambda>,
        Args...>::promise_type;
};
#endif

template <
    typename ReturnType,
    ::REDPOINT_EOS_FILE_NS_ID(4071280369, Redpoint::EOS::Async)::IsSharedFromThis ClassType,
    typename... Args>
struct coroutine_traits<::Redpoint::EOS::Async::TTask<ReturnType>, ClassType &, Args...>
{
    REDPOINT_EOS_ASYNC_STATIC_ASSERTS

    using promise_type = typename ::REDPOINT_EOS_FILE_NS_ID(4071280369, Redpoint::EOS::Async)::
        TCoroutineClassTraits<::Redpoint::EOS::Async::TTask<ReturnType>, ClassType &, Args...>::promise_type;
};
template <::REDPOINT_EOS_FILE_NS_ID(4071280369, Redpoint::EOS::Async)::IsSharedFromThis ClassType, typename... Args>
struct coroutine_traits<::Redpoint::EOS::Async::TTask<void>, ClassType &, Args...>
{
    REDPOINT_EOS_ASYNC_STATIC_ASSERTS

    using promise_type = typename ::REDPOINT_EOS_FILE_NS_ID(4071280369, Redpoint::EOS::Async)::
        TCoroutineClassTraits<::Redpoint::EOS::Async::TTask<void>, ClassType &, Args...>::promise_type;
};
template <
    typename ReturnType,
    ::REDPOINT_EOS_FILE_NS_ID(4071280369, Redpoint::EOS::Async)::IsSharedFromThis ClassType,
    typename... Args>
struct coroutine_traits<::Redpoint::EOS::Async::TTask<ReturnType>, ClassType &&, Args...>
{
    REDPOINT_EOS_ASYNC_STATIC_ASSERTS

    using promise_type = typename ::REDPOINT_EOS_FILE_NS_ID(4071280369, Redpoint::EOS::Async)::
        TCoroutineClassTraits<::Redpoint::EOS::Async::TTask<ReturnType>, ClassType &&, Args...>::promise_type;
};
template <::REDPOINT_EOS_FILE_NS_ID(4071280369, Redpoint::EOS::Async)::IsSharedFromThis ClassType, typename... Args>
struct coroutine_traits<::Redpoint::EOS::Async::TTask<void>, ClassType &&, Args...>
{
    REDPOINT_EOS_ASYNC_STATIC_ASSERTS

    using promise_type = typename ::REDPOINT_EOS_FILE_NS_ID(4071280369, Redpoint::EOS::Async)::
        TCoroutineClassTraits<::Redpoint::EOS::Async::TTask<void>, ClassType &&, Args...>::promise_type;
};

template <
    typename ReturnType,
    ::REDPOINT_EOS_FILE_NS_ID(4071280369, Redpoint::EOS::Async)::IsUObject ClassType,
    typename... Args>
struct coroutine_traits<::Redpoint::EOS::Async::TTask<ReturnType>, ClassType &, Args...>
{
    REDPOINT_EOS_ASYNC_STATIC_ASSERTS

    using promise_type = typename ::REDPOINT_EOS_FILE_NS_ID(4071280369, Redpoint::EOS::Async)::
        TCoroutineClassTraits<::Redpoint::EOS::Async::TTask<ReturnType>, ClassType &, Args...>::promise_type;
};
template <::REDPOINT_EOS_FILE_NS_ID(4071280369, Redpoint::EOS::Async)::IsUObject ClassType, typename... Args>
struct coroutine_traits<::Redpoint::EOS::Async::TTask<void>, ClassType &, Args...>
{
    REDPOINT_EOS_ASYNC_STATIC_ASSERTS

    using promise_type = typename ::REDPOINT_EOS_FILE_NS_ID(4071280369, Redpoint::EOS::Async)::
        TCoroutineClassTraits<::Redpoint::EOS::Async::TTask<void>, ClassType &, Args...>::promise_type;
};
template <
    typename ReturnType,
    ::REDPOINT_EOS_FILE_NS_ID(4071280369, Redpoint::EOS::Async)::IsUObject ClassType,
    typename... Args>
struct coroutine_traits<::Redpoint::EOS::Async::TTask<ReturnType>, ClassType &&, Args...>
{
    REDPOINT_EOS_ASYNC_STATIC_ASSERTS

    using promise_type = typename ::REDPOINT_EOS_FILE_NS_ID(4071280369, Redpoint::EOS::Async)::
        TCoroutineClassTraits<::Redpoint::EOS::Async::TTask<ReturnType>, ClassType &&, Args...>::promise_type;
};
template <::REDPOINT_EOS_FILE_NS_ID(4071280369, Redpoint::EOS::Async)::IsUObject ClassType, typename... Args>
struct coroutine_traits<::Redpoint::EOS::Async::TTask<void>, ClassType &&, Args...>
{
    REDPOINT_EOS_ASYNC_STATIC_ASSERTS

    using promise_type = typename ::REDPOINT_EOS_FILE_NS_ID(4071280369, Redpoint::EOS::Async)::
        TCoroutineClassTraits<::Redpoint::EOS::Async::TTask<void>, ClassType &&, Args...>::promise_type;
};

#undef REDPOINT_EOS_ASYNC_STATIC_ASSERTS
}

REDPOINT_EOS_CODE_GUARD_END()

#undef REDPOINT_EOS_ASYNC_COROUTINE_TRAITS_NAMESPACE
#undef REDPOINT_EOS_ASYNC_INCLUDE_EXTRA_MSVC_LAMBDA_TRAITS