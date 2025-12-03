// Copyright June Rhodes. All Rights Reserved.

#include "Containers/Ticker.h"
#include "Containers/UnrealString.h"
#include "Misc/AutomationTest.h"
#include "RedpointEOSAsync/AsyncAutomationTest.h"
#include "RedpointEOSAsync/Task.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "TestAsyncUObject.h"
#include "UObject/Object.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2553909252, Redpoint::EOS::Async)
{
using namespace ::Redpoint::EOS::Async;

static TTask<void> DeferToNextFrame()
{
    auto Deferred = TTask<void>::Deferred();
    FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateLambda([Deferred](float) -> bool {
            Deferred.SetValue();
            return false;
        }),
        0.0f);
    return Deferred;
}

static TTask<void>::With<ETaskBinding::Static> ReturnVoid()
{
    co_return;
}

static TTask<int>::With<ETaskBinding::Static> ReturnInt()
{
    co_return 5;
}

static TTask<int>::With<ETaskBinding::Static> ReturnIntOnNextFrame()
{
    co_await DeferToNextFrame();
    co_return 5;
}

static TTask<int>::With<ETaskBinding::Static> AddNumbersWithAwait()
{
    co_return (co_await ReturnIntOnNextFrame()) + (co_await ReturnIntOnNextFrame());
}

static TTask<int>::With<ETaskBinding::Static> CoAwaitInLoop()
{
    int k = 0;
    for (int i = 0; i < 5; i++)
    {
        k += co_await ReturnIntOnNextFrame();
    }
    co_return k;
}

static TTask<FString>::With<ETaskBinding::Static> PassedStringValue(FString Value)
{
    co_return Value;
}

// @note: These function/class declarations should not compile if static_assert has been set up correctly.
#if 0

static TTask<void>::With<ETaskBinding::Static> PassedStringReference(const FString &)
{
    co_return;
}

static TTask<void>::With<ETaskBinding::Static> PassedStringMove(FString &&)
{
    co_return;
}

class FClassThatShouldNotBeAbleToUseTasks
{
public:
    TTask<void> VoidTask()
    {
        co_return;
    }

    TTask<int> IntTask()
    {
        co_return 5;
    }
};

#endif

class FSharedClass : public TSharedFromThis<FSharedClass>
{
private:
    FString SomeValue;

public:
    FSharedClass(const FString &InSomeValue)
        : SomeValue(InSomeValue)
    {
    }

    TTask<void> VoidTask()
    {
        co_return;
    }

    TTask<int> IntTask()
    {
        co_await DeferToNextFrame();
        co_return this->SomeValue.Len();
    }
};

static TTask<int>::With<ETaskBinding::Static> TestLambdaWithCapture()
{
    FString Blah = TEXT("Test");
    auto A = []() -> TTask<int>::With<ETaskBinding::Lambda> {
        co_return 5;
    };
#if 0
    // @note: This lambda should not compile if TLambdaTask static_asserts have been set up correctly.
    auto L = [Blah]() -> TLambdaTask<int> {
        co_return Blah.Len();
    };
    // @note: This lambda will not compile because lambdas are an incomplete type when evaluating is_base.
    auto X = [Blah]() -> TTask<int> {
        co_return Blah.Len();
    };
#endif
    co_return co_await A();
}

static TSharedRef<FAsyncAutomationTest> OnlineSubsystemEOS_AsyncTask = MakeShared<FAsyncAutomationTest>(
    TEXT("Redpoint.EOS.AsyncTask"),
    [](TSharedRef<FAutomationTestBase> Assert) -> TTask<void>::With<ETaskBinding::Lambda> {
        co_await ReturnVoid();

        co_await TestLambdaWithCapture();

        Assert->TestEqual(TEXT("PassedStringValue"), co_await PassedStringValue(TEXT("Text")), TEXT("Text"));

// @note: These function/class declarations should not compile if static_assert has been set up correctly.
#if 0
        co_await PassedStringReference(TEXT("Test"));

        FString ValueToMove = TEXT("Test");
        co_await PassedStringMove(MoveTemp(ValueToMove));

        {
            FClassThatShouldNotBeAbleToUseTasks Obj;
            co_await Obj.VoidTask();
            co_await Obj.IntTask();
        }
#endif

        auto TaskReturningLambdaThatReleasesScope = []() -> TTask<int> {
            TSharedRef<FSharedClass> ReleasingObj = MakeShared<FSharedClass>(TEXT("Test"));
            return ReleasingObj->IntTask();
        };

        auto GotTask = TaskReturningLambdaThatReleasesScope();
        Assert->TestFalse(TEXT("Expected task from releasing lambda to no longer be bound!"), GotTask.IsBound());

        co_await DeferToNextFrame();

        {
            TSharedRef<FSharedClass> Obj = MakeShared<FSharedClass>(TEXT("Test"));
            co_await Obj->VoidTask();
            Assert->TestEqual(TEXT("Obj->IntTask (TSharedFromThis)"), co_await Obj->IntTask(), 4);
        }
        {
            auto *Obj = NewObject<UTestAsyncUObject>();
            co_await Obj->VoidTask();
            Assert->TestEqual(TEXT("Obj->IntTask (UObject)"), co_await Obj->IntTask(), 5);
        }

        Assert->TestEqual(TEXT("ReturnInt"), co_await ReturnInt(), 5);
        Assert->TestEqual(TEXT("ReturnIntOnNextFrame"), co_await ReturnIntOnNextFrame(), 5);
        Assert->TestEqual(TEXT("AddNumbersWithAwait"), co_await AddNumbersWithAwait(), 10);
        Assert->TestEqual(TEXT("CoAwaitInLoop"), co_await CoAwaitInLoop(), 25);

        auto Immediate1 = TTask<void>::FromResult();
        auto Immediate2 = TTask<bool>::FromResult(true);

        Assert->TestTrue(TEXT("Immediate1 should be ready"), Immediate1.IsReady());
        Assert->TestTrue(TEXT("Immediate2 should be ready"), Immediate2.IsReady());
        Assert->TestTrue(TEXT("Immediate2 should be awaitable and return true value"), co_await Immediate2);

        co_await TTask<void>::FromResult();
        Assert->TestTrue(
            TEXT("FromResult value should be awaitable and return true value"),
            co_await TTask<bool>::FromResult(true));

        bool bImmediateVoidCallback = false;
        AsCallback(TTask<void>::FromResult(), [&bImmediateVoidCallback]() {
            bImmediateVoidCallback = true;
        });

        bool bImmediateBoolCallback = false;
        AsCallback(TTask<bool>::FromResult(true), [&bImmediateBoolCallback](bool Value) {
            bImmediateBoolCallback = Value;
        });

        Assert->TestTrue(TEXT("bImmediateVoidCallback should be true"), bImmediateVoidCallback);
        Assert->TestTrue(TEXT("bImmediateBoolCallback should be true"), bImmediateBoolCallback);

        co_return;
    });

}

REDPOINT_EOS_CODE_GUARD_END()