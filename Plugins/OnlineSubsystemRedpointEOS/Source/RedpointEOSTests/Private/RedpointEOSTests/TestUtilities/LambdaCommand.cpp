// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/TestUtilities/LambdaCommand.h"

#include "RedpointEOSTests/TestUtilities/AsyncHotReloadableAutomationTestBase.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2996387358, Redpoint::EOS::Tests::TestUtilities)
{
using namespace ::Redpoint::EOS::Tests::TestUtilities;

FLambdaCommand::FLambdaCommand(FAsyncHotReloadableAutomationTestBase *InTestBase, FLambdaTest InTestCode)
    : TestBase(InTestBase)
    , TestCode(MoveTemp(InTestCode))
    , bIsStarted(false)
    , bIsDone(false)
{
}

bool FLambdaCommand::Update()
{
    if (!this->bIsStarted)
    {
        this->bIsStarted = true;
        this->TestCode([this]() {
            this->bIsDone = true;
        });
    }

    if (this->TestBase != nullptr)
    {
        TArray<int> IndexesToRemove;
        TArray<TFunction<void()>> ContinuesToFire;
        int ConditionCount = this->TestBase->WaitingOnConditions.Num();
        for (int i = ConditionCount - 1; i >= 0; i--)
        {
            if (this->TestBase->WaitingOnConditions[i].Key())
            {
                check(
                    this->TestBase->WaitingOnConditions.Num() ==
                    ConditionCount /* Conditions must not call WaitOnCondition! */);
                ContinuesToFire.Add(this->TestBase->WaitingOnConditions[i].Value);
                this->TestBase->WaitingOnConditions.RemoveAt(i);
            }
        }
        for (const auto &FireFunc : ContinuesToFire)
        {
            FireFunc();
        }
    }

    return this->bIsDone;
}

}

REDPOINT_EOS_CODE_GUARD_END()