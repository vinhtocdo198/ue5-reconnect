// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Misc/AutomationTest.h"
#include "RedpointEOSAsync/AsyncAutomationTest.h"
#include "RedpointEOSAsync/Task.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(295298669, Redpoint::EOS::Async)
{
using namespace ::Redpoint::EOS::Async;

class FAsyncTestsLatentCommand : public IAutomationLatentCommand
{
public:
    TSharedRef<FAutomationTestBase> Assert;
    bool bDone;
    bool bStarted;
    TFunction<TTask<void>(TSharedRef<FAutomationTestBase> Assert)> TestCode;

    FAsyncTestsLatentCommand(
        const TSharedRef<FAutomationTestBase> &InAssert,
        const TFunction<TTask<void>(TSharedRef<FAutomationTestBase> Assert)> &InTestCode);
    UE_NONCOPYABLE(FAsyncTestsLatentCommand);
    virtual ~FAsyncTestsLatentCommand() override = default;

    virtual bool Update() override;
};

}

namespace Redpoint::EOS::Async
{
REDPOINT_EOS_FILE_NS_EXPORT(295298669, Redpoint::EOS::Async, FAsyncTestsLatentCommand)
}

REDPOINT_EOS_CODE_GUARD_END()