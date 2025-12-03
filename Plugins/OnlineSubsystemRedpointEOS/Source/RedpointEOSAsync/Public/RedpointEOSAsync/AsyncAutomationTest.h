// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Misc/AutomationTest.h"
#include "RedpointEOSAsync/Task.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#if __has_include(<source_location>)
#include <source_location>
#elif __has_include(<experimental/source_location>)
#include <experimental/source_location>
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3601138469, Redpoint::EOS::Async)
{
using namespace ::Redpoint::EOS::Async;

#if __has_include(<source_location>)
using source_location = std::source_location;
#elif __has_include(<experimental/source_location>)
using source_location = std::experimental::source_location;
#else
struct source_location
{
    static source_location current()
    {
        return source_location();
    }
    FString file_name() const
    {
        return TEXT("");
    }
    int32 line() const
    {
        return 0;
    }
};
#endif

class REDPOINTEOSASYNC_API FAsyncAutomationTest : public FAutomationTestBase,
                                                  public TSharedFromThis<FAsyncAutomationTest>
{
private:
    FString PrettyName;
    TFunction<TTask<void>(TSharedRef<FAutomationTestBase> Assert)> TestCode;
    const source_location Location;

public:
    FAsyncAutomationTest(
        const FString &InPrettyName,
        const TFunction<TTask<void>(TSharedRef<FAutomationTestBase> Assert)> &InTestCode,
        const source_location InLocation = source_location::current());

#if REDPOINT_EOS_UE_5_5_OR_LATER
    virtual EAutomationTestFlags GetTestFlags() const override;
#else
    virtual uint32 GetTestFlags() const override;
#endif
    virtual bool IsStressTest() const;
    virtual uint32 GetRequiredDeviceNum() const override;
    virtual FString GetTestSourceFileName() const override;
    virtual int32 GetTestSourceFileLine() const override;

protected:
    virtual void GetTests(TArray<FString> &OutBeautifiedNames, TArray<FString> &OutTestCommands) const override;
    virtual bool RunTest(const FString &Parameters) override;
    virtual FString GetBeautifiedTestName() const override;
};

}

namespace Redpoint::EOS::Async
{
REDPOINT_EOS_FILE_NS_EXPORT(3601138469, Redpoint::EOS::Async, FAsyncAutomationTest)
}

REDPOINT_EOS_CODE_GUARD_END()