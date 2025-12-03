// Copyright June Rhodes. All Rights Reserved.

#include "HAL/MemoryMisc.h"
#include "Misc/AutomationTest.h"
#include "RedpointEOSCore/Utils/MultiOperation.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FOnlineSubsystemEOS_MultiOperation_RunZero,
    "Redpoint.EOS.MultiOperation.RunZero",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

bool FOnlineSubsystemEOS_MultiOperation_RunZero::RunTest(const FString &Parameters)
{
    using namespace ::Redpoint::EOS::Core::Utils;

    TArray<bool> Inputs;
    bool bWasOnDoneCalled = false;
    FMultiOperation<bool, int>::Run(
        Inputs,
        FMultiOperation<bool, int>::FInitiate::CreateLambda([](bool Value, const TDelegate<void(int Out)> &OnDone) {
            OnDone.ExecuteIfBound(1);
            return true;
        }),
        FMultiOperation<bool, int>::FOnDone::CreateLambda([this, &bWasOnDoneCalled](const TArray<int> &Result) {
            this->TestEqual("Got 0 results from results", 0, Result.Num());
            bWasOnDoneCalled = true;
        }));
    this->TestTrue("OnDone was called", bWasOnDoneCalled);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FOnlineSubsystemEOS_MultiOperation_RunOne,
    "Redpoint.EOS.MultiOperation.RunOne",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

bool FOnlineSubsystemEOS_MultiOperation_RunOne::RunTest(const FString &Parameters)
{
    using namespace ::Redpoint::EOS::Core::Utils;

    TArray<bool> Inputs;
    Inputs.Add(true);
    bool bWasOnDoneCalled = false;
    FMultiOperation<bool, int>::Run(
        Inputs,
        FMultiOperation<bool, int>::FInitiate::CreateLambda([](bool Value, const TDelegate<void(int Out)> &OnDone) {
            OnDone.ExecuteIfBound(1);
            return true;
        }),
        FMultiOperation<bool, int>::FOnDone::CreateLambda([this, &bWasOnDoneCalled](const TArray<int> &Result) {
            this->TestEqual("Got 1 results from results", 1, Result.Num());
            bWasOnDoneCalled = true;
        }));
    this->TestTrue("OnDone was called", bWasOnDoneCalled);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FOnlineSubsystemEOS_MultiOperation_RunMultiple,
    "Redpoint.EOS.MultiOperation.RunMultiple",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

bool FOnlineSubsystemEOS_MultiOperation_RunMultiple::RunTest(const FString &Parameters)
{
    using namespace ::Redpoint::EOS::Core::Utils;

    TArray<bool> Inputs;
    Inputs.Add(true);
    Inputs.Add(false);
    Inputs.Add(true);
    bool bWasOnDoneCalled = false;
    FMultiOperation<bool, int>::Run(
        Inputs,
        FMultiOperation<bool, int>::FInitiate::CreateLambda([](bool Value, const TDelegate<void(int Out)> &OnDone) {
            OnDone.ExecuteIfBound(Value ? 2 : 3);
            return true;
        }),
        FMultiOperation<bool, int>::FOnDone::CreateLambda([this, &bWasOnDoneCalled](const TArray<int> &Result) {
            this->TestEqual("Got 3 results from results", 3, Result.Num());
            this->TestEqual("Index 0 was 2", 2, Result[0]);
            this->TestEqual("Index 1 was 3", 3, Result[1]);
            this->TestEqual("Index 2 was 2", 2, Result[2]);
            bWasOnDoneCalled = true;
        }));
    this->TestTrue("OnDone was called", bWasOnDoneCalled);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FOnlineSubsystemEOS_MultiOperation_RunBatched,
    "Redpoint.EOS.MultiOperation.RunBatched",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

bool FOnlineSubsystemEOS_MultiOperation_RunBatched::RunTest(const FString &Parameters)
{
    using namespace ::Redpoint::EOS::Core::Utils;

    TArray<int> Inputs;
    Inputs.Add(0);
    Inputs.Add(1);
    Inputs.Add(2);
    Inputs.Add(3);
    Inputs.Add(4);
    Inputs.Add(5);
    Inputs.Add(6);
    Inputs.Add(7);
    Inputs.Add(8);
    bool bWasOnDoneCalled = false;
    bool bBatchOneCalled = false;
    bool bBatchTwoCalled = false;
    FMultiOperation<int, int>::RunBatched(
        Inputs,
        5,
        FMultiOperation<int, int>::FBatchedInitiate::CreateLambda([this, &bBatchOneCalled, &bBatchTwoCalled](
                                                                      TArray<int> Batch,
                                                                      const TDelegate<void(TArray<int> Out)> &OnDone) {
            this->TestTrue("Batch is either 4 or 5 in size", Batch.Num() == 4 || Batch.Num() == 5);
            if (Batch.Num() == 5)
            {
                bBatchOneCalled = true;
                this->TestEqual("Batch 1 index 0 equal 0", 0, Batch[0]);
                this->TestEqual("Batch 1 index 1 equal 1", 1, Batch[1]);
                this->TestEqual("Batch 1 index 2 equal 2", 2, Batch[2]);
                this->TestEqual("Batch 1 index 3 equal 3", 3, Batch[3]);
                this->TestEqual("Batch 1 index 4 equal 4", 4, Batch[4]);
            }
            else if (Batch.Num() == 4)
            {
                bBatchTwoCalled = true;
                this->TestEqual("Batch 2 index 0 equal 5", 5, Batch[0]);
                this->TestEqual("Batch 2 index 1 equal 6", 6, Batch[1]);
                this->TestEqual("Batch 2 index 2 equal 7", 7, Batch[2]);
                this->TestEqual("Batch 2 index 3 equal 8", 8, Batch[3]);
            }
            OnDone.ExecuteIfBound(Batch);
            return true;
        }),
        FMultiOperation<int, int>::FOnDone::CreateLambda([this, &bWasOnDoneCalled](TArray<int> Result) {
            this->TestEqual("Got 9 results from results", 9, Result.Num());
            this->TestEqual("Index 0 was 0", 0, Result[0]);
            this->TestEqual("Index 1 was 1", 1, Result[1]);
            this->TestEqual("Index 2 was 2", 2, Result[2]);
            this->TestEqual("Index 3 was 3", 3, Result[3]);
            this->TestEqual("Index 4 was 4", 4, Result[4]);
            this->TestEqual("Index 5 was 5", 5, Result[5]);
            this->TestEqual("Index 6 was 6", 6, Result[6]);
            this->TestEqual("Index 7 was 7", 7, Result[7]);
            this->TestEqual("Index 8 was 8", 8, Result[8]);
            bWasOnDoneCalled = true;
        }));
    this->TestTrue("First batch was called", bBatchOneCalled);
    this->TestTrue("Second batch was called", bBatchTwoCalled);
    this->TestTrue("OnDone was called", bWasOnDoneCalled);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FOnlineSubsystemEOS_MultiOperation_RunBatchedEdge,
    "Redpoint.EOS.MultiOperation.RunBatchedEdge",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

bool FOnlineSubsystemEOS_MultiOperation_RunBatchedEdge::RunTest(const FString &Parameters)
{
    using namespace ::Redpoint::EOS::Core::Utils;

    TArray<int> Inputs;
    Inputs.Add(0);
    Inputs.Add(1);
    Inputs.Add(2);
    Inputs.Add(3);
    Inputs.Add(4);
    bool bWasOnDoneCalled = false;
    bool bBatchOneCalled = false;
    int BatchCallCount = 0;
    FMultiOperation<int, int>::RunBatched(
        Inputs,
        5,
        FMultiOperation<int, int>::FBatchedInitiate::CreateLambda([this, &bBatchOneCalled, &BatchCallCount](
                                                                      TArray<int> Batch,
                                                                      const TDelegate<void(TArray<int> Out)> &OnDone) {
            BatchCallCount++;
            this->TestTrue("Batch is 5 in size", Batch.Num() == 5);
            if (Batch.Num() == 5)
            {
                bBatchOneCalled = true;
                this->TestEqual("Batch 1 index 0 equal 0", 0, Batch[0]);
                this->TestEqual("Batch 1 index 1 equal 1", 1, Batch[1]);
                this->TestEqual("Batch 1 index 2 equal 2", 2, Batch[2]);
                this->TestEqual("Batch 1 index 3 equal 3", 3, Batch[3]);
                this->TestEqual("Batch 1 index 4 equal 4", 4, Batch[4]);
            }
            OnDone.ExecuteIfBound(Batch);
            return true;
        }),
        FMultiOperation<bool, int>::FOnDone::CreateLambda([this, &bWasOnDoneCalled](TArray<int> Result) {
            this->TestEqual("Got 5 results from results", 5, Result.Num());
            this->TestEqual("Index 0 was 0", 0, Result[0]);
            this->TestEqual("Index 1 was 1", 1, Result[1]);
            this->TestEqual("Index 2 was 2", 2, Result[2]);
            this->TestEqual("Index 3 was 3", 3, Result[3]);
            this->TestEqual("Index 4 was 4", 4, Result[4]);
            bWasOnDoneCalled = true;
        }));
    this->TestTrue("First batch was called", bBatchOneCalled);
    this->TestEqual("Batch called exactly once", 1, BatchCallCount);
    this->TestTrue("OnDone was called", bWasOnDoneCalled);
    return true;
}