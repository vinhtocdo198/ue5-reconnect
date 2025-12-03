// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAsync/AsyncTestsLatentCommand.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(295298669, Redpoint::EOS::Async)
{

FAsyncTestsLatentCommand::FAsyncTestsLatentCommand(
    const TSharedRef<FAutomationTestBase> &InAssert,
    const TFunction<TTask<void>(TSharedRef<FAutomationTestBase> Assert)> &InTestCode)
    : Assert(InAssert)
    , bDone()
    , bStarted()
    , TestCode(InTestCode)
{
}

bool FAsyncTestsLatentCommand::Update()
{
    if (!this->bStarted)
    {
        AsCallback(this->TestCode(this->Assert), [this]() {
            this->bDone = true;
        });
        this->bStarted = true;
    }

    return this->bDone;
}

}

REDPOINT_EOS_CODE_GUARD_END()