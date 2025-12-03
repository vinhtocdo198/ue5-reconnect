// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Async/TaskGraphInterfaces.h"
#include "Containers/Map.h"
#include "OnlineSubsystemRedpointEOS/Shared/EOSStats.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include <functional>

/**
 * This should be used when callbacks where the callback lambda can not be transported
 * across the native boundary (such as Java callbacks on Android).
 *
 * It is expected that this type will be instantiated as a global variable.
 */
template <typename TRequest, typename TResponse> class FIdentifiableCallbackDispatcher
{
private:
    int32 NextDispatchId = 1000;
    TMap<int32, TFunction<void(const TResponse &InResponse)>> ResponseCallbacks;

public:
    void Dispatch(
        const TRequest &InRequest,
        TFunction<bool(int32 InDispatchId, const TRequest &InRequest)> InDispatch,
        TFunction<void(const TResponse &InResponse)> InCompleteCallback)
    {
        int32 DispatchId = NextDispatchId++;
        ResponseCallbacks.Add(DispatchId, InCompleteCallback);
        if (!InDispatch(DispatchId, InRequest))
        {
            ResponseCallbacks.Remove(DispatchId);
        }
    }

    void ReceiveResponse(int32 InDispatchId, const TResponse &InResponse)
    {
        FSimpleDelegateGraphTask::CreateAndDispatchWhenReady(
            FSimpleDelegateGraphTask::FDelegate::CreateLambda([=]() {
                checkf(
                    ResponseCallbacks.Contains(InDispatchId),
                    TEXT("Expected dispatch callback to exist (make sure this isn't a double call either)!"));
                ResponseCallbacks[InDispatchId](InResponse);
                ResponseCallbacks.Remove(InDispatchId);
            }),
            GET_STATID(STAT_EOSIdentifiableCallbackDispatcher),
            nullptr,
            ENamedThreads::GameThread);
    }
};