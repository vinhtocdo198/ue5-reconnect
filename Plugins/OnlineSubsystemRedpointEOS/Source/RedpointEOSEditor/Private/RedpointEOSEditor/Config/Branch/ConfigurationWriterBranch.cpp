// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditor/Config/Branch/ConfigurationWriterBranch.h"

#if REDPOINT_EOS_UE_5_5_OR_LATER
#include "HAL/PlatformFileManager.h"
#include "ISourceControlModule.h"
#include "Misc/App.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/ConfigContext.h"
#include "Misc/FileHelper.h"
#include "RedpointEOSCore/Editor/EditorSignalling.h"
#include "RedpointEOSEditor/Config/Branch/ApplyPendingChanges.h"
#include "RedpointEOSEditor/Config/Branch/ConfigurationStreamAccessor.h"
#include "RedpointEOSEditor/Config/Branch/IndexConfigLines.h"
#include "RedpointEOSEditor/Config/Branch/ParseBufferToConfigLines.h"
#include "RedpointEOSEditor/Config/ConfigurationEnumLookup.h"
#include "RedpointEOSEditor/Config/ConfigurationFilePathHelpers.h"
#include "RedpointEOSEditorModule.h"
#include "SourceControlHelpers.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if REDPOINT_EOS_UE_5_5_OR_LATER

