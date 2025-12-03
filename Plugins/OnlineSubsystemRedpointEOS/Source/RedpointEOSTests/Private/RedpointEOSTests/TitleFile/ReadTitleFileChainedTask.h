// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskContext.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(116812569, Redpoint::EOS::Tests::TitleFile)
{
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

class FReadTitleFileChainedTask : public FChainedTask, public TSharedFromThis<FReadTitleFileChainedTask>
{
private:
    int32 InstanceId;
    FString Filename;
    FDelegateHandle OnReadFileHandle;

public:
    FReadTitleFileChainedTask(int32 InInstanceId, FString InFilename);
    UE_NONCOPYABLE(FReadTitleFileChainedTask);
    virtual ~FReadTitleFileChainedTask() override = default;

    virtual void ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone) override;

private:
    void OnReadFileComplete(bool bWasSuccessful, const FString &, FChainedTaskContextRef Context, FOnComplete OnDone);
};

}

namespace Redpoint::EOS::Tests::TitleFile
{
REDPOINT_EOS_FILE_NS_EXPORT(116812569, Redpoint::EOS::Tests::TitleFile, FReadTitleFileChainedTask)
}

REDPOINT_EOS_CODE_GUARD_END()