// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/TestUtilities/InstanceFactory.h"

#include "Dom/JsonObject.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "OnlineSubsystemRedpointEOSModule.h"
#include "RedpointEOSCore/Id/UniqueNetIdEOS.h"
#include "RedpointEOSCore/Utils/MultiOperation.h"
#include "RedpointEOSTests/TestUtilities/EOSConfigEASLogin.h"
#include "RedpointEOSTests/TestUtilities/Logging.h"
#include "RedpointEOSTests/TestUtilities/RegisterOSSCallback.h"
#include "RedpointEOSTests/TestUtilities/TestHelpers.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(366403318, Redpoint::EOS::Tests::TestUtilities)
{
using namespace ::Redpoint::EOS::Tests::TestUtilities;

bool FInstanceFactory::LoadEpicGamesAutomatedTestingCredentials(
    TArray<FEpicGamesAutomatedTestingCredential> &OutEpicGamesAutomatedTestingCredentials)
{
    TArray<FString> SearchPaths;
    FString EASAutomatedTestCredentialPathAuthGraph =
        FPlatformMisc::GetEnvironmentVariable(TEXT("EAS_AUTOMATED_TESTING_CREDENTIAL_PATH_AUTH_GRAPH"));
    if (!EASAutomatedTestCredentialPathAuthGraph.IsEmpty())
    {
        SearchPaths.Add(EASAutomatedTestCredentialPathAuthGraph);
    }
    FString EASAutomatedTestCredentialPath =
        FPlatformMisc::GetEnvironmentVariable(TEXT("EAS_AUTOMATED_TESTING_CREDENTIAL_PATH"));
    if (!EASAutomatedTestCredentialPath.IsEmpty())
    {
        SearchPaths.Add(EASAutomatedTestCredentialPath);
    }
    FString DeviceCredentialsPath =
        FModuleManager::GetModuleChecked<FOnlineSubsystemRedpointEOSModule>(FName("OnlineSubsystemRedpointEOS"))
            .GetPathToEASAutomatedTestingCredentials();
    if (!DeviceCredentialsPath.IsEmpty())
    {
        SearchPaths.Add(DeviceCredentialsPath);
    }
#if PLATFORM_WINDOWS
    FString UserProfile = FPlatformMisc::GetEnvironmentVariable(TEXT("USERPROFILE"));
    SearchPaths.Add(
        UserProfile / TEXT("Projects") / TEXT("eas-automated-testing-credentials") / TEXT("Credentials.json"));
#elif PLATFORM_MAC
    FString Home = FPlatformMisc::GetEnvironmentVariable(TEXT("HOME"));
    SearchPaths.Add(Home / TEXT("Projects") / TEXT("eas-automated-testing-credentials") / TEXT("Credentials.json"));
#endif
    FString SelectedPath = TEXT("");
    for (const auto &SearchPath : SearchPaths)
    {
        UE_LOG(LogRedpointEOSTests, Verbose, TEXT("Searching for credentials JSON: %s"), *SearchPath);
        if (FPaths::FileExists(SearchPath))
        {
            UE_LOG(LogRedpointEOSTests, Verbose, TEXT("Located credentials JSON at: %s"), *SearchPath);
            SelectedPath = SearchPath;
            break;
        }
    }
    if (SelectedPath.IsEmpty())
    {
        UE_LOG(LogRedpointEOSTests, Error, TEXT("Credentials JSON could not be found"));
        return false;
    }

    FString JsonString;
    if (!FFileHelper::LoadFileToString(JsonString, *SelectedPath))
    {
        UE_LOG(LogRedpointEOSTests, Error, TEXT("Credentials JSON could not be loaded"));
        return false;
    }

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
    if (!FJsonSerializer::Deserialize(Reader, JsonObject))
    {
        UE_LOG(LogRedpointEOSTests, Error, TEXT("Credentials JSON could not be parsed"));
        return false;
    }

    const TArray<TSharedPtr<FJsonValue>> *PlatformCredentials;
    if (JsonObject->TryGetArrayField(TEXT("Credentials"), PlatformCredentials))
    {
        for (const auto &PlatformCredential : *PlatformCredentials)
        {
            const TSharedPtr<FJsonObject> *Credential = nullptr;
            if (PlatformCredential->TryGetObject(Credential))
            {
                FEpicGamesAutomatedTestingCredential CredentialLoaded;
                CredentialLoaded.Username = (*Credential)->GetStringField(TEXT("Username"));
                CredentialLoaded.Password = (*Credential)->GetStringField(TEXT("Password"));
                OutEpicGamesAutomatedTestingCredentials.Add(CredentialLoaded);
            }
        }
        return true;
    }
    else
    {
        UE_LOG(LogRedpointEOSTests, Error, TEXT("Credentials JSON does not provide credentials"));
        return false;
    }
}

void FInstanceFactory::CreateInstances(
    FAsyncHotReloadableAutomationTestBase *Test,
    const TArray<int32> &InstanceLayout,
    const FOnDone &OnDoneFinal,
    EFlag Flags,
    const FOnInstancesCreated &Callback)
{
    using namespace ::Redpoint::EOS::Core::Utils;

    bool bIsDedicatedServer = (Flags & EFlag::DedicatedServer) != EFlag::None;
    bool bIsEpicGames = (Flags & EFlag::EpicGames) != EFlag::None;

    TArray<FEpicGamesAutomatedTestingCredential> EpicGamesCredentials;
    if (bIsEpicGames)
    {
        if (!LoadEpicGamesAutomatedTestingCredentials(EpicGamesCredentials))
        {
            Test->AddWarning(TEXT("This test is being skipped, because EAS testing credentials are not available."));
            OnDoneFinal();
            return;
        }
    }

    int32 GlobalUserCount = 0;
    TArray<FInstanceDescriptor> InstanceDescriptors;
    for (int i = 0; i < InstanceLayout.Num(); i++)
    {
        TArray<FInstanceUserDescriptor> InstanceUserDescriptors;
        for (int u = 0; u < InstanceLayout[i]; u++)
        {
            FInstanceUserDescriptor InstanceUserDescriptor;
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

        FInstanceDescriptor InstanceDescriptor;
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

    FMultiOperation<FInstanceDescriptor, TArray<FInstance>>::Run(
        InstanceDescriptors,
        FMultiOperation<FInstanceDescriptor, TArray<FInstance>>::FInitiate::CreateLambda(
            [Test, EpicGamesCredentials](
                const FInstanceDescriptor &InstanceDescriptor,
                const TDelegate<void(TArray<FInstance> OutValue)> &OnDone) -> bool {
                FOnlineSubsystemRedpointEOSModule &OSSModule =
                    FModuleManager::GetModuleChecked<FOnlineSubsystemRedpointEOSModule>(
                        FName(TEXT("OnlineSubsystemRedpointEOS")));

                auto Subsystem =
                    OSSModule.CreateSubsystem(InstanceDescriptor.SubsystemName, MakeShared<FEOSConfigEASLogin>());
                check(Subsystem);

                Test->RegisteredSubsystems.Add(Subsystem);

                auto Identity = Subsystem->GetIdentityInterface();
                check(Identity);

                FMultiOperation<FInstanceUserDescriptor, FInstance>::Run(
                    InstanceDescriptor.Users,
                    FMultiOperation<FInstanceUserDescriptor, FInstance>::FInitiate::CreateLambda(
                        [Test, Subsystem, Identity](
                            const FInstanceUserDescriptor &InstanceUserDescriptor,
                            const TDelegate<void(FInstance)> &OnUserDone) -> bool {
                            auto CancelLogin = RegisterOSSCallback(
                                Test,
                                Identity,
                                InstanceUserDescriptor.LocalUserNum,
                                &IOnlineIdentity::AddOnLoginCompleteDelegate_Handle,
                                &IOnlineIdentity::ClearOnLoginCompleteDelegate_Handle,
                                TFunction<void(int32, bool, const FUniqueNetId &, const FString &)>(
                                    [Subsystem, Identity, OnUserDone](
                                        int32 LocalUserNum,
                                        bool bWasSuccessful,
                                        const FUniqueNetId &UserId,
                                        const FString &Error) {
                                        if (!bWasSuccessful)
                                        {
                                            OnUserDone.ExecuteIfBound(FInstance());
                                            return;
                                        }

                                        auto EOSUser = StaticCastSharedRef<const FUniqueNetIdEOS>(UserId.AsShared());

                                        FInstance Inst;
                                        Inst.Subsystem = Subsystem;
                                        Inst.UserId = EOSUser;
                                        Inst.WorldContextHandle = NAME_None;
                                        OnUserDone.ExecuteIfBound(Inst);
                                    }));
                            FOnlineAccountCredentials Creds;
                            Creds.Type = TEXT("AUTOMATED_TESTING");
                            Creds.Id = InstanceUserDescriptor.CredentialId;
                            Creds.Token = InstanceUserDescriptor.CredentialToken;
                            if (!Identity->Login(InstanceUserDescriptor.LocalUserNum, Creds))
                            {
                                CancelLogin();
                                OnUserDone.ExecuteIfBound(FInstance());
                            }
                            return true;
                        }),
                    FMultiOperation<FInstanceUserDescriptor, FInstance>::FOnDone::CreateLambda(
                        [OnDone](const TArray<FInstance> &Instances) {
                            OnDone.ExecuteIfBound(Instances);
                        }));
                return true;
            }),
        FMultiOperation<FString, TArray<FInstance>>::FOnDone::CreateLambda(
            [Test, Callback, OnDoneFinal](const TArray<TArray<FInstance>> &InstanceGroups) {
                TArray<FInstance> Instances;
                for (const auto &InstanceGroup : InstanceGroups)
                {
                    for (const auto &Instance : InstanceGroup)
                    {
                        Test->TestTrue("Subsystem is valid", Instance.Subsystem.IsValid());
                        if (!Instance.Subsystem.IsValid())
                        {
                            OnDoneFinal();
                            return;
                        }
                        Instances.Add(Instance);
                    }
                }
                Callback(Instances, OnDoneFinal);
            }));
}

void FInstanceFactory::CreateInstances(
    FAsyncHotReloadableAutomationTestBase *Test,
    const TArray<int32> &InstanceLayout,
    const FOnDone &OnDoneFinal,
    EFlag Flags,
    const FOnSingleInstanceCreated &Callback)
{
    CreateInstances(
        Test,
        InstanceLayout,
        OnDoneFinal,
        Flags,
        [Callback](const TArray<FInstance> &Instances, const FOnDone &OnDone) {
            Callback(Instances[0].Subsystem.Pin(), Instances[0].UserId, OnDone);
        });
}

}

REDPOINT_EOS_CODE_GUARD_END()