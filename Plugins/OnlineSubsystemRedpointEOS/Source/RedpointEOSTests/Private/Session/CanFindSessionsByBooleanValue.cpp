// Copyright June Rhodes. All Rights Reserved.

#include "Containers/Ticker.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Misc/AutomationTest.h"
#include "OnlineSessionSettings.h"
#include "TestHelpers.h"

IMPLEMENT_MANAGED_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_OnlineSessionInterface_CanFindSessionsByBooleanValue,
    "Redpoint.EOS.OnlineSessionInterface.CanFindSessionsByBooleanValue",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter,
    FOnlineSubsystemEOS_OnlineSessionInterface_CanFindSessionsByBooleanValue_Manager);

class FOnlineSubsystemEOS_OnlineSessionInterface_CanFindSessionsByBooleanValue_Manager
    : public TSharedFromThis<FOnlineSubsystemEOS_OnlineSessionInterface_CanFindSessionsByBooleanValue_Manager>
{
public:
    UE_NONCOPYABLE(FOnlineSubsystemEOS_OnlineSessionInterface_CanFindSessionsByBooleanValue_Manager);
    virtual ~FOnlineSubsystemEOS_OnlineSessionInterface_CanFindSessionsByBooleanValue_Manager() = default;

    typedef FOnlineSubsystemEOS_OnlineSessionInterface_CanFindSessionsByBooleanValue_Manager TThisClass;

    class FOnlineSubsystemEOS_OnlineSessionInterface_CanFindSessionsByBooleanValue *T;
    FMultiplayerScenarioInstance Host;
    FMultiplayerScenarioInstance Client;
    TWeakPtr<IOnlineSession, ESPMode::ThreadSafe> HostSessionWk;
    TWeakPtr<IOnlineSession, ESPMode::ThreadSafe> ClientSessionWk;
    TFunction<void()> OnDone;
    FString TargetSessionId;
    int FindAttempts;
    TSharedPtr<FOnlineSessionSearch> Search;
    bool bShuttingDown;

    FDelegateHandle SessionCreateHandle;
    void Start_CreateSession()
    {
        UE_LOG(LogRedpointEOSTests, Verbose, TEXT("CanFindSessionsByBooleanValue: Starting CreateSession operation"));

        auto HostSession = this->HostSessionWk.Pin();
        if (!T->TestTrue("Host session interface is valid", HostSession.IsValid()))
        {
            this->Shutdown();
            return;
        }

        TSharedRef<FOnlineSessionSettings> SessionSettings = MakeShared<FOnlineSessionSettings>();
        SessionSettings->NumPublicConnections = 4;
        SessionSettings->bShouldAdvertise = true;
        SessionSettings->bUsesPresence = false;
        SessionSettings->Settings.Add(
            FName(TEXT("BooleanValue")),
            FOnlineSessionSetting(true, EOnlineDataAdvertisementType::ViaOnlineService));

        this->SessionCreateHandle = HostSession->AddOnCreateSessionCompleteDelegate_Handle(
            FOnCreateSessionCompleteDelegate::CreateSP(this, &TThisClass::Handle_CreateSession));
        if (!T->TestTrue(
                "CreateSession call succeeded",
                HostSession->CreateSession(*this->Host.UserId, FName(TEXT("TestSession")), *SessionSettings)))
        {
            HostSession->ClearOnCreateSessionCompleteDelegate_Handle(this->SessionCreateHandle);
            this->Shutdown();
        }
    }
    void Handle_CreateSession(FName SessionName, bool bWasSuccessful)
    {
        UE_LOG(LogRedpointEOSTests, Verbose, TEXT("CanFindSessionsByBooleanValue: Handling CreateSession response"));

        auto HostSession = this->HostSessionWk.Pin();
        if (!T->TestTrue("Host session interface is valid", HostSession.IsValid()))
        {
            this->Shutdown();
            return;
        }

        HostSession->ClearOnCreateSessionCompleteDelegate_Handle(this->SessionCreateHandle);

        if (!T->TestTrue("Session was created", bWasSuccessful))
        {
            this->Shutdown();
            return;
        }

        this->TargetSessionId = HostSession->GetNamedSession(FName(TEXT("TestSession")))->GetSessionIdStr();

        this->Start_FindSessions();
    }

    FDelegateHandle SessionFindHandle;
    void Start_FindSessions()
    {
        UE_LOG(LogRedpointEOSTests, Verbose, TEXT("CanFindSessionsByBooleanValue: Starting FindSessions operation"));

        auto ClientSession = this->ClientSessionWk.Pin();
        if (!T->TestTrue("Client session interface is valid", ClientSession.IsValid()))
        {
            this->Shutdown();
            return;
        }

        this->Search = MakeShared<FOnlineSessionSearch>();
        this->Search->QuerySettings.SearchParams.Empty();
        this->Search->QuerySettings.Set(FName(TEXT("BooleanValue")), true, EOnlineComparisonOp::Equals);
        FOnlineSessionSearchParam EmptyParam(0);
        EmptyParam.Data = FVariantData();
        EmptyParam.ComparisonOp = EOnlineComparisonOp::Equals;
        EmptyParam.ID = 0;
        this->Search->QuerySettings.SearchParams.Add(TEXT("__EOS_bListening"), EmptyParam);

        this->SessionFindHandle = ClientSession->AddOnFindSessionsCompleteDelegate_Handle(
            FOnFindSessionsCompleteDelegate::CreateSP(this, &TThisClass::Handle_FindSessions));
        if (!T->TestTrue(
                "FindSessions call succeeded",
                ClientSession->FindSessions(*this->Client.UserId, this->Search.ToSharedRef())))
        {
            ClientSession->ClearOnFindSessionsCompleteDelegate_Handle(this->SessionFindHandle);
            this->Shutdown();
            return;
        }

        this->FindAttempts++;
    }
    void Handle_FindSessions(bool bWasSuccessful)
    {
        UE_LOG(
            LogRedpointEOSTests,
            Verbose,
            TEXT("CanFindSessionsByBooleanValue: Handling FindSessions response (attempt %d)"),
            this->FindAttempts);

        auto ClientSession = this->ClientSessionWk.Pin();
        if (!T->TestTrue("Client session interface is valid", ClientSession.IsValid()))
        {
            this->Shutdown();
            return;
        }

        ClientSession->ClearOnFindSessionsCompleteDelegate_Handle(this->SessionFindHandle);

        if (!T->TestTrue("Find sessions operation succeeded", bWasSuccessful))
        {
            this->Shutdown();
            return;
        }

        TArray<FOnlineSessionSearchResult> MatchingResults;
        for (const auto &SearchResult : Search->SearchResults)
        {
            if (SearchResult.Session.GetSessionIdStr() == TargetSessionId)
            {
                MatchingResults.Add(SearchResult);
            }
        }

        if (MatchingResults.Num() == 0)
        {
            if (this->FindAttempts < 30)
            {
                UE_LOG(
                    LogRedpointEOSTests,
                    Verbose,
                    TEXT("CanFindSessionsByBooleanValue: No matching session yet, rescheduling..."));
                FTSTicker::GetCoreTicker().AddTicker(
                    FTickerDelegate::CreateLambda([this](float DeltaTime) {
                        this->Start_FindSessions();
                        return false;
                    }),
                    1.0f);
            }
            else
            {
                T->TestTrue("Failed to find session within 30 attempts", false);
                this->Shutdown();
            }
            return;
        }

        UE_LOG(LogRedpointEOSTests, Verbose, TEXT("CanFindSessionsByBooleanValue: Found matching session!"));

        // Otherwise, we found our matching session.
        this->Shutdown();
    }

    void Shutdown()
    {
        if (!this->bShuttingDown)
        {
            this->bShuttingDown = true;
            this->Start_DestroySession();
        }
    }

    FDelegateHandle DestroySessionHandle;
    void Start_DestroySession()
    {
        UE_LOG(
            LogRedpointEOSTests,
            Verbose,
            TEXT("CanFindSessionWeWereRegisteredIn: Starting DestroySession operation"));

        auto HostSession = this->HostSessionWk.Pin();
        if (!T->TestTrue("Host session interface is valid", HostSession.IsValid()))
        {
            this->OnDone();
            return;
        }

        this->DestroySessionHandle = HostSession->AddOnDestroySessionCompleteDelegate_Handle(
            FOnDestroySessionCompleteDelegate::CreateSP(this, &TThisClass::Handle_DestroySession));
        if (!T->TestTrue("Unregister player succeeded", HostSession->DestroySession(FName(TEXT("TestSession")))))
        {
            HostSession->ClearOnDestroySessionCompleteDelegate_Handle(this->DestroySessionHandle);
            this->OnDone();
        }
    }
    void Handle_DestroySession(FName SessionName, bool bWasSuccessful)
    {
        UE_LOG(
            LogRedpointEOSTests,
            Verbose,
            TEXT("CanFindSessionWeWereRegisteredIn: Handling DestroySession response"));

        auto HostSession = this->HostSessionWk.Pin();
        if (!T->TestTrue("Host session interface is valid", HostSession.IsValid()))
        {
            this->OnDone();
            return;
        }

        HostSession->ClearOnDestroySessionCompleteDelegate_Handle(this->DestroySessionHandle);

        if (!T->TestTrue("Session was destroyed", bWasSuccessful))
        {
            this->OnDone();
            return;
        }

        this->OnDone();
    }

    FOnlineSubsystemEOS_OnlineSessionInterface_CanFindSessionsByBooleanValue_Manager(
        class FOnlineSubsystemEOS_OnlineSessionInterface_CanFindSessionsByBooleanValue *InT,
        const FMultiplayerScenarioInstance &InHost,
        const FMultiplayerScenarioInstance &InClient,
        const TFunction<void()> &InOnDone)
        : T(InT)
        , Host(InHost)
        , Client(InClient)
        , HostSessionWk(InHost.Subsystem.Pin()->GetSessionInterface())
        , ClientSessionWk(InClient.Subsystem.Pin()->GetSessionInterface())
        , OnDone(InOnDone)
        , TargetSessionId()
        , FindAttempts(0)
        , Search(nullptr)
        , bShuttingDown(false)
        , SessionCreateHandle()
        , SessionFindHandle()
        , DestroySessionHandle()
    {
    }
};

void FOnlineSubsystemEOS_OnlineSessionInterface_CanFindSessionsByBooleanValue::CreateManagingInstance(
    const FOnDone &OnDone,
    const TFunction<
        void(const TSharedRef<FOnlineSubsystemEOS_OnlineSessionInterface_CanFindSessionsByBooleanValue_Manager> &)>
        &OnInstanceCreated)
{
    FInstanceFactory::CreateInstances(
        this,
        TArray<int32>{1, 1},
        OnDone,
        FInstanceFactory::EFlag::None,
        [this, OnInstanceCreated](TArray<FMultiplayerScenarioInstance> Instances, const FOnDone &OnDone) {
            auto Instance =
                MakeShared<FOnlineSubsystemEOS_OnlineSessionInterface_CanFindSessionsByBooleanValue_Manager>(
                    this,
                    Instances[0],
                    Instances[1],
                    OnDone);
            OnInstanceCreated(Instance);
            Instance->Start_CreateSession();
        });
}
