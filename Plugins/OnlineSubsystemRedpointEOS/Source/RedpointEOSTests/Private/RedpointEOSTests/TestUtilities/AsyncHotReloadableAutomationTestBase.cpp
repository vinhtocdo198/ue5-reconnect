// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/TestUtilities/AsyncHotReloadableAutomationTestBase.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1902792364, Redpoint::EOS::Tests::TestUtilities)
{

void FAsyncHotReloadableAutomationTestBase::WhenCondition(TFunction<bool()> Condition, TFunction<void()> OnContinue)
{
    this->WaitingOnConditions.Add(TTuple<TFunction<bool()>, TFunction<void()>>(Condition, OnContinue));
}

}

REDPOINT_EOS_CODE_GUARD_END()