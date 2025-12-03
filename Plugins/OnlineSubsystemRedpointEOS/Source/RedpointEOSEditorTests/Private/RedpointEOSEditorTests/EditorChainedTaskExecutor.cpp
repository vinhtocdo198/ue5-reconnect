// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditorTests/EditorChainedTaskExecutor.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "Editor.h"
#include "Editor/UnrealEdEngine.h"
#include "EngineUtils.h"
#include "Factories/WorldFactory.h"
#include "FileHelpers.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "LevelEditor.h"
#include "Misc/ConfigCacheIni.h"
#include "OnlineEngineInterfaceEOS.h"
#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "OnlineSubsystemRedpointEOSModule.h"
#include "OnlineSubsystemUtils.h"
#include "RedpointEOSCore/Utils/MultiOperation.h"
#include "RedpointEOSEditorTests/Logging.h"
#include "RedpointEOSEditorTests/TestMapName.h"
#include "RedpointEOSEditorTestsModule.h"
#include "RedpointEOSTestGameMode.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"
#include "RedpointEOSTests/TestUtilities/EOSConfigEASLogin.h"
#include "RedpointEOSTests/TestUtilities/RegisterOSSCallback.h"
#include "Tests/AutomationEditorCommon.h"
#include "UObject/GCObjectScopeGuard.h"
#include "UObject/Object.h"
#include "UObject/SavePackage.h"
#include "UnrealEdGlobals.h"
#include "UnrealEngine.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2606800861, Redpoint::EOS::Editor::Tests)
{
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
using namespace ::Redpoint::EOS::Editor::Tests;
using namespace ::Redpoint::EOS::Core::Utils;

bool FEditorChainedTaskExecutor::CreateMapIfNeeded(const FString &MapName)
{
    auto *Factory = NewObject<UWorldFactory>(GetTransientPackage(), UWorldFactory::StaticClass());
    FGCObjectScopeGuard FactoryGCGuard(Factory);

    auto &AssetToolsModule = FAssetToolsModule::GetModule();

    for (int i = 0; i < 3; i++)
    {
        if (FEditorFileUtils::IsMapPackageAsset(FString::Printf(TEXT("/Game/Developers/%s.%s"), *MapName, *MapName)))
        {
            return true;
        }

        FString DefaultAssetName;
        FString PackageNameToUse;
        AssetToolsModule.Get().CreateUniqueAssetName(
            FString::Printf(TEXT("/Game/Developers/%s"), *MapName),
            FString(),
            PackageNameToUse,
            DefaultAssetName);

        UObject *Map =
            AssetToolsModule.Get().CreateAsset(MapName, TEXT("/Game/Developers"), UWorld::StaticClass(), Factory);
        if (!IsValid(Map) || !IsValid(Map->GetPackage()))
        {
            UE_LOG(
                LogRedpointEOSEditorTests,
                Warning,
                TEXT("Failed to create map with name '%s', retrying..."),
                *MapName);
            continue;
        }

        TArray<UPackage *> PackagesToSave;
        PackagesToSave.Add(Map->GetPackage());
        UEditorLoadingAndSavingUtils::SavePackages(PackagesToSave, false);
        return true;
    }

    // If we get to here, we failed to create the map!
    UE_LOG(
        LogRedpointEOSEditorTests,
        Error,
        TEXT("Unable to create map with name '%s' and yet no map was returned by GetAssetByObjectPath. The test can "
             "not continue!"),
        *MapName);
    return false;
}

void FEditorChainedTaskExecutor::OnDoneRequested()
{
#if REDPOINT_EOS_UE_5_4_OR_LATER
    UE_LOG(LogRedpointEOSEditorTests, Verbose, TEXT("Test is finishing..."));
    auto Handle = MakeShared<FDelegateHandle>();
    auto ForceHold = this->AsShared();
    *Handle = FEditorDelegates::ShutdownPIE.Add(
        FEditorDelegates::FOnPIEEvent::FDelegate::CreateSPLambda(this, [this, Handle, ForceHold](const bool) {
            UE_LOG(
                LogRedpointEOSEditorTests,
                Verbose,
                TEXT("Editor reported play-in-editor has ended, finishing test..."));
            ((UOnlineEngineInterfaceEOS *)UOnlineEngineInterface::Get())->AutomationCredentials = nullptr;
            ((UEditorEngine *)GEngine)->UserEditedPlayWorldURL = TEXT("");
            this->OnDone();
            FEditorDelegates::ShutdownPIE.Remove(*Handle);
        }));
    UE_LOG(LogRedpointEOSEditorTests, Verbose, TEXT("Test is requesting end play-in-editor..."));
    GUnrealEd->RequestEndPlayMap();
#else
    GEngine->Exec(nullptr, TEXT("log LogRedpointEOSRooms off"));
    GEngine->Exec(nullptr, TEXT("log LogRedpointEOSPresence off"));
    GUnrealEd->RequestEndPlayMap();
    ((UOnlineEngineInterfaceEOS *)UOnlineEngineInterface::Get())->AutomationCredentials = nullptr;
    ((UEditorEngine *)GEngine)->UserEditedPlayWorldURL = TEXT("");
    GEngine->Exec(nullptr, TEXT("log LogRedpointEOSRooms reset"));
    GEngine->Exec(nullptr, TEXT("log LogRedpointEOSPresence reset"));
    this->OnDone();
#endif
}

void FEditorChainedTaskExecutor::OnStepComplete(
    bool bContinueTesting,
    TSharedRef<FChainedTaskContext> InContext,
    TSharedRef<FChainedTaskExecutor> InSelf,
    TSharedRef<ChainedTask::FChainedTask> InExecutedStep,
    int32 InTotalSteps)
{
    FChainedTaskExecutor::OnStepComplete(bContinueTesting, InContext, InSelf, InExecutedStep, InTotalSteps);
}

FEditorChainedTaskExecutor::FEditorChainedTaskExecutor(
    FAsyncHotReloadableAutomationTestBase *InTest,
    TFunction<void()> InOnDone,
    const TArray<int32> &InInstanceLayout,
    EPlayNetMode InPlayMode,
    const FString &InAdditionalLaunchParameters,
    EChainedTaskExecutorFlag InFlags)
    : FChainedTaskExecutor(InTest, InOnDone, InInstanceLayout, InFlags)
    , PlayMode(InPlayMode)
    , AdditionalLaunchParameters(InAdditionalLaunchParameters)
    , TimeoutAt()
{
}

void FEditorChainedTaskExecutor::Execute()
{
    checkf(!this->bExecuting, TEXT("Chained task already executing!"));
    TSharedRef<FChainedTaskExecutor> This = this->AsShared();
    if (this->Steps.Num() == 0)
    {
        this->Steps.Add(MakeShared<FLambdaChainedTask>(
            [](const FChainedTaskContextRef &, const FLambdaChainedTask::FOnComplete &TaskOnDone) {
                TaskOnDone.ExecuteIfBound(true);
            }));
    }
    TSharedRef<FChainedTaskContext> Context = MakeShareable(new FChainedTaskContext(This));

    FOnlineSubsystemRedpointEOSModule &OnlineSubsystemEOSModule =
        FModuleManager::Get().GetModuleChecked<FOnlineSubsystemRedpointEOSModule>(TEXT("OnlineSubsystemRedpointEOS"));
    OnlineSubsystemEOSModule.AutomationTestingConfigOverride = MakeShared<FEOSConfigEASLogin>();

    // Force set up of AES-GCM encryption for encryption testing.
    GConfig->SetString(
        TEXT("PacketHandlerComponents"),
        TEXT("EncryptionComponent"),
        TEXT("AESGCMHandlerComponent"),
        GEngineIni);

    bool bIsDedicatedServer = (Flags & EChainedTaskExecutorFlag::DedicatedServer) != EChainedTaskExecutorFlag::None;
    bool bIsEpicGames = (Flags & EChainedTaskExecutorFlag::EpicGames) != EChainedTaskExecutorFlag::None;

    TArray<FInstanceFactory::FEpicGamesAutomatedTestingCredential> EpicGamesCredentials;
    if (bIsEpicGames)
    {
        if (!FInstanceFactory::LoadEpicGamesAutomatedTestingCredentials(EpicGamesCredentials))
        {
            Test->AddWarning(TEXT("This test is being skipped, because EAS testing credentials are not available."));
            this->OnDone();
            return;
        }
    }

    int32 GlobalUserCount = 0;
    TArray<FInstanceFactory::FInstanceDescriptor> InstanceDescriptors;
    for (int i = 0; i < InstanceLayout.Num(); i++)
    {
        TArray<FInstanceFactory::FInstanceUserDescriptor> InstanceUserDescriptors;
        for (int u = 0; u < InstanceLayout[i]; u++)
        {
            FInstanceFactory::FInstanceUserDescriptor InstanceUserDescriptor;
            if (bIsDedicatedServer && i == 0 && u == 0)
            {
                InstanceUserDescriptor.GlobalUserNum = 0;
                InstanceUserDescriptor.LocalUserNum = 0;
                InstanceUserDescriptor.CredentialId = TEXT("");
                InstanceUserDescriptor.CredentialToken = TEXT("");
            }
            else
            {
                InstanceUserDescriptor.GlobalUserNum = GlobalUserCount++;
                InstanceUserDescriptor.LocalUserNum = u;
                if (bIsEpicGames)
                {
                    InstanceUserDescriptor.CredentialId =
                        EpicGamesCredentials[InstanceUserDescriptor.GlobalUserNum].Username;
                    InstanceUserDescriptor.CredentialToken =
                        EpicGamesCredentials[InstanceUserDescriptor.GlobalUserNum].Password;
                }
                else
                {
                    InstanceUserDescriptor.CredentialId =
                        FString::Printf(TEXT("CreateOnDemand:%s"), *Test->GetTestName());
                    InstanceUserDescriptor.CredentialToken =
                        FString::Printf(TEXT("%d"), TestHelpers::Port(InstanceUserDescriptor.GlobalUserNum));
                }
            }
            InstanceUserDescriptors.Add(InstanceUserDescriptor);
        }

        FInstanceFactory::FInstanceDescriptor InstanceDescriptor;
        InstanceDescriptor.InstanceId = i;
        if (bIsDedicatedServer && i == 0)
        {
            InstanceDescriptor.SubsystemName = FName(*FString::Printf(
                TEXT("%s_%s_AutomationDedicatedServer"),
                *Test->GetTestName(),
                *FString::Printf(TEXT("%d"), TestHelpers::Port(i))));
        }
        else
        {
            InstanceDescriptor.SubsystemName =
                bIsEpicGames ? FName(*FString::Printf(TEXT("%s_Test%02d"), *Test->GetTestName(), i))
                             : FName(*FString::Printf(
                                   TEXT("%s_%s"),
                                   *Test->GetTestName(),
                                   *FString::Printf(TEXT("%d"), TestHelpers::Port(i))));
        }
        InstanceDescriptor.Users = InstanceUserDescriptors;
        InstanceDescriptors.Add(InstanceDescriptor);
    }

    auto CredentialsPIE = MakeShared<FAutomationLoginCredentialsForPIE>();
    for (const auto &InstanceDescriptor : InstanceDescriptors)
    {
        CredentialsPIE->Credentials.Add(FOnlineAccountCredentials(
            TEXT("AUTOMATED_TESTING"),
            InstanceDescriptor.Users[0].CredentialId,
            InstanceDescriptor.Users[0].CredentialToken));
    }
    ((UOnlineEngineInterfaceEOS *)UOnlineEngineInterface::Get())->AutomationCredentials = CredentialsPIE;

    if (!Test->TestTrue(
            FString::Printf(TEXT("Expected to be able to create or find %s."), *FTestMapName::GetMapName(1)),
            this->CreateMapIfNeeded(FTestMapName::GetMapName(1))))
    {
        this->OnDone();
        return;
    }
    if (!Test->TestTrue(
            FString::Printf(TEXT("Expected to be able to create or find %s."), *FTestMapName::GetMapName(2)),
            this->CreateMapIfNeeded(FTestMapName::GetMapName(2))))
    {
        this->OnDone();
        return;
    }

    FEditorFileUtils::LoadMap(*FString::Printf(TEXT("/Game/Developers/%s"), *FTestMapName::GetMapName(1)), true, true);

    // Wait until the map loads.
    FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateSPLambda(
            this,
            [this, InstanceDescriptors, Context](float) {
                ULevelEditorPlaySettings *PlaySettings = NewObject<ULevelEditorPlaySettings>();
                PlaySettings->SetRunUnderOneProcess(true);
                PlaySettings->SetPlayNumberOfClients(InstanceDescriptors.Num());
                PlaySettings->SetPlayNetMode(this->PlayMode);
                PlaySettings->SetServerPort(static_cast<uint16>(TestHelpers::Port(7777)));
                PlaySettings->bLaunchSeparateServer = false;

                ((UEditorEngine *)GEngine)->UserEditedPlayWorldURL =
                    ((UEditorEngine *)GEngine)
                        ->BuildPlayWorldURL(
                            *FString::Printf(TEXT("/Game/Developers/%s"), *FTestMapName::GetMapName(1)),
                            false,
                            this->AdditionalLaunchParameters);

                FLevelEditorModule &LevelEditorModule =
                    FModuleManager::Get().GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
                FRequestPlaySessionParams Params;
                Params.DestinationSlateViewport = LevelEditorModule.GetFirstActiveViewport();
                Params.EditorPlaySettings = PlaySettings;
                Params.SessionDestination = EPlaySessionDestinationType::InProcess;
                Params.GameModeOverride = ARedpointEOSTestGameMode::StaticClass();
                GUnrealEd->RequestPlaySession(Params);

                // Now wait for the editor to start the play-in-editor session.
                this->TimeoutAt = FPlatformTime::Seconds() + 60;
                FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateSP(
                    this,
                    &FEditorChainedTaskExecutor::OnCheckIfReadyToTest,
                    Context,
                    InstanceDescriptors));

                return false;
            }),
        0.1f);
}

