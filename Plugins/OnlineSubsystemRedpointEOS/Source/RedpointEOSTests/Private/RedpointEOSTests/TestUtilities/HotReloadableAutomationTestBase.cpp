// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/TestUtilities/HotReloadableAutomationTestBase.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3106764650, Redpoint::EOS::Tests::TestUtilities)
{

FHotReloadableAutomationTestBase::FHotReloadableAutomationTestBase(const FString &InName, const bool bInComplexTask)
    : FAutomationTestBase(InName, bInComplexTask)
{
    FAutomationTestFramework::Get().UnregisterAutomationTest(TestName);
    FAutomationTestFramework::Get().RegisterAutomationTest(InName, this);
}

}

REDPOINT_EOS_CODE_GUARD_END()