namespace REDPOINT_EOS_FILE_NS_ID(2436931538, Redpoint::EOS::Editor::Config::Branch)
{
using namespace ::Redpoint::EOS::Core::Editor;
using namespace ::Redpoint::EOS::Editor::Config;
using namespace ::Redpoint::EOS::Editor::Config::Branch;

FConfigurationWriterBranch::FConfigurationWriterBranch()
    : PendingChanges()
{
}

FConfigurationWriterPendingChanges &FConfigurationWriterBranch::GetConfigFile(
    EConfigurationFileType File,
    FName Platform)
{
    auto FilePath = GetFilePath(File, Platform);
    auto *Existing = this->PendingChanges.Find(FilePath);
    if (Existing != nullptr)
    {
        return *Existing;
    }
    return this->PendingChanges.Emplace(
        FilePath,
        TTuple<FString, EConfigurationFileType, FName>(FilePath, File, Platform));
}

bool FConfigurationWriterBranch::SetEnumInternal(
    const FString &Key,
    const FString &EnumClass,
    uint8 Value,
    const FString &Section,
    EConfigurationFileType File,
    FName Platform)
{
    const UEnum *Enum = LookupEnum(EnumClass);
    if (Enum == nullptr)
    {
        UE_LOG(
            LogRedpointEOSEditor,
            Error,
            TEXT("Unable to retrieve UObject metadata for enum %s, so the call to SetEnumInternal will not have any "
                 "effect."),
            *EnumClass);
        return false;
    }
    FString EnumValue = Enum->GetNameStringByValue((int64)Value);

    auto &Changes = this->GetConfigFile(File, Platform);
    Changes.Upserts.Add(FConfigurationSectionKey(Section, Key), EnumValue);
    return true;
}

bool FConfigurationWriterBranch::SetBool(
    const FString &Key,
    bool Value,
    const FString &Section,
    EConfigurationFileType File,
    FName Platform)
{
    auto &Changes = this->GetConfigFile(File, Platform);
    Changes.Upserts.Add(FConfigurationSectionKey(Section, Key), Value);
    return true;
}

bool FConfigurationWriterBranch::SetInt32(
    const FString &Key,
    int32 Value,
    const FString &Section,
    EConfigurationFileType File,
    FName Platform)
{
    auto &Changes = this->GetConfigFile(File, Platform);
    Changes.Upserts.Add(FConfigurationSectionKey(Section, Key), Value);
    return true;
}

bool FConfigurationWriterBranch::SetString(
    const FString &Key,
    const FString &Value,
    const FString &Section,
    EConfigurationFileType File,
    FName Platform)
{
    auto &Changes = this->GetConfigFile(File, Platform);
    Changes.Upserts.Add(FConfigurationSectionKey(Section, Key), Value);
    return true;
}

bool FConfigurationWriterBranch::ReplaceArray(
    const FString &Key,
    const TArray<FString> &Value,
    const FString &Section,
    EConfigurationFileType File,
    FName Platform)
{
    auto &Changes = this->GetConfigFile(File, Platform);
    Changes.Replacements.Add(FConfigurationSectionKey(Section, Key), Value);
    return true;
}

bool FConfigurationWriterBranch::Remove(
    const FString &Key,
    const FString &Section,
    EConfigurationFileType File,
    FName Platform)
{
    auto &Changes = this->GetConfigFile(File, Platform);
    Changes.Removals.Add(FConfigurationSectionKey(Section, Key));
    return true;
}

EConfigurationWriterFlushResult FConfigurationWriterBranch::FlushChanges()
{
    // Attempt to check out all of the relevant files.
    TSet<FString> FilePaths;
    if (USourceControlHelpers::IsEnabled())
    {
        for (const auto &FM : this->PendingChanges)
        {
            FilePaths.Add(FM.Value.Path);
        }
        TSet<FString> FilePathsForCheckout;
        ISourceControlProvider &SourceControlProvider = ISourceControlModule::Get().GetProvider();
        for (const auto &FilePath : FilePaths.Array())
        {
            auto State = SourceControlProvider.GetState(FilePath, EStateCacheUsage::Use);
            if (!State->IsUnknown() && !State->CanAdd() && State->IsSourceControlled() && State->CanCheckout() &&
                !State->IsCheckedOut())
            {
                FilePathsForCheckout.Add(FilePath);
            }
        }
        if (!USourceControlHelpers::CheckOutFiles(FilePathsForCheckout.Array(), true))
        {
            UE_LOG(
                LogRedpointEOSEditor,
                Error,
                TEXT("Could not check out one or more configuration files from source control. Your configuration "
                     "changes have not been saved."));
            FEditorSignalling::OnEditorCustomSignal().Broadcast(
                TEXT("Configuration"),
                TEXT("SourceControlPreventingConfigSave"));
            return EConfigurationWriterFlushResult::FailedToCheckOutFiles;
        }
    }

    // Apply all changes to files.
    TArray<FString> SavedFiles;
    for (const auto &FM : this->PendingChanges)
    {
        // Read file.
        FString Contents;
        if (!FFileHelper::LoadFileToString(Contents, *FM.Value.Path) && FPaths::FileExists(*FM.Value.Path))
        {
            UE_LOG(
                LogRedpointEOSEditor,
                Error,
                TEXT("Unable to read the configuration file '%s' while saving changes. The changes to this file will "
                     "be discarded."),
                *FM.Value.Path);
            continue;
        }

        // Generate the config index so we can apply differential changes to it.
        auto ConfigIndex = IndexConfigLines(ParseBufferToConfigLines(Contents));

        // Apply changes and get new content.
        auto NewContents = ApplyPendingChanges(ConfigIndex, FM.Value);

        // Write file.
        if (NewContents.TrimStartAndEnd() != Contents.TrimStartAndEnd())
        {
            if (!FFileHelper::SaveStringToFile(NewContents.TrimStartAndEnd(), *FM.Value.Path))
            {
                UE_LOG(
                    LogRedpointEOSEditor,
                    Error,
                    TEXT("Unable to save the configuration file '%s' after making changes. The changes to this file "
                         "will be discarded."),
                    *FM.Value.Path);
            }
            else
            {
                // Mark this file as "stage changes for source control".
                SavedFiles.Add(*FM.Value.Path);

                // If the save was successful, also reload the in-memory version.
                FString IgnoredFilePath;
                auto *Branch = FConfigurationStreamAccessor::GetConfigBranchUnchecked(
                    FM.Value.File,
                    FM.Value.Platform,
                    IgnoredFilePath);
                if (Branch != nullptr)
                {
                    // Setting this flag causes automation tests to fail, but it's definitely required in any real
                    // editor scenario. Without this flag turned on, the changes we made on disk don't actually reload
                    // into memory.
                    if (!FApp::IsUnattended())
                    {
                        Branch->bIsSafeUnloaded = true;
                    }

                    Branch->SafeReload();
                }
            }
        }
    }

    // Mark all the config files we modified for add.
    if (USourceControlHelpers::IsEnabled())
    {
        ISourceControlProvider &SourceControlProvider = ISourceControlModule::Get().GetProvider();
        TArray<FString> SavedFilesToAdd;
        for (const auto &FilePath : SavedFiles)
        {
            auto State = SourceControlProvider.GetState(FilePath, EStateCacheUsage::Use);
            if (!State->IsSourceControlled() && !State->IsUnknown() &&
                FPlatformFileManager::Get().GetPlatformFile().FileExists(*FilePath))
            {
                SavedFilesToAdd.Add(FilePath);
            }
        }
        if (SavedFilesToAdd.Num() > 0)
        {
            USourceControlHelpers::MarkFilesForAdd(SavedFilesToAdd, true);
        }
    }
    return EConfigurationWriterFlushResult::Success;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()