bool FEditorChainedTaskExecutor::HasStandalonePlayerControllers(
    TArray<FInstanceFactory::FInstanceDescriptor> InstanceDescriptors)
{
    if (FPlatformTime::Seconds() > this->TimeoutAt)
    {
        // We didn't connect in time. Fail the test.
        for (const FWorldContext &WorldContext : GEngine->GetWorldContexts())
        {
            if (WorldContext.WorldType == EWorldType::PIE && WorldContext.World() != nullptr)
            {
                UE_LOG(
                    LogRedpointEOSEditorTests,
                    Verbose,
                    TEXT("PIE world context %d had player controllers:"),
                    WorldContext.PIEInstance);
                for (TActorIterator<APlayerController> It(WorldContext.World()); It; ++It)
                {
                    UE_LOG(LogRedpointEOSEditorTests, Verbose, TEXT("- %s (%d)"), *It->GetName(), It->GetLocalRole());
                }
            }
        }

        Test->TestTrue(TEXT("Player controller was initialized within 60 seconds"), false);
        return true;
    }

    // Make sure all PIE instances all have player controllers.
    bool bHasAllPCs = true;
    for (int32 i = 0; i < InstanceDescriptors.Num(); i++)
    {
        bool bHasPC = false;
        for (const FWorldContext &WorldContext : GEngine->GetWorldContexts())
        {
            if (WorldContext.WorldType == EWorldType::PIE && WorldContext.PIEInstance == i &&
                WorldContext.World() != nullptr)
            {
                for (TActorIterator<APlayerController> It(WorldContext.World()); It; ++It)
                {
                    bHasPC = true;
                    break;
                }
            }
            if (bHasPC)
            {
                break;
            }
        }
        if (!bHasPC)
        {
            bHasAllPCs = false;
        }
    }
    if (bHasAllPCs)
    {
        return true;
    }

    return false;
}

