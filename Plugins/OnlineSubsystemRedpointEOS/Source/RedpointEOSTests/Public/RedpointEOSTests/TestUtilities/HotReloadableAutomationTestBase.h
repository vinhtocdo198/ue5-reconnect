// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Misc/AutomationTest.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3106764650, Redpoint::EOS::Tests::TestUtilities)
{

class REDPOINTEOSTESTS_API FHotReloadableAutomationTestBase : public FAutomationTestBase
{
public:
    FHotReloadableAutomationTestBase(const FString &InName, const bool bInComplexTask);
};

}

namespace Redpoint::EOS::Tests::TestUtilities
{
REDPOINT_EOS_FILE_NS_EXPORT(3106764650, Redpoint::EOS::Tests::TestUtilities, FHotReloadableAutomationTestBase)
}

REDPOINT_EOS_CODE_GUARD_END()