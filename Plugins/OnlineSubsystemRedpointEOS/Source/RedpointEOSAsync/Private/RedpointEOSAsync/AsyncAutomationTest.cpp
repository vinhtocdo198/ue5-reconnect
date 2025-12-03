// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAsync/AsyncAutomationTest.h"

#include "RedpointEOSAsync/AsyncTestsLatentCommand.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3601138469, Redpoint::EOS::Async)
{
using namespace ::Redpoint::EOS::Async;

FAsyncAutomationTest::FAsyncAutomationTest(
    const FString &InPrettyName,
    const TFunction<TTask<void>(TSharedRef<FAutomationTestBase> Assert)> &InTestCode,
    const source_location InLocation)
    : FAutomationTestBase(InPrettyName, false)
    , PrettyName(InPrettyName)
    , TestCode(InTestCode)
    , Location(InLocation)
{
}

#if REDPOINT_EOS_UE_5_5_OR_LATER
EAutomationTestFlags FAsyncAutomationTest::GetTestFlags() const
{
    return EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext |
           EAutomationTestFlags::ServerContext | EAutomationTestFlags::ProductFilter;
}
#else
uint32 FAsyncAutomationTest::GetTestFlags() const
{
    return static_cast<uint32>(
        EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext |
        EAutomationTestFlags::ServerContext | EAutomationTestFlags::ProductFilter);
}
#endif

bool FAsyncAutomationTest::IsStressTest() const
{
    return false;
}

uint32 FAsyncAutomationTest::GetRequiredDeviceNum() const
{
    return 1;
}

FString FAsyncAutomationTest::GetTestSourceFileName() const
{
    return Location.file_name();
}

int32 FAsyncAutomationTest::GetTestSourceFileLine() const
{
    return static_cast<int32>(Location.line());
}

void FAsyncAutomationTest::GetTests(TArray<FString> &OutBeautifiedNames, TArray<FString> &OutTestCommands) const
{
    OutBeautifiedNames.Add(PrettyName);
    OutTestCommands.Add(FString());
}

bool FAsyncAutomationTest::RunTest(const FString &Parameters)
{
    FAutomationTestFramework::Get().SetRequestedTestFilter(EAutomationTestFlags::ProductFilter);
    FAutomationTestFramework::Get().EnqueueLatentCommand(
        MakeShareable(new FAsyncTestsLatentCommand(this->AsShared(), this->TestCode)));
    return true;
}

FString FAsyncAutomationTest::GetBeautifiedTestName() const
{
    return PrettyName;
};

}

REDPOINT_EOS_CODE_GUARD_END()