bool FEditorChainedTaskExecutor::HasListenServerPlayerControllers()
{
    if (FPlatformTime::Seconds() > this->TimeoutAt)
    {
        // We didn't connect in time. Fail the test.
        for (const FWorldContext &WorldContext : GEngine->GetWorldContexts())
        {
            if (WorldContext.WorldType == EWorldType::PIE && WorldContext.World() != nullptr)
            {
                UE_LOG(
                    LogRedpointEOSEditorTests,
                    Verbose,
                    TEXT("Test is failing. PIE world context %d had player controllers:"),
                    WorldContext.PIEInstance);
                for (TActorIterator<APlayerController> It(WorldContext.World()); It; ++It)
                {
                    UE_LOG(LogRedpointEOSEditorTests, Verbose, TEXT("- %s (%d)"), *It->GetName(), It->GetLocalRole());
                }
            }
        }

        UE_LOG(
            LogRedpointEOSEditorTests,
            Verbose,
            TEXT("Expected: Network connection was successfully established within 60 seconds"));
        Test->TestTrue(TEXT("Network connection was successfully established within 60 seconds"), false);
        return true;
    }

    bool bFoundAuthorativePC0OnHost = false;
    bool bFoundAuthorativePC1OnHost = false;
    bool bFoundAutonomousProxyPC1OnClient = false;

    for (const FWorldContext &WorldContext : GEngine->GetWorldContexts())
    {
        if (WorldContext.WorldType == EWorldType::PIE && WorldContext.World() != nullptr)
        {
            for (TActorIterator<APlayerController> It(WorldContext.World()); It; ++It)
            {
                if (WorldContext.PIEInstance == 0 && It->GetName() == TEXT("PlayerController_0") &&
                    It->GetLocalRole() == ENetRole::ROLE_Authority)
                {
                    bFoundAuthorativePC0OnHost = true;
                }

                if (WorldContext.PIEInstance == 0 && It->GetName() == TEXT("PlayerController_1") &&
                    It->GetLocalRole() == ENetRole::ROLE_Authority)
                {
                    bFoundAuthorativePC1OnHost = true;
                }

                if (WorldContext.PIEInstance == 1 && It->GetName() == TEXT("PlayerController_1") &&
                    It->GetLocalRole() == ENetRole::ROLE_AutonomousProxy)
                {
                    bFoundAutonomousProxyPC1OnClient = true;
                }
            }
        }
    }

    return bFoundAuthorativePC0OnHost && bFoundAuthorativePC1OnHost && bFoundAutonomousProxyPC1OnClient;
}

