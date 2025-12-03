// Copyright June Rhodes. All Rights Reserved.

#include "Interfaces/OnlineSessionInterface.h"
#include "Misc/AutomationTest.h"
#include "OnlineSessionSettings.h"
#include "TestHelpers.h"

IMPLEMENT_MANAGED_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_OnlineSessionInterface_CanRecreateSessionWithSameName,
    "Redpoint.EOS.OnlineSessionInterface.CanRecreateSessionWithSameName",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter,
    FOnlineSubsystemEOS_OnlineSessionInterface_CanRecreateSessionWithSameName_Manager);

class FOnlineSubsystemEOS_OnlineSessionInterface_CanRecreateSessionWithSameName_Manager
    : public TSharedFromThis<FOnlineSubsystemEOS_OnlineSessionInterface_CanRecreateSessionWithSameName_Manager>
{
public:
    UE_NONCOPYABLE(FOnlineSubsystemEOS_OnlineSessionInterface_CanRecreateSessionWithSameName_Manager);
    virtual ~FOnlineSubsystemEOS_OnlineSessionInterface_CanRecreateSessionWithSameName_Manager() = default;

    typedef FOnlineSubsystemEOS_OnlineSessionInterface_CanRecreateSessionWithSameName_Manager TThisClass;

    class FOnlineSubsystemEOS_OnlineSessionInterface_CanRecreateSessionWithSameName *T;
    FMultiplayerScenarioInstance Host;
    TWeakPtr<IOnlineSession, ESPMode::ThreadSafe> HostSessionWk;
    TFunction<void()> OnDone;
    bool bShuttingDown;
    bool bDidInitialCreate;

    void Shutdown()
    {
        if (!this->bShuttingDown)
        {
            this->bShuttingDown = true;
            this->Start_DestroySession();
        }
    }

    FDelegateHandle SessionCreateHandle;
    void Start_CreateSession()
    {
        UE_LOG(LogRedpointEOSTests, Verbose, TEXT("CanRecreateSessionWithSameName: Starting CreateSession operation"));

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
        UE_LOG(LogRedpointEOSTests, Verbose, TEXT("CanRecreateSessionWithSameName: Handling CreateSession response"));

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

        if (this->bDidInitialCreate)
        {
            // Test finished.
            this->Shutdown();
            return;
        }

        this->bDidInitialCreate = true;
        this->Start_DestroySession();
    }

    FDelegateHandle DestroySessionHandle;
    void Start_DestroySession()
    {
        UE_LOG(LogRedpointEOSTests, Verbose, TEXT("CanRecreateSessionWithSameName: Starting DestroySession operation"));

        auto HostSession = this->HostSessionWk.Pin();
        if (!T->TestTrue("Host session interface is valid", HostSession.IsValid()))
        {
            this->OnDone();
            return;
        }

        this->DestroySessionHandle = HostSession->AddOnDestroySessionCompleteDelegate_Handle(
            FOnDestroySessionCompleteDelegate::CreateSP(this, &TThisClass::Handle_DestroySession));
        if (!T->TestTrue("Destroy session succeeded", HostSession->DestroySession(FName(TEXT("TestSession")))))
        {
            HostSession->ClearOnDestroySessionCompleteDelegate_Handle(this->DestroySessionHandle);
            this->OnDone();
        }
    }
    void Handle_DestroySession(FName SessionName, bool bWasSuccessful)
    {
        UE_LOG(LogRedpointEOSTests, Verbose, TEXT("CanRecreateSessionWithSameName: Handling DestroySession response"));

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

        if (this->bShuttingDown)
        {
            this->OnDone();
        }
        else
        {
            // Start recreate.
            this->Start_CreateSession();
        }
    }

    FOnlineSubsystemEOS_OnlineSessionInterface_CanRecreateSessionWithSameName_Manager(
        class FOnlineSubsystemEOS_OnlineSessionInterface_CanRecreateSessionWithSameName *InT,
        const FMultiplayerScenarioInstance &InHost,
        const TFunction<void()> &InOnDone)
        : T(InT)
        , Host(InHost)
        , HostSessionWk(InHost.Subsystem.Pin()->GetSessionInterface())
        , OnDone(InOnDone)
        , bShuttingDown(false)
        , bDidInitialCreate(false)
        , SessionCreateHandle()
        , DestroySessionHandle()
    {
    }
};

void FOnlineSubsystemEOS_OnlineSessionInterface_CanRecreateSessionWithSameName::CreateManagingInstance(
    const FOnDone &OnDone,
    const TFunction<
        void(const TSharedRef<FOnlineSubsystemEOS_OnlineSessionInterface_CanRecreateSessionWithSameName_Manager> &)>
        &OnInstanceCreated)
{
    FInstanceFactory::CreateInstances(
        this,
        TArray<int32>{1},
        OnDone,
        FInstanceFactory::EFlag::None,
        [this, OnInstanceCreated](TArray<FMultiplayerScenarioInstance> Instances, const FOnDone &OnDone) {
            auto Instance =
                MakeShared<FOnlineSubsystemEOS_OnlineSessionInterface_CanRecreateSessionWithSameName_Manager>(
                    this,
                    Instances[0],
                    OnDone);
            OnInstanceCreated(Instance);
            Instance->Start_CreateSession();
        });
}
