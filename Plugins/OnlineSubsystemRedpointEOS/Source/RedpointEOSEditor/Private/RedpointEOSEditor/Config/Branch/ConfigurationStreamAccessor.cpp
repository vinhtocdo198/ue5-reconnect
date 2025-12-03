// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditor/Config/Branch/ConfigurationStreamAccessor.h"

#if REDPOINT_EOS_UE_5_5_OR_LATER
#include "Misc/AutomationTest.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/ConfigContext.h"
#include "Misc/FileHelper.h"
#include "RedpointEOSEditor/Config/ConfigurationFilePathHelpers.h"
#include "RedpointEOSEditorModule.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if REDPOINT_EOS_UE_5_5_OR_LATER

namespace REDPOINT_EOS_FILE_NS_ID(564516167, Redpoint::EOS::Editor::Config::Branch)
{

FConfigurationStreamAccessor::FInstantiatedConfigFile::FInstantiatedConfigFile(const FString &InFilePath)
    : FilePath(InFilePath)
    , ConfigSystem(MakeShared<FConfigCacheIni>(EConfigCacheType::Temporary))
    , File()
    , Context(FConfigContext::ReadSingleIntoLocalFile(File, TEXT("")))
    , Branch()
{
    Context.ConfigSystem = &ConfigSystem.Get();
    Context.bWriteDestIni = true;
    Context.BaseIniName = InFilePath;
    FString Unused;
    Branch = Context.ConfigSystem->FindBranch(*InFilePath, Unused);
    if (Branch == nullptr)
    {
        Branch = &Context.ConfigSystem->AddNewBranch(InFilePath);
        Branch->Hierarchy.Add(0, InFilePath);
        Branch->bIsSafeUnloaded = true;
    }
    else if (FindStaticLayer(Branch, InFilePath) == nullptr)
    {
        // Branch exists but doesn't have the layer. This only seems to affect TrustedClient.ini in the
        // automation tests, and for instantiated files we know there's only a single layer so we don't
        // need to worry about position in the hierarchy.
        Branch->Hierarchy.Add(0, InFilePath);
        Branch->bIsSafeUnloaded = true;
    }
    Context.Branch = Branch;
    Context.bForceReload = true;
    Context.Load(*InFilePath);
}

void FConfigurationStreamAccessor::FInstantiatedConfigFile::ReloadIfNotLoaded()
{
    if (FindStaticLayer(Branch, Context.BaseIniName) == nullptr || Branch->StaticLayers.Num() == 0)
    {
        // Branch exists but doesn't have the layer. This only seems to affect TrustedClient.ini in the
        // automation tests, and for instantiated files we know there's only a single layer so we don't
        // need to worry about position in the hierarchy.
        if (Branch->Hierarchy.Num() == 0)
        {
            Branch->Hierarchy.Add(0, Context.BaseIniName);
        }
        Branch->bIsSafeUnloaded = true;
        Context.bForceReload = true;
        Context.Load(*Context.BaseIniName);
    }
}

TMap<TTuple<EConfigurationFileType, FName>, TSharedPtr<FConfigurationStreamAccessor::FInstantiatedConfigFile>>
    FConfigurationStreamAccessor::InstantiatedConfigFiles;

TSharedRef<FConfigurationStreamAccessor::FInstantiatedConfigFile> FConfigurationStreamAccessor::GetInstantiatedFile(
    EConfigurationFileType File,
    FName Platform)
{
    TSharedPtr<FInstantiatedConfigFile> &Ptr = FConfigurationStreamAccessor::InstantiatedConfigFiles.FindOrAdd(
        TTuple<EConfigurationFileType, FName>(File, Platform));
    if (!Ptr.IsValid())
    {
        Ptr = MakeShared<FInstantiatedConfigFile>(GetFilePath(File, Platform));
    }
    Ptr->ReloadIfNotLoaded();
    return Ptr.ToSharedRef();
}

FConfigBranch *FConfigurationStreamAccessor::GetConfigBranchUnchecked(
    EConfigurationFileType File,
    FName Platform,
    FString &FilePath)
{
    FilePath = GetFilePath(File, Platform);
    FConfigCacheIni *Cache = File == EConfigurationFileType::DedicatedServer
                                 ? GConfig->ForPlatform(FName(TEXT("DedicatedServer")))
                                 : (Platform.IsNone() ? GConfig : GConfig->ForPlatform(Platform));
    FConfigBranch *Branch = nullptr;
    switch (File)
    {
    case EConfigurationFileType::Engine:
        Branch = Cache->FindBranch(FName("Engine"), GEngineIni);
        break;
    case EConfigurationFileType::DedicatedServer: {
        auto InstantiatedConfigFile = GetInstantiatedFile(File, Platform);
        Branch = InstantiatedConfigFile->Branch;
        break;
    }
    case EConfigurationFileType::Game:
        Branch = Cache->FindBranch(FName("Game"), GGameIni);
        break;
    case EConfigurationFileType::Editor:
        Branch = Cache->FindBranch(FName("Editor"), GEditorIni);
        break;
    case EConfigurationFileType::TrustedClient: {
        auto InstantiatedConfigFile = GetInstantiatedFile(File, Platform);
        Branch = InstantiatedConfigFile->Branch;
        break;
    }
    default:
        checkf(false, TEXT("Unsupported EConfigurationFileType"));
        break;
    }
    TArray<TPair<FString, FConfigCommandStream>> StaticLayers;
    if (Branch != nullptr)
    {
        for (const auto &StaticLayer : Branch->StaticLayers)
        {
            StaticLayers.Add(StaticLayer);
        }
    }
    return Branch;
}

const FConfigCommandStream *FConfigurationStreamAccessor::FindStaticLayer(
    FConfigBranch *Branch,
    const FString &FilePath)
{
    auto *ExactMatch = Branch->StaticLayers.Find(FilePath);
    if (ExactMatch != nullptr)
    {
        return ExactMatch;
    }

    for (const auto &KV : Branch->StaticLayers)
    {
        if (FPaths::IsSamePath(FilePath, KV.Key))
        {
            return &KV.Value;
        }
    }

    return nullptr;
}

static void ReadTest(FAutomationTestBase *Test, EConfigurationFileType FileType, FName Platform)
{
    FString FilePath;
    auto *Branch = FConfigurationStreamAccessor::GetConfigBranchUnchecked(FileType, Platform, FilePath);
    if (Test->TestTrue(TEXT("Expected branch to not be null!"), Branch != nullptr))
    {
        bool bSuccess;
        auto *Stream = FConfigurationStreamAccessor::FindStaticLayer(Branch, FilePath);
        if (FPaths::FileExists(FilePath))
        {
            bSuccess = Test->TestTrue(TEXT("Expected target stream to exist in branch!"), Stream != nullptr);
        }
        else
        {
            bSuccess = Test->TestTrue(
                TEXT("Expected target stream to not exist in branch (because the file does not exist on disk)!"),
                Stream == nullptr);
        }
        if (!bSuccess)
        {
            FString BranchDump;

            BranchDump += FString::Printf(TEXT("we were looking for '%s':\n"), *FilePath);
            BranchDump += FString::Printf(
                TEXT("name: %s, platform: %s\n"),
                *Branch->IniName.ToString(),
                *Branch->Platform.ToString());
            BranchDump += FString::Printf(TEXT("%d static layers:\n"), Branch->StaticLayers.Num());
            for (const auto &StaticLayer : Branch->StaticLayers)
            {
                BranchDump += FString::Printf(TEXT("- %s (%d sections)\n"), *StaticLayer.Key, StaticLayer.Value.Num());
            }
            BranchDump += FString::Printf(TEXT("%d dynamic layers:\n"), Branch->DynamicLayers.Num());
            for (const auto &DynamicLayer : Branch->DynamicLayers)
            {
                BranchDump += FString::Printf(
                    TEXT("- %s (%d sections)"),
                    DynamicLayer != nullptr ? *DynamicLayer->Filename : TEXT("(unknown)"),
                    DynamicLayer != nullptr ? DynamicLayer->Num() : 0);
            }
            BranchDump += FString::Printf(TEXT("Saved layer (%d sections)\n"), Branch->SavedLayer.Num());
            BranchDump +=
                FString::Printf(TEXT("Combined static layers (%d sections)\n"), Branch->CombinedStaticLayers.Num());
            BranchDump +=
                FString::Printf(TEXT("Final combined layers (%d sections)\n"), Branch->FinalCombinedLayers.Num());
            BranchDump +=
                FString::Printf(TEXT("Command line overrides (%d sections)\n"), Branch->CommandLineOverrides.Num());
            BranchDump += FString::Printf(TEXT("In-memory file (%d sections)\n"), Branch->InMemoryFile.Num());
            BranchDump += FString::Printf(TEXT("Runtime changes (%d sections)\n"), Branch->RuntimeChanges.Num());
            UE_LOG(LogRedpointEOSEditor, Warning, TEXT("%s"), *BranchDump);
            Test->AddWarning(BranchDump);
        }
    }
}

#define IMPLEMENT_REDPOINT_CONFIGURATION_TEST_CURRENT(ConfigType)                                                      \
    IMPLEMENT_SIMPLE_AUTOMATION_TEST(                                                                                  \
        FOnlineSubsystemEOS_Editor_Config_##ConfigType##_Current_Read,                                                 \
        "Redpoint.EOS.Editor.Config." #ConfigType ".Current.Read",                                               \
        EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext |                                    \
            EAutomationTestFlags::ServerContext | EAutomationTestFlags::ProductFilter);                                \
    bool FOnlineSubsystemEOS_Editor_Config_##ConfigType##_Current_Read::RunTest(const FString &Parameters)             \
    {                                                                                                                  \
        ReadTest(this, EConfigurationFileType::ConfigType, NAME_None);                                                 \
        return true;                                                                                                   \
    }
#define IMPLEMENT_REDPOINT_CONFIGURATION_TEST_PLATFORM(ConfigType)                                                     \
    IMPLEMENT_SIMPLE_AUTOMATION_TEST(                                                                                  \
        FOnlineSubsystemEOS_Editor_Config_##ConfigType##_Platform_Read,                                                \
        "Redpoint.EOS.Editor.Config." #ConfigType ".Platform.Read",                                              \
        EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext |                                    \
            EAutomationTestFlags::ServerContext | EAutomationTestFlags::ProductFilter);                                \
    bool FOnlineSubsystemEOS_Editor_Config_##ConfigType##_Platform_Read::RunTest(const FString &Parameters)            \
    {                                                                                                                  \
        ReadTest(this, EConfigurationFileType::ConfigType, FName(TEXT("Android")));                                    \
        return true;                                                                                                   \
    }

IMPLEMENT_REDPOINT_CONFIGURATION_TEST_CURRENT(Engine)
IMPLEMENT_REDPOINT_CONFIGURATION_TEST_PLATFORM(Engine)
IMPLEMENT_REDPOINT_CONFIGURATION_TEST_CURRENT(Game)
IMPLEMENT_REDPOINT_CONFIGURATION_TEST_PLATFORM(Game)
IMPLEMENT_REDPOINT_CONFIGURATION_TEST_CURRENT(Editor)
IMPLEMENT_REDPOINT_CONFIGURATION_TEST_CURRENT(DedicatedServer)
IMPLEMENT_REDPOINT_CONFIGURATION_TEST_CURRENT(TrustedClient)

#undef IMPLEMENT_REDPOINT_CONFIGURATION_TEST_CURRENT
#undef IMPLEMENT_REDPOINT_CONFIGURATION_TEST_PLATFORM

}

#endif

REDPOINT_EOS_CODE_GUARD_END()
