// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSCore/Utils/ExclusiveAsyncOperationQueue.h"

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSCore/Logging.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(460204929, Redpoint::EOS::Core::Utils)
{
using namespace ::Redpoint::EOS::Core;

void FExclusiveAsyncOperationQueue::ProcessOperationQueue()
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    if (this->bCurrentOperationRunning)
    {
        // Already running something.
        return;
    }

    if (this->Operations.IsEmpty())
    {
        // Both the queue and current are empty, nothing to do.
        return;
    }

    // Get the next still bound operation or empty the queue.
    do
    {
        this->CurrentOperation = this->Operations[0];
        this->Operations.RemoveAt(0);
        if (!this->CurrentOperation.IsBound())
        {
            UE_LOG(
                LogRedpointEOSCore,
                Warning,
                TEXT("Operation in FExclusiveAsyncOperationQueue was no longer bound. Skipping!"));
        }
    } while (!this->CurrentOperation.IsBound() && !this->Operations.IsEmpty());

    if (!this->CurrentOperation.IsBound())
    {
        // The queue was emptied and didn't have any operations still bound.
        UE_LOG(
            LogRedpointEOSCore,
            Warning,
            TEXT(
                "Queue emptied in FExclusiveAsyncOperationQueue due to all pending operations no longer being bound!"));
        return;
    }

    this->CurrentOperationGuid = FGuid::NewGuid();
    this->bCurrentOperationRunning = true;
    this->CurrentOperation.Execute(
        FOnDone::CreateSP(this, &FExclusiveAsyncOperationQueue::HandleOnDone, this->CurrentOperationGuid));
}

void FExclusiveAsyncOperationQueue::HandleOnDone(FGuid ExecutingOperationGuid)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    if (this->CurrentOperationGuid != ExecutingOperationGuid)
    {
        // This operation is calling back multiple times.
        UE_LOG(
            LogRedpointEOSCore,
            Warning,
            TEXT("Operation in FExclusiveAsyncOperationQueue attempted to callback multiple times and was ignored!"));
        return;
    }

    this->CurrentOperationGuid = FGuid::NewGuid();
    this->bCurrentOperationRunning = false;
    this->ProcessOperationQueue();
}

FExclusiveAsyncOperationQueue::FExclusiveAsyncOperationQueue()
    : CurrentOperation()
    , CurrentOperationGuid(FGuid::NewGuid())
    , bCurrentOperationRunning(false)
    , Operations()
{
}

void FExclusiveAsyncOperationQueue::AddToQueue(const FOperation &InOperation)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    this->Operations.Add(InOperation);
    this->ProcessOperationQueue();
}

int FExclusiveAsyncOperationQueue::GetInProgressAndRemainingCount() const
{
    return this->Operations.Num() + (this->bCurrentOperationRunning ? 1 : 0);
}

}

REDPOINT_EOS_CODE_GUARD_END()