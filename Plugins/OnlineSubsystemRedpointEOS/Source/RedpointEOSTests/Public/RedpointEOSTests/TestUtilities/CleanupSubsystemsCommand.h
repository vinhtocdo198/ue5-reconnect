// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Misc/AutomationTest.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/TestUtilities/Delegates.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(
    1902792364,
    Redpoint::EOS::Tests::TestUtilities,
    FAsyncHotReloadableAutomationTestBase)

namespace REDPOINT_EOS_FILE_NS_ID(2476792889, Redpoint::EOS::Tests::TestUtilities)
{
using namespace ::Redpoint::EOS::Tests::TestUtilities;

class REDPOINTEOSTESTS_API FCleanupSubsystemsCommand : public IAutomationLatentCommand
{
public:
    FCleanupSubsystemsCommand() = delete;
    UE_NONCOPYABLE(FCleanupSubsystemsCommand);
    FCleanupSubsystemsCommand(FAsyncHotReloadableAutomationTestBase *InTestBase);
    virtual ~FCleanupSubsystemsCommand() override = default;
    virtual bool Update() override;

private:
    FAsyncHotReloadableAutomationTestBase *TestBase;
};

}

namespace Redpoint::EOS::Tests::TestUtilities
{
REDPOINT_EOS_FILE_NS_EXPORT(2476792889, Redpoint::EOS::Tests::TestUtilities, FCleanupSubsystemsCommand)
}

REDPOINT_EOS_CODE_GUARD_END()