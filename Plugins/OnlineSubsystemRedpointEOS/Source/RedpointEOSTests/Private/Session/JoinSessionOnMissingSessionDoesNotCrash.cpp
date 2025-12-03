// Copyright June Rhodes. All Rights Reserved.

#include "Interfaces/OnlineSessionInterface.h"
#include "Misc/AutomationTest.h"
#include "OnlineSessionSettings.h"
#include "TestHelpers.h"

IMPLEMENT_MANAGED_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_OnlineSessionInterface_JoinSessionOnMissingSessionDoesNotCrash,
    "Redpoint.EOS.OnlineSessionInterface.JoinSessionOnMissingSessionDoesNotCrash",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter,
    FOnlineSubsystemEOS_OnlineSessionInterface_JoinSessionOnMissingSessionDoesNotCrash_Manager);

class FOnlineSubsystemEOS_OnlineSessionInterface_JoinSessionOnMissingSessionDoesNotCrash_Manager
    : public TSharedFromThis<FOnlineSubsystemEOS_OnlineSessionInterface_JoinSessionOnMissingSessionDoesNotCrash_Manager>
{
public:
    UE_NONCOPYABLE(FOnlineSubsystemEOS_OnlineSessionInterface_JoinSessionOnMissingSessionDoesNotCrash_Manager);
    virtual ~FOnlineSubsystemEOS_OnlineSessionInterface_JoinSessionOnMissingSessionDoesNotCrash_Manager() = default;

    typedef FOnlineSubsystemEOS_OnlineSessionInterface_JoinSessionOnMissingSessionDoesNotCrash_Manager TThisClass;

    class FOnlineSubsystemEOS_OnlineSessionInterface_JoinSessionOnMissingSessionDoesNotCrash *T;
    FMultiplayerScenarioInstance Host;
    TWeakPtr<IOnlineSession, ESPMode::ThreadSafe> HostSessionWk;
    TFunction<void()> OnDone;

    void Start_FindSessionById()
    {
        UE_LOG(
            LogRedpointEOSTests,
            Verbose,
            TEXT("JoinSessionOnMissingSessionDoesNotCrash: Starting Start_FindSessionById operation"));

        auto HostSession = this->HostSessionWk.Pin();
        if (!T->TestTrue("Host session interface is valid", HostSession.IsValid()))
        {
            this->Shutdown();
            return;
        }

        if (!T->TestTrue(
                "FindSessionById call succeeded",
                HostSession->FindSessionById(
                    *this->Host.UserId,
                    *HostSession->CreateSessionIdFromString("c7055aea5c644e7d8f5898464b7a5305"),
                    *this->Host.UserId,
                    FOnSingleSessionResultCompleteDelegate::CreateSP(this, &TThisClass::Handle_FindSessionById))))
        {
            this->Shutdown();
            return;
        }
    }
    void Handle_FindSessionById(int32 LocalUserNum, bool bWasSuccessful, const FOnlineSessionSearchResult &Result)
    {
        UE_LOG(
            LogRedpointEOSTests,
            Verbose,
            TEXT("JoinSessionOnMissingSessionDoesNotCrash: Handling FindSessionById response"));

        auto HostSession = this->HostSessionWk.Pin();
        if (!T->TestTrue("Host session interface is valid", HostSession.IsValid()))
        {
            this->Shutdown();
            return;
        }

        if (!T->TestFalse("FindSessionById should fail", bWasSuccessful))
        {
            this->Shutdown();
            return;
        }

        // We ignore the success result here and intentionally try to use a bad result with JoinSession.
        this->Start_JoinSession(Result);
    }

    FDelegateHandle JoinSessionHandle;
    void Start_JoinSession(const FOnlineSessionSearchResult &BadResult)
    {
        UE_LOG(
            LogRedpointEOSTests,
            Verbose,
            TEXT("JoinSessionOnMissingSessionDoesNotCrash: Starting Start_JoinSession operation"));

        auto HostSession = this->HostSessionWk.Pin();
        if (!T->TestTrue("Host session interface is valid", HostSession.IsValid()))
        {
            this->Shutdown();
            return;
        }

        this->JoinSessionHandle = HostSession->AddOnJoinSessionCompleteDelegate_Handle(
            FOnJoinSessionCompleteDelegate::CreateSP(this, &TThisClass::Handle_JoinSession));
        if (T->TestTrue(
                "Expected JoinSession call to start",
                HostSession->JoinSession(*this->Host.UserId, NAME_GameSession, BadResult)))
        {
            HostSession->ClearOnJoinSessionCompleteDelegate_Handle(this->JoinSessionHandle);
            this->Shutdown();
        }
    }
    void Handle_JoinSession(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult)
    {
        UE_LOG(LogRedpointEOSTests, Verbose, TEXT("CanFindSessionsByBooleanValue: Handling JoinSession response"));

        auto HostSession = this->HostSessionWk.Pin();
        if (!T->TestTrue("Host session interface is valid", HostSession.IsValid()))
        {
            this->Shutdown();
            return;
        }

        HostSession->ClearOnJoinSessionCompleteDelegate_Handle(this->JoinSessionHandle);

        if (!T->TestTrue("JoinSession is expected to fail", JoinResult != EOnJoinSessionCompleteResult::Success))
        {
            this->Shutdown();
            return;
        }

        // Test passed.
        this->Shutdown();
        return;
    }

    void Shutdown()
    {
        this->OnDone();
    }

    FOnlineSubsystemEOS_OnlineSessionInterface_JoinSessionOnMissingSessionDoesNotCrash_Manager(
        class FOnlineSubsystemEOS_OnlineSessionInterface_JoinSessionOnMissingSessionDoesNotCrash *InT,
        const FMultiplayerScenarioInstance &InHost,
        const TFunction<void()> &InOnDone)
        : T(InT)
        , Host(InHost)
        , HostSessionWk(InHost.Subsystem.Pin()->GetSessionInterface())
        , OnDone(InOnDone)
    {
    }
};

void FOnlineSubsystemEOS_OnlineSessionInterface_JoinSessionOnMissingSessionDoesNotCrash::CreateManagingInstance(
    const FOnDone &OnDone,
    const TFunction<void(
        const TSharedRef<FOnlineSubsystemEOS_OnlineSessionInterface_JoinSessionOnMissingSessionDoesNotCrash_Manager> &)>
        &OnInstanceCreated)
{
    this->AddExpectedError(TEXT("sessions.invalid_session"), EAutomationExpectedErrorFlags::Contains);
    this->AddExpectedError(TEXT("eos.not_found"), EAutomationExpectedErrorFlags::Contains);

    FInstanceFactory::CreateInstances(
        this,
        TArray<int32>{1},
        OnDone,
        FInstanceFactory::EFlag::None,
        [this, OnInstanceCreated](TArray<FMultiplayerScenarioInstance> Instances, const FOnDone &OnDone) {
            auto Instance =
                MakeShared<FOnlineSubsystemEOS_OnlineSessionInterface_JoinSessionOnMissingSessionDoesNotCrash_Manager>(
                    this,
                    Instances[0],
                    OnDone);
            OnInstanceCreated(Instance);
            Instance->Start_FindSessionById();
        });
}