bool FEditorChainedTaskExecutor::HasDedicatedServerPlayerControllers()
{
    if (FPlatformTime::Seconds() > this->TimeoutAt)
    {
        // We didn't connect in time. Fail the test.
        for (const FWorldContext &WorldContext : GEngine->GetWorldContexts())
        {
            if (WorldContext.WorldType == EWorldType::PIE && WorldContext.World() != nullptr)
            {
                UE_LOG(
                    LogRedpointEOSEditorTests,
                    Verbose,
                    TEXT("Test is failing. PIE world context %d had player controllers:"),
                    WorldContext.PIEInstance);
                for (TActorIterator<APlayerController> It(WorldContext.World()); It; ++It)
                {
                    UE_LOG(LogRedpointEOSEditorTests, Verbose, TEXT("- %s (%d)"), *It->GetName(), It->GetLocalRole());
                }
            }
        }

        UE_LOG(
            LogRedpointEOSEditorTests,
            Verbose,
            TEXT("Expected: Network connection was successfully established within 60 seconds"));
        Test->TestTrue(TEXT("Network connection was successfully established within 60 seconds"), false);
        return true;
    }

    bool bFoundAuthorativePC0OnHost = false;

    for (const FWorldContext &WorldContext : GEngine->GetWorldContexts())
    {
        if (WorldContext.WorldType == EWorldType::PIE && WorldContext.World() != nullptr)
        {
            for (TActorIterator<APlayerController> It(WorldContext.World()); It; ++It)
            {
                if (WorldContext.PIEInstance == 0 && It->GetName() == TEXT("PlayerController_0") &&
                    It->GetLocalRole() == ENetRole::ROLE_Authority)
                {
                    bFoundAuthorativePC0OnHost = true;
                }
            }
        }
    }

    return bFoundAuthorativePC0OnHost;
}

