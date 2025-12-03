// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSAsync/AsyncAutomationTest.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(4127373091, Redpoint::EOS::Tests::Async)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Async;

TFunction<TTask<void>(TSharedRef<FAutomationTestBase> Assert)> TestWithPlatforms(
    TArray<int32> InstanceLayout,
    TFunction<TTask<void>(TSharedRef<FAutomationTestBase> Assert, TArray<FPlatformHandle> Platforms)> Test);

}

namespace Redpoint::EOS::Tests::Async
{
REDPOINT_EOS_FILE_NS_EXPORT(4127373091, Redpoint::EOS::Tests::Async, TestWithPlatforms)
}

REDPOINT_EOS_CODE_GUARD_END()
