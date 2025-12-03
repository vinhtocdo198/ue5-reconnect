// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Delegates/Delegate.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Utils/Future.h"
#include "Templates/Function.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(460204929, Redpoint::EOS::Core::Utils)
{
using namespace ::Redpoint::EOS::API;

class REDPOINTEOSCORE_API FExclusiveAsyncOperationQueue : public TSharedFromThis<FExclusiveAsyncOperationQueue>
{
public:
    typedef TDelegate<void()> FOnDone;
    typedef TDelegate<void(const FOnDone &InOnDone)> FOperation;

private:
    FOperation CurrentOperation;
    FGuid CurrentOperationGuid;
    bool bCurrentOperationRunning;
    TArray<FOperation> Operations;

    void ProcessOperationQueue();
    void HandleOnDone(FGuid ExecutingOperationGuid);

public:
    FExclusiveAsyncOperationQueue();
    UE_NONCOPYABLE(FExclusiveAsyncOperationQueue);
    virtual ~FExclusiveAsyncOperationQueue() = default;

    void AddToQueue(const FOperation &InOperation);

    template <typename Shared, typename Return>
    TFuture<Return> AddToQueue(
        TSharedRef<Shared> SP,
        TFunction<Return> Default,
        const TFunction<TFuture<Return>()> &Operation)
    {
        REDPOINT_EOS_USE_LLM_TAG(Core);

        auto Promise = MakeShared<TPromise<Return>>();
        auto Wk = TWeakPtr<Shared>(SP);
        this->AddToQueue(FOperation::CreateLambda([Wk, Default, Operation, Promise](const FOnDone &OnDone) {
            auto Pinned = Wk.Pin();
            if (Pinned.IsValid())
            {
                Operation().Next([Promise, OnDone](const Return &R) {
                    OnDone.ExecuteIfBound();
                    Promise->SetValue(R);
                });
            }
            else
            {
                OnDone.ExecuteIfBound();
                Promise->SetValue(Default());
            }
        }));
        return Promise->GetFuture();
    }

    template <typename Shared>
    TFuture<FError> AddToQueue(TSharedRef<Shared> SP, const TFunction<TFuture<FError>()> &Operation)
    {
        REDPOINT_EOS_USE_LLM_TAG(Core);

        auto Promise = MakeShared<TPromise<FError>>();
        auto Wk = TWeakPtr<Shared>(SP);
        this->AddToQueue(FOperation::CreateLambda([Wk, Operation, Promise](const FOnDone &OnDone) {
            auto Pinned = Wk.Pin();
            if (Pinned.IsValid())
            {
                Operation().Next([Promise, OnDone](const FError &R) {
                    OnDone.ExecuteIfBound();
                    Promise->SetValue(R);
                });
            }
            else
            {
                OnDone.ExecuteIfBound();
                Promise->SetValue(Errors::Canceled(
                    ANSI_TO_TCHAR(__FUNCTION__),
                    TEXT("The implementor of the call went out of scope during execution.")));
            }
        }));
        return Promise->GetFuture();
    }

    template <typename Shared>
    TFuture<void> AddToQueue(TSharedRef<Shared> SP, const TFunction<TFuture<void>()> &Operation)
    {
        REDPOINT_EOS_USE_LLM_TAG(Core);

        auto Promise = MakeShared<TPromise<void>>();
        auto Wk = TWeakPtr<Shared>(SP);
        this->AddToQueue(FOperation::CreateLambda([Wk, Operation, Promise](const FOnDone &OnDone) {
            auto Pinned = Wk.Pin();
            if (Pinned.IsValid())
            {
                Operation().Next([Promise, OnDone]() {
                    OnDone.ExecuteIfBound();
                    Promise->SetValue();
                });
            }
            else
            {
                OnDone.ExecuteIfBound();
                Promise->SetValue();
            }
        }));
        return Promise->GetFuture();
    }

    template <typename Shared> auto AddToQueue(Shared *SP, const auto &Operation)
    {
        REDPOINT_EOS_USE_LLM_TAG(Core);

        static_assert(
            std::is_base_of<TSharedFromThis<Shared>, Shared>::value,
            "Expected Shared to implement TSharedFromThis<>.");
        return this->AddToQueue(SP->AsShared(), Operation);
    }

    int GetInProgressAndRemainingCount() const;
};

}

namespace Redpoint::EOS::Core::Utils
{
REDPOINT_EOS_FILE_NS_EXPORT(460204929, Redpoint::EOS::Core::Utils, FExclusiveAsyncOperationQueue)
}

REDPOINT_EOS_CODE_GUARD_END()