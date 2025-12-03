// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Containers/Ticker.h"
#include "Delegates/Delegate.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "RedpointEOSAuth/Logging.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "Templates/SharedPointer.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(231184859, Redpoint::EOS::Auth)
{
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::API;

/**
 * A helper class for implementing credential obtainment, where the implementation can be shared between the
 * authentication graph and IOnlineExternalCredentials::Refresh.
 */
template <typename TClass, typename TResult> class FAuthenticationCredentialObtainer : public TSharedFromThis<TClass>
{
public:
    typedef TDelegate<void(const FError &ErrorCode, TResult Result)> FOnCredentialObtained;

private:
    TSharedPtr<TClass> SelfReference;
    FOnCredentialObtained Callback;
    TSharedPtr<FAuthenticationGraphState> State;

protected:
    void Done(bool bWasSuccessful, TResult Result)
    {
        this->Callback.ExecuteIfBound(
            bWasSuccessful
                ? Errors::Success()
                : Errors::UnexpectedError(ANSI_TO_TCHAR(__FUNCTION__), TEXT("Credential obtainment failed.")),
            Result);
        this->SelfReference = nullptr;
    }

    void Done(const FError &ErrorCode, TResult Result)
    {
        this->Callback.ExecuteIfBound(ErrorCode, Result);
        this->SelfReference = nullptr;
    }

    void EmitError(const FString &ErrorMessage, const FString &HumanSafeErrorMessage = TEXT(""))
    {
        UE_LOG(LogRedpointEOSAuth, Error, TEXT("%s"), *ErrorMessage);
        if (State.IsValid())
        {
            State->ErrorMessages.Add(HumanSafeErrorMessage.IsEmpty() ? ErrorMessage : HumanSafeErrorMessage);
        }
    }

public:
    FAuthenticationCredentialObtainer(const FOnCredentialObtained &InCallback)
        : SelfReference()
        , Callback(InCallback)
        , State()
    {
    }
    UE_NONCOPYABLE(FAuthenticationCredentialObtainer);
    virtual ~FAuthenticationCredentialObtainer() = default;

    /** Start the asynchronous operation. Return true if Tick should run, or false on failure to start. */
    virtual bool Init(UWorld *World, int32 LocalUserNum) = 0;

    /** Tick the asynchronous operation. Optional. Return false when the task has completed (you should call Done here
     * or in an event handler you registered in Init). */
    virtual bool Tick(float DeltaSeconds)
    {
        return false;
    }

    template <typename... TArgs>
    static void StartFromAuthenticationGraph(
        const TSharedRef<FAuthenticationGraphState> &InState,
        FOnCredentialObtained OnDone,
        TArgs... Args)
    {
        UWorld *World = InState->GetWorld();

        TSharedPtr<TClass> Task = MakeShared<TClass>(OnDone, Args...);
        Task->State = InState;
        Task->SelfReference = Task;
        if (!Task->Init(!IsValid(World) ? nullptr : World, InState->LocalUserNum))
        {
            Task->SelfReference = nullptr;
            return;
        }

        FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateSP(Task.ToSharedRef(), &TClass::Tick), 0);
    }

    template <typename... TArgs>
    static void StartFromCredentialRefresh(
        const TSoftObjectPtr<UWorld> &InWorld,
        int32 LocalUserNum,
        FOnCredentialObtained OnDone,
        TArgs... Args)
    {
        TSharedPtr<TClass> Task = MakeShared<TClass>(OnDone, Args...);
        Task->SelfReference = Task;
        if (!Task->Init(InWorld.IsValid() ? InWorld.Get() : nullptr, LocalUserNum))
        {
            Task->SelfReference = nullptr;
            return;
        }

        FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateSP(Task.ToSharedRef(), &TClass::Tick), 0);
    }
};

}

namespace Redpoint::EOS::Auth
{
REDPOINT_EOS_FILE_NS_EXPORT(231184859, Redpoint::EOS::Auth, FAuthenticationCredentialObtainer)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()