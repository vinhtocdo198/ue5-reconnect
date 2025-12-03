// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditor/Config/Legacy/ConfigurationWriterLegacy.h"

#if !REDPOINT_EOS_UE_5_5_OR_LATER
#include "Misc/AssertionMacros.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/FileHelper.h"
#include "OnlineSubsystemRedpointEOS/Public/OnlineSubsystemRedpointEOSModule.h"
#include "RedpointEOSConfig/EngineConfigHelpers.h"
#include "RedpointEOSCore/Editor/EditorSignalling.h"
#include "RedpointEOSEditor/Config/ConfigurationEnumLookup.h"
#include "RedpointEOSEditor/Config/ConfigurationFilePathHelpers.h"
#include "RedpointEOSEditorModule.h"
#include "SourceControlHelpers.h"
#include "UObject/Class.h"
#include "UObject/ObjectMacros.h"
#include "UObject/UObjectGlobals.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_UE_5_5_OR_LATER

namespace REDPOINT_EOS_FILE_NS_ID(353527102, Redpoint::EOS::Editor::Config::Legacy)
{
using namespace ::Redpoint::EOS::Editor::Config;

FConfigurationWriterLegacy::FConfigurationWriterLegacy()
    : QueuedDelegates()
    , FilesQueuedForModify()
    , bRequireFullReload(false)
    , EOSLoadedFiles()
{
}

static void SetArray(FConfigFile *F, const TCHAR *Section, const TCHAR *Key, const TArray<FString> &Value)
{
    F->SetArray(Section, Key, Value);
}

FConfigurationLoadedFileLegacy FConfigurationWriterLegacy::GetConfigFile(const FString &Path)
{
    FConfigFile *File = Redpoint::EOS::Config::FEngineConfigHelpers::FindByName(Path);
    if (File == nullptr)
    {
        File = &GConfig->Add(Path, FConfigFile());
        File->Read(Path);
    }

    FConfigFile *SavedFile = nullptr;
    for (const auto &KnownFilename : GConfig->GetFilenames())
    {
        FConfigFile *KnownFile = Redpoint::EOS::Config::FEngineConfigHelpers::FindByName(*KnownFilename);
        if (KnownFile != nullptr)
        {
            for (const auto &KV : KnownFile->SourceIniHierarchy)
            {
#if REDPOINT_EOS_UE_5_1_OR_LATER
                if (KV.Value == Path)
#else
                if (KV.Value.Filename == Path)
#endif
                {
                    checkf(
                        SavedFile == nullptr,
                        TEXT("Configuration writing code needs to be adapted to support multiple impacted upstream "
                             "config files"));
                    SavedFile = KnownFile;
                    break;
                }
            }
        }
    }
    if (!this->EOSLoadedFiles.Contains(Path))
    {
        this->EOSLoadedFiles.Add(Path, MakeShared<FConfigurationWriterPendingChanges>(Path));
    }

    return FConfigurationLoadedFileLegacy(File, SavedFile, this->EOSLoadedFiles[Path].ToSharedRef());
}

bool FConfigurationWriterLegacy::SetEnumInternal(
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

    FConfigurationLoadedFileLegacy LCF = GetConfigFile(GetFilePath(File, Platform));
    if (!LCF.F)
    {
        return false;
    }
    FString ExistingValue;
    if (LCF.F->GetString(*Section, *Key, ExistingValue))
    {
        if (ExistingValue == EnumValue)
        {
            // This already matches the intended value, don't do anything.
            return false;
        }
    }

    this->FilesQueuedForModify.Add(FConfigurationWriterLegacyQueuedFileToModify(File, Platform));
    this->QueuedDelegates.Add(
        FConfigurationWriterLegacyQueuedDelegate::CreateLambda([this, File, Platform, Section, Key, EnumValue]() {
            FConfigurationLoadedFileLegacy LocalLCF = GetConfigFile(GetFilePath(File, Platform));
            if (!LocalLCF.F)
            {
                UE_LOG(
                    LogRedpointEOSEditor,
                    Error,
                    TEXT("GetConfigFile failed to load file during delegate callback."));
                return;
            }
            LocalLCF.F->SetString(*Section, *Key, *EnumValue);
            LocalLCF.F->Dirty = true;
            if (LocalLCF.SF)
            {
                LocalLCF.SF->SetString(*Section, *Key, *EnumValue);
            }
            LocalLCF.Info->Upserts.Add(FConfigurationSectionKey(Section, Key), FConfigurationSetting(EnumValue));
            LocalLCF.Info->bIsModified = true;
        }));
    return true;
}

bool FConfigurationWriterLegacy::SetBool(
    const FString &Key,
    bool Value,
    const FString &Section,
    EConfigurationFileType File,
    FName Platform)
{
    FConfigurationLoadedFileLegacy LCF = GetConfigFile(GetFilePath(File, Platform));
    if (!LCF.F)
    {
        return false;
    }
    bool bExistingValue;
    if (LCF.F->GetBool(*Section, *Key, bExistingValue))
    {
        if (bExistingValue == Value)
        {
            // This already matches the intended value, don't
            // do anything.
            return false;
        }
    }

    this->FilesQueuedForModify.Add(FConfigurationWriterLegacyQueuedFileToModify(File, Platform));
    this->QueuedDelegates.Add(
        FConfigurationWriterLegacyQueuedDelegate::CreateLambda([this, File, Platform, Section, Key, Value]() {
            FConfigurationLoadedFileLegacy LocalLCF = GetConfigFile(GetFilePath(File, Platform));
            if (!LocalLCF.F)
            {
                UE_LOG(
                    LogRedpointEOSEditor,
                    Error,
                    TEXT("GetConfigFile failed to load file during delegate callback."));
                return;
            }
            LocalLCF.F->SetString(*Section, *Key, Value ? TEXT("True") : TEXT("False"));
            LocalLCF.F->Dirty = true;
            if (LocalLCF.SF)
            {
                LocalLCF.SF->SetString(*Section, *Key, Value ? TEXT("True") : TEXT("False"));
            }
            LocalLCF.Info->Upserts.Add(FConfigurationSectionKey(Section, Key), FConfigurationSetting(Value));
            LocalLCF.Info->bIsModified = true;
        }));
    return true;
}

bool FConfigurationWriterLegacy::SetInt32(
    const FString &Key,
    int32 Value,
    const FString &Section,
    EConfigurationFileType File,
    FName Platform)
{
    FConfigurationLoadedFileLegacy LCF = GetConfigFile(GetFilePath(File, Platform));
    if (!LCF.F)
    {
        return false;
    }
    int32 ExistingValue;
    if (LCF.F->GetInt(*Section, *Key, ExistingValue))
    {
        if (ExistingValue == Value)
        {
            // This already matches the intended value, don't
            // do anything.
            return false;
        }
    }

    this->FilesQueuedForModify.Add(FConfigurationWriterLegacyQueuedFileToModify(File, Platform));
    this->QueuedDelegates.Add(
        FConfigurationWriterLegacyQueuedDelegate::CreateLambda([this, File, Platform, Section, Key, Value]() {
            FConfigurationLoadedFileLegacy LocalLCF = GetConfigFile(GetFilePath(File, Platform));
            if (!LocalLCF.F)
            {
                UE_LOG(
                    LogRedpointEOSEditor,
                    Error,
                    TEXT("GetConfigFile failed to load file during delegate callback."));
                return;
            }
            LocalLCF.F->SetInt64(*Section, *Key, Value);
            LocalLCF.F->Dirty = true;
            if (LocalLCF.SF)
            {
                LocalLCF.SF->SetInt64(*Section, *Key, Value);
            }
            LocalLCF.Info->Upserts.Add(FConfigurationSectionKey(Section, Key), FConfigurationSetting(Value));
            LocalLCF.Info->bIsModified = true;
        }));
    return true;
}

bool FConfigurationWriterLegacy::SetString(
    const FString &Key,
    const FString &Value,
    const FString &Section,
    EConfigurationFileType File,
    FName Platform)
{
    FConfigurationLoadedFileLegacy LCF = GetConfigFile(GetFilePath(File, Platform));
    if (!LCF.F)
    {
        return false;
    }
    FString ExistingValue;
    if (LCF.F->GetString(*Section, *Key, ExistingValue))
    {
        if (ExistingValue == Value)
        {
            // This already matches the intended value, don't
            // do anything.
            return false;
        }
    }

    this->FilesQueuedForModify.Add(FConfigurationWriterLegacyQueuedFileToModify(File, Platform));
    this->QueuedDelegates.Add(
        FConfigurationWriterLegacyQueuedDelegate::CreateLambda([this, File, Platform, Section, Key, Value]() {
            FConfigurationLoadedFileLegacy LocalLCF = GetConfigFile(GetFilePath(File, Platform));
            if (!LocalLCF.F)
            {
                UE_LOG(
                    LogRedpointEOSEditor,
                    Error,
                    TEXT("GetConfigFile failed to load file during delegate callback."));
                return;
            }
            LocalLCF.F->SetString(*Section, *Key, *Value);
            LocalLCF.F->Dirty = true;
            if (LocalLCF.SF)
            {
                LocalLCF.SF->SetString(*Section, *Key, *Value);
            }
            LocalLCF.Info->Upserts.Add(FConfigurationSectionKey(Section, Key), FConfigurationSetting(Value));
            LocalLCF.Info->bIsModified = true;
        }));
    return true;
}

bool FConfigurationWriterLegacy::ReplaceArray(
    const FString &Key,
    const TArray<FString> &Value,
    const FString &Section,
    EConfigurationFileType File,
    FName Platform)
{
    FConfigurationLoadedFileLegacy LCF = GetConfigFile(GetFilePath(File, Platform));
    if (!LCF.F)
    {
        return false;
    }

    TArray<FString> ExistingValue;
    if (LCF.F->GetArray(*Section, *Key, ExistingValue))
    {
        if (ExistingValue.Num() == Value.Num())
        {
            bool bExactMatch = true;
            for (int32 i = 0; i < ExistingValue.Num(); i++)
            {
                if (ExistingValue[i] != Value[i])
                {
                    bExactMatch = false;
                    break;
                }
            }
            if (bExactMatch)
            {
                // This already matches the intended value, don't
                // do anything.
                return false;
            }
        }
    }

    // Arrays in config are super broken across a lot of scenarios. We have to force a full reload.
    this->bRequireFullReload = true;

    this->FilesQueuedForModify.Add(FConfigurationWriterLegacyQueuedFileToModify(File, Platform));
    this->QueuedDelegates.Add(
        FConfigurationWriterLegacyQueuedDelegate::CreateLambda([this, File, Platform, Section, Key, Value]() {
            FConfigurationLoadedFileLegacy LocalLCF = GetConfigFile(GetFilePath(File, Platform));
            if (!LocalLCF.F)
            {
                UE_LOG(
                    LogRedpointEOSEditor,
                    Error,
                    TEXT("GetConfigFile failed to load file during delegate callback."));
                return;
            }

#if REDPOINT_EOS_UE_5_4_OR_LATER
            LocalLCF.F->RemoveKeyFromSection(*Section, *Key);
            LocalLCF.F->RemoveKeyFromSection(*Section, FName(*FString::Printf(TEXT("!%s"), *Key)));
            LocalLCF.F->RemoveKeyFromSection(*Section, FName(*FString::Printf(TEXT("+%s"), *Key)));
            if (LocalLCF.SF)
            {
                LocalLCF.SF->RemoveKeyFromSection(*Section, *Key);
                LocalLCF.SF->RemoveKeyFromSection(*Section, FName(*FString::Printf(TEXT("!%s"), *Key)));
                LocalLCF.SF->RemoveKeyFromSection(*Section, FName(*FString::Printf(TEXT("+%s"), *Key)));
            }
#else
            FConfigSection *Sec = LocalLCF.F->Find(Section);
            if (Sec)
            {
                Sec->Remove(*Key);
                Sec->Remove(FName(*FString::Printf(TEXT("!%s"), *Key)));
                Sec->Remove(FName(*FString::Printf(TEXT("+%s"), *Key)));
            }
            if (LocalLCF.SF)
            {
                FConfigSection *SecSF = LocalLCF.SF->Find(Section);
                if (SecSF)
                {
                    SecSF->Remove(*Key);
                    SecSF->Remove(FName(*FString::Printf(TEXT("!%s"), *Key)));
                    SecSF->Remove(FName(*FString::Printf(TEXT("+%s"), *Key)));
                }
            }
#endif

            // We don't call SetArray here. Replacements in non-EOS sections
            // have to be done by manually writing the ! and + instructions
            // into the INI file before we do a full reload. Instead we store
            // the value into the Replacements array.
            LocalLCF.Info->Replacements.Add(FConfigurationSectionKey(Section, Key), FConfigurationSetting(Value));

            LocalLCF.F->Dirty = true;
            LocalLCF.Info->Upserts.Add(FConfigurationSectionKey(Section, Key), FConfigurationSetting(Value));
            LocalLCF.Info->bIsModified = true;
        }));
    return true;
}

bool FConfigurationWriterLegacy::Remove(
    const FString &Key,
    const FString &Section,
    EConfigurationFileType File,
    FName Platform)
{
    FConfigurationLoadedFileLegacy LCF = GetConfigFile(GetFilePath(File, Platform));
    if (!LCF.F)
    {
        return false;
    }

#if REDPOINT_EOS_UE_5_4_OR_LATER
    const FConfigSection *Sec = LCF.F->FindOrAddConfigSection(Section);
#else
    FConfigSection *Sec = LCF.F->FindOrAddSection(Section);
#endif
    if (!Sec)
    {
        return false;
    }
    if (Sec->Find(*Key) == nullptr)
    {
        return false;
    }

    this->FilesQueuedForModify.Add(FConfigurationWriterLegacyQueuedFileToModify(File, Platform));
    this->QueuedDelegates.Add(
        FConfigurationWriterLegacyQueuedDelegate::CreateLambda([this, File, Platform, Section, Key]() {
            FConfigurationLoadedFileLegacy LocalLCF = GetConfigFile(GetFilePath(File, Platform));
            if (!LocalLCF.F)
            {
                UE_LOG(
                    LogRedpointEOSEditor,
                    Error,
                    TEXT("GetConfigFile failed to load file during delegate callback."));
                return;
            }

#if REDPOINT_EOS_UE_5_4_OR_LATER
            const FConfigSection *LocalSec = LocalLCF.F->FindOrAddConfigSection(Section);
            if (!LocalSec)
            {
                UE_LOG(
                    LogRedpointEOSEditor,
                    Error,
                    TEXT("FindOrAddConfigSection failed to load file during delegate callback."));
                return;
            }
            LocalLCF.F->RemoveKeyFromSection(*Section, *Key);

            if (LocalLCF.SF)
            {
                const FConfigSection *LocalSecSF = LocalLCF.SF->FindOrAddConfigSection(Section);
                if (LocalSecSF)
                {
                    LocalLCF.SF->RemoveKeyFromSection(*Section, *Key);
                }
            }
#else
            FConfigSection *NestedSec = LocalLCF.F->FindOrAddSection(Section);
            if (!NestedSec)
            {
                UE_LOG(
                    LogRedpointEOSEditor,
                    Error,
                    TEXT("FindOrAddSection failed to load file during delegate callback."));
                return;
            }
            NestedSec->Remove(*Key);

            if (LocalLCF.SF)
            {
                FConfigSection *SecSF = LocalLCF.SF->FindOrAddSection(Section);
                if (SecSF)
                {
                    SecSF->Remove(*Key);
                }
            }
#endif

            LocalLCF.F->Dirty = true;
            LocalLCF.Info->Removals.Add(FConfigurationSectionKey(Section, Key));
            LocalLCF.Info->bIsModified = true;
        }));
    return true;
}

EConfigurationWriterFlushResult FConfigurationWriterLegacy::FlushChanges()
{
    // Attempt to check out all of the relevant files.
    TSet<FString> FilePaths;
    if (USourceControlHelpers::IsEnabled())
    {
        for (const auto &FM : this->FilesQueuedForModify)
        {
            FilePaths.Add(GetFilePath(FM.Get<0>(), FM.Get<1>()));
        }
        TSet<FString> FilePathsForCheckout;
        for (const FString &FilePath : FilePaths.Array())
        {
            FSourceControlState SCState = USourceControlHelpers::QueryFileState(FilePath, true);
            if (!SCState.bIsUnknown && !SCState.bCanAdd && SCState.bIsSourceControlled)
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
            Redpoint::EOS::Core::Editor::FEditorSignalling::OnEditorCustomSignal().Broadcast(
                TEXT("Configuration"),
                TEXT("SourceControlPreventingConfigSave"));
            return EConfigurationWriterFlushResult::FailedToCheckOutFiles;
        }
    }

    // Enumerate all of the delegates to apply writes to the (now read-write) config files.
    for (const auto &Delegate : this->QueuedDelegates)
    {
        Delegate.ExecuteIfBound();
    }

    for (const auto &KV : this->EOSLoadedFiles)
    {
        check(KV.Value->Path == KV.Key);
        FConfigurationLoadedFileLegacy LCF = this->GetConfigFile(KV.Value->Path);
        if (LCF.F != nullptr)
        {
#if REDPOINT_EOS_UE_5_4_OR_LATER
            const FConfigSection *UnwantedSection =
                LCF.F->FindSection(TEXT("/Script/OnlineSubsystemEOSEditor.OnlineSubsystemEOSEditorConfig"));
#else
            FConfigSection *UnwantedSection =
                LCF.F->Find(TEXT("/Script/OnlineSubsystemEOSEditor.OnlineSubsystemEOSEditorConfig"));
#endif
            if (UnwantedSection)
            {
                // Remove the section that gets automatically added by the config serialization system if it already
                // exists.
                LCF.F->Remove(TEXT("/Script/OnlineSubsystemEOSEditor.OnlineSubsystemEOSEditorConfig"));
                LCF.F->Dirty = true;
                LCF.Info->bIsModified = true;
            }
        }
    }

    for (const auto &KV : this->EOSLoadedFiles)
    {
        if (KV.Value->bIsModified)
        {
            check(KV.Value->Path == KV.Key);
            FConfigurationLoadedFileLegacy LCF = this->GetConfigFile(KV.Value->Path);
            check(LCF.F != nullptr);

            // Write the changes to disk.
            LCF.F->NoSave = false;
            LCF.F->Write(KV.Value->Path);

            if (this->bRequireFullReload && LCF.Info->Replacements.Num() > 0)
            {
                // Prevent further writes by the editor.
                LCF.F->NoSave = true;
                LCF.F->Dirty = false;

                // Bucket replacements by section.
                TMap<FString, TArray<TTuple<FString, TArray<FString>>>> KeysAndValuesBySection;
                for (const auto &Replacement : LCF.Info->Replacements)
                {
                    if (!KeysAndValuesBySection.Contains(Replacement.Key.Section))
                    {
                        KeysAndValuesBySection.Add(Replacement.Key.Section, TArray<TTuple<FString, TArray<FString>>>());
                    }
                    KeysAndValuesBySection[Replacement.Key.Section].Add(
                        TTuple<FString, TArray<FString>>(Replacement.Key.Key, Replacement.Value.GetStringArray()));
                }

                // To generate array replacements, we have to manually emit the ! and +
                // instructions into the INI file. That's because SetArray emits it in the
                // form A=.. instead of !A=ClearArray \n +A=.. which is required for
                // NetDriver configuration.
                FString Contents;
                if (FFileHelper::LoadFileToString(Contents, *KV.Value->Path))
                {
                    const TCHAR *Ptr = Contents.Len() > 0 ? *Contents : nullptr;
                    const TCHAR *Original = Ptr;
                    const TCHAR *LastEndOfLineWithContent = Ptr;
                    bool Done = false;
                    FString CurrentSection = TEXT("");
                    while (!Done && Ptr != nullptr)
                    {
                        while (*Ptr == '\r' || *Ptr == '\n')
                        {
                            Ptr++;
                        }

                        const TCHAR *BeforeMove = Ptr;

                        FString TheLine;
                        int32 LinesConsumed = 0;
                        FParse::LineExtended(&Ptr, TheLine, LinesConsumed, false);
                        if (Ptr == nullptr || *Ptr == 0)
                        {
                            Done = true;
                        }

                        TCHAR *Start = const_cast<TCHAR *>(*TheLine);

                        while (*Start && FChar::IsWhitespace(Start[FCString::Strlen(Start) - 1]))
                        {
                            Start[FCString::Strlen(Start) - 1] = 0;
                        }

                        if (*Start == '[' && Start[FCString::Strlen(Start) - 1] == ']')
                        {
                            // If this is the end of a section we care about, emit our entries prior to Start.
                            if (KeysAndValuesBySection.Contains(CurrentSection))
                            {
                                TArray<FString> NewLines;
                                for (const auto &Entry : KeysAndValuesBySection[CurrentSection])
                                {
                                    NewLines.Add(FString::Printf(TEXT("!%s=ClearArray"), *Entry.Key));
                                    for (const auto &Subentry : Entry.Value)
                                    {
                                        NewLines.Add(FString::Printf(TEXT("+%s=%s"), *Entry.Key, *Subentry));
                                    }
                                }
                                FString NewContent = FString::Join(NewLines, TEXT("\n"));
                                auto NewOffset = Ptr - Original + NewContent.Len();
                                Contents = Contents.Mid(0, (int32)(LastEndOfLineWithContent - Original)) + +TEXT("\n") +
                                           NewContent + Contents.Mid((int32)(LastEndOfLineWithContent - Original));
                                Original = *Contents;
                                Ptr = (*Contents) + NewOffset;
                                LastEndOfLineWithContent = Ptr;
                                KeysAndValuesBySection.Remove(CurrentSection);
                            }

                            // This is a section.
                            Start++;
                            Start[FCString::Strlen(Start) - 1] = 0;
                            CurrentSection = Start;
                        }

                        if (!FString(Start).IsEmpty())
                        {
                            LastEndOfLineWithContent = BeforeMove + FCString::Strlen(Start);
                        }

                        // We don't care about any other types of lines.
                    }

                    Contents = Contents.TrimEnd();

                    if (KeysAndValuesBySection.Num() > 0)
                    {
                        // For any sections we haven't emitted, they don't already exist in the file, so
                        // we have to add them at the end.
                        TArray<FString> ExtendedLines;
                        for (const auto &Sec : KeysAndValuesBySection)
                        {
                            ExtendedLines.Add(TEXT(""));
                            ExtendedLines.Add(FString::Printf(TEXT("[%s]"), *Sec.Key));
                            for (const auto &Entry : KeysAndValuesBySection[Sec.Key])
                            {
                                ExtendedLines.Add(FString::Printf(TEXT("!%s=ClearArray"), *Entry.Key));
                                for (const auto &Subentry : Entry.Value)
                                {
                                    ExtendedLines.Add(FString::Printf(TEXT("+%s=%s"), *Entry.Key, *Subentry));
                                }
                            }
                        }
                        Contents = (Contents + TEXT("\n")).TrimStart() + FString::Join(ExtendedLines, TEXT("\n"));
                    }

                    FFileHelper::SaveStringToFile(Contents, *KV.Value->Path);
                }
            }

            if (!this->bRequireFullReload)
            {
                // Sync the individual changes up to other config files
                // in memory, based on their cache keys.
                for (const FString &IniPairName : GConfig->GetFilenames())
                {
                    FConfigFile *ConfigFile = Redpoint::EOS::Config::FEngineConfigHelpers::FindByName(IniPairName);
                    if (ConfigFile != nullptr && ConfigFile->Num() > 0)
                    {
                        for (const auto &H : ConfigFile->SourceIniHierarchy)
                        {
#if REDPOINT_EOS_UE_5_1_OR_LATER
                            if (H.Value == KV.Value->Path)
#else
                            if (H.Value.Filename == KV.Value->Path)
#endif
                            {
                                // Apply our changes to this file as well.
                                for (const auto &Upsert : KV.Value->Upserts)
                                {
                                    switch (Upsert.Value.GetType())
                                    {
                                    case EConfigurationSettingType::Bool:
                                        ConfigFile->SetString(
                                            *Upsert.Key.Section,
                                            *Upsert.Key.Key,
                                            Upsert.Value.GetBool() ? TEXT("True") : TEXT("False"));
                                        break;
                                    case EConfigurationSettingType::String:
                                        ConfigFile->SetString(
                                            *Upsert.Key.Section,
                                            *Upsert.Key.Key,
                                            *Upsert.Value.GetString());
                                        break;
                                    case EConfigurationSettingType::StringArray:
                                        SetArray(
                                            ConfigFile,
                                            *Upsert.Key.Section,
                                            *Upsert.Key.Key,
                                            Upsert.Value.GetStringArray());
                                        break;
                                    default:
                                        checkf(false, TEXT("Unsupported EConfigurationSettingType value."));
                                        break;
                                    }
                                }
                                for (const auto &Remove : KV.Value->Removals)
                                {
#if REDPOINT_EOS_UE_5_4_OR_LATER
                                    const FConfigSection *Sec = ConfigFile->FindOrAddConfigSection(*Remove.Section);
                                    if (Sec)
                                    {
                                        ConfigFile->RemoveKeyFromSection(*Remove.Section, *Remove.Key);
                                    }
#else
                                    FConfigSection *Sec = ConfigFile->FindOrAddSection(*Remove.Section);
                                    if (Sec)
                                    {
                                        Sec->Remove(*Remove.Key);
                                    }
#endif
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (this->bRequireFullReload)
    {
        TSet<FName> AllImpactedFiles;
        for (const auto &KV : this->FilesQueuedForModify)
        {
            switch (KV.Get<0>())
            {
            case EConfigurationFileType::Engine:
                AllImpactedFiles.Add(FName(TEXT("Engine")));
                break;
            case EConfigurationFileType::Game:
                AllImpactedFiles.Add(FName(TEXT("Game")));
                break;
            case EConfigurationFileType::Editor:
                AllImpactedFiles.Add(FName(TEXT("Editor")));
                break;
            case EConfigurationFileType::DedicatedServer:
            case EConfigurationFileType::TrustedClient:
                break;
            default:
                checkf(false, TEXT("FEOSConfigurationWriter handling unsupported EConfigurationFileType"));
            }
        }

        // Reload the configuration hierarchy so the defaults we just wrote will propagate
        // to the platform-specific ini file registered in GEngineIni.
        for (const FString &IniPairName : GConfig->GetFilenames())
        {
            FConfigFile *ConfigFile = Redpoint::EOS::Config::FEngineConfigHelpers::FindByName(IniPairName);
            if (ConfigFile != nullptr && ConfigFile->Num() > 0)
            {
                FName BaseName = ConfigFile->Name;
                if (BaseName.IsNone())
                {
                    // Do not attempt to reload configuration files that have NAME_None.
                    continue;
                }
                if (!AllImpactedFiles.Contains(BaseName))
                {
                    // We didn't modify this file, so we don't need to reload it.
                    continue;
                }

                // @warning: Do not change this from a verify() to a soft warning! If this call fails,
                // the editor will throw away all the user's existing settings!
                UE_LOG(LogRedpointEOSEditor, Verbose, TEXT("Reloading configuration file: %s"), *BaseName.ToString());
                verify(FConfigCacheIni::LoadLocalIniFile(*ConfigFile, *BaseName.ToString(), true, nullptr, true));
            }
        }
    }

    // Mark all the config files we modified for add.
    if (USourceControlHelpers::IsEnabled())
    {
        USourceControlHelpers::MarkFilesForAdd(FilePaths.Array(), true);
    }
    return EConfigurationWriterFlushResult::Success;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()
