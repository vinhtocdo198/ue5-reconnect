// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "OnlineSubsystem.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/TestUtilities/Delegates.h"
#include "RedpointEOSTests/TestUtilities/HotReloadableAutomationTestBase.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(2996387358, Redpoint::EOS::Tests::TestUtilities, FLambdaCommand)

namespace REDPOINT_EOS_FILE_NS_ID(1902792364, Redpoint::EOS::Tests::TestUtilities)
{
using namespace ::Redpoint::EOS::Tests::TestUtilities;

class REDPOINTEOSTESTS_API FAsyncHotReloadableAutomationTestBase : public FHotReloadableAutomationTestBase
{
    friend FLambdaCommand;

public:
    TArray<IOnlineSubsystemPtr> RegisteredSubsystems;
    TMap<FDelegateHandle, TFunction<void()>> RegisteredDelegates;
    TMap<FDelegateHandle, bool> FireOnceFlags;
    TArray<TTuple<TFunction<bool()>, TFunction<void()>>> WaitingOnConditions;

    FAsyncHotReloadableAutomationTestBase(const FString &InName, const bool bInComplexTask)
        : FHotReloadableAutomationTestBase(InName, bInComplexTask)
        , RegisteredSubsystems()
        , RegisteredDelegates()
        , FireOnceFlags()
        , WaitingOnConditions()
    {
    }
    void WhenCondition(TFunction<bool()> Condition, TFunction<void()> OnContinue);

protected:
    virtual void RunAsyncTest(const FOnDone &OnDone) = 0;
};

}

namespace Redpoint::EOS::Tests::TestUtilities
{
REDPOINT_EOS_FILE_NS_EXPORT(1902792364, Redpoint::EOS::Tests::TestUtilities, FAsyncHotReloadableAutomationTestBase)
}

REDPOINT_EOS_CODE_GUARD_END()