bool FEditorChainedTaskExecutor::OnCheckIfReadyToTest(
    float DeltaSeconds,
    TSharedRef<FChainedTaskContext> Context,
    TArray<FInstanceFactory::FInstanceDescriptor> InstanceDescriptors)
{
    // Are we ready?
    bool bReady = false;
    if (this->PlayMode == EPlayNetMode::PIE_Client)
    {
        bReady = this->HasDedicatedServerPlayerControllers();
    }
    else if (this->PlayMode == EPlayNetMode::PIE_ListenServer)
    {
        bReady = this->HasListenServerPlayerControllers();
    }
    else
    {
        bReady = this->HasStandalonePlayerControllers(InstanceDescriptors);
    }
    if (!bReady)
    {
        if (FPlatformTime::Seconds() > this->TimeoutAt)
        {
            Test->TestTrue(TEXT("Editor started test within timeout"), false);
            this->OnDone();
            return false;
        }
        else
        {
            return true;
        }
    }

    // For all instance descriptors, create local players for splitscreen if needed.
    for (int i = 0; i < InstanceDescriptors.Num(); i++)
    {
        const auto &WorldContext = GEngine->GetWorldContextFromPIEInstanceChecked(i);
        for (int u = 1; u < InstanceDescriptors[i].Users.Num(); u++)
        {
            FString PlayerError;
            auto *LocalPlayer = WorldContext.OwningGameInstance->CreateLocalPlayer(u, PlayerError, true);
            if (LocalPlayer == nullptr)
            {
                Test->TestTrue(FString::Printf(TEXT("Failed to create splitscreen player: %s"), *PlayerError), false);
            }
        }
    }

    // Sign into splitscreen player subsystems.
    FMultiOperation<FInstanceFactory::FInstanceDescriptor, TArray<FInstanceFactory::FInstance>>::Run(
        InstanceDescriptors,
        FMultiOperation<FInstanceFactory::FInstanceDescriptor, TArray<FInstanceFactory::FInstance>>::FInitiate::
            CreateLambda(
                [this](
                    const FInstanceFactory::FInstanceDescriptor &InstanceDescriptor,
                    const TDelegate<void(TArray<FInstanceFactory::FInstance> OutValue)> &OnInstanceDone) -> bool {
                    auto Subsystem =
                        ((FOnlineSubsystemEOS *)Online::GetSubsystem(
                             GEngine->GetWorldContextFromPIEInstance(InstanceDescriptor.InstanceId)->World(),
                             FName(TEXT("RedpointEOS"))))
                            ->AsShared();
                    FMultiOperation<FInstanceFactory::FInstanceUserDescriptor, FInstanceFactory::FInstance>::Run(
                        InstanceDescriptor.Users,
                        FMultiOperation<FInstanceFactory::FInstanceUserDescriptor, FInstanceFactory::FInstance>::
                            FInitiate::CreateLambda(
                                [this, InstanceDescriptor, Subsystem](
                                    const FInstanceFactory::FInstanceUserDescriptor &InstanceUserDescriptor,
                                    const TDelegate<void(FInstanceFactory::FInstance)> &OnUserDone) -> bool {
                                    auto Identity = Subsystem->GetIdentityInterface();
                                    check(Identity);

                                    FInstanceFactory::FInstance Instance;
                                    Instance.Subsystem = Subsystem;
                                    Instance.WorldContextHandle =
                                        GEngine->GetWorldContextFromPIEInstance(InstanceDescriptor.InstanceId)
                                            ->ContextHandle;
                                    if (InstanceUserDescriptor.LocalUserNum == 0)
                                    {
                                        Instance.UserId = StaticCastSharedPtr<const FUniqueNetIdEOS>(
                                            Identity->GetUniquePlayerId(InstanceUserDescriptor.LocalUserNum));
                                        OnUserDone.ExecuteIfBound(Instance);
                                        return true;
                                    }

                                    auto CancelLogin = RegisterOSSCallback(
                                        this->Test,
                                        Identity,
                                        InstanceUserDescriptor.LocalUserNum,
                                        &IOnlineIdentity::AddOnLoginCompleteDelegate_Handle,
                                        &IOnlineIdentity::ClearOnLoginCompleteDelegate_Handle,
                                        TFunction<void(int32, bool, const FUniqueNetId &, const FString &)>(
                                            [Instance, OnUserDone](
                                                int32 LocalUserNum,
                                                bool bWasSuccessful,
                                                const FUniqueNetId &UserId,
                                                const FString &Error) {
                                                if (!bWasSuccessful)
                                                {
                                                    OnUserDone.ExecuteIfBound(FInstanceFactory::FInstance());
                                                    return;
                                                }

                                                auto InstanceCopy = Instance;
                                                InstanceCopy.UserId =
                                                    StaticCastSharedRef<const FUniqueNetIdEOS>(UserId.AsShared());
                                                OnUserDone.ExecuteIfBound(InstanceCopy);
                                                return;
                                            }));
                                    FOnlineAccountCredentials Creds;
                                    Creds.Type = TEXT("AUTOMATED_TESTING");
                                    Creds.Id = InstanceUserDescriptor.CredentialId;
                                    Creds.Token = InstanceUserDescriptor.CredentialToken;
                                    if (!Identity->Login(InstanceUserDescriptor.LocalUserNum, Creds))
                                    {
                                        CancelLogin();
                                        OnUserDone.ExecuteIfBound(FInstanceFactory::FInstance());
                                    }
                                    return true;
                                }),
                        FMultiOperation<FInstanceFactory::FInstanceUserDescriptor, FInstanceFactory::FInstance>::
                            FOnDone::CreateLambda(
                                [OnInstanceDone](const TArray<FInstanceFactory::FInstance> &Instances) {
                                    OnInstanceDone.ExecuteIfBound(Instances);
                                }));
                    return true;
                }),
        FMultiOperation<FInstanceFactory::FInstanceDescriptor, TArray<FInstanceFactory::FInstance>>::FOnDone::
            CreateLambda([this, Context](const TArray<TArray<FInstanceFactory::FInstance>> &InstanceGroups) {
                TArray<FMultiplayerScenarioInstance> ScenarioInstances;
                for (const auto &InstanceGroup : InstanceGroups)
                {
                    for (const auto &Instance : InstanceGroup)
                    {
                        FMultiplayerScenarioInstance ScenarioInstance;
                        ScenarioInstance.Subsystem = Instance.Subsystem;
                        ScenarioInstance.UserId = Instance.UserId;
                        ScenarioInstance.WorldContextHandle = Instance.WorldContextHandle;
                        ScenarioInstances.Add(ScenarioInstance);
                    }
                }

                auto TotalSteps = this->Steps.Num();
                auto First = this->Steps[0];
                this->Steps.RemoveAt(0);
                Context->AttachInstances(ScenarioInstances);
                Context->Assert().AddInfo(FString::Printf(
                    TEXT("Executing chained task step %d of %d..."),
                    TotalSteps - this->Steps.Num(),
                    TotalSteps));
                First->ExecuteAsync(
                    Context,
                    FChainedTask::FOnComplete::CreateSP(
                        this,
                        &FEditorChainedTaskExecutor::OnStepComplete,
                        Context,
                        this->AsShared(),
                        First,
                        TotalSteps));
            }));
    return false;
}

}

REDPOINT_EOS_CODE_GUARD_END()