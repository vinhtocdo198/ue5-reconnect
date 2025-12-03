// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditor/Config/Branch/ApplyPendingChanges.h"

#if REDPOINT_EOS_UE_5_5_OR_LATER
#include "HAL/FileManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "RedpointEOSEditor/Config/Branch/IndexConfigLines.h"
#include "RedpointEOSEditor/Config/Branch/ParseBufferToConfigLines.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if REDPOINT_EOS_UE_5_5_OR_LATER

namespace REDPOINT_EOS_FILE_NS_ID(2590437723, Redpoint::EOS::Editor::Config::Branch)
{
using namespace ::Redpoint::EOS::Editor::Config::Branch;

FString ApplyPendingChanges(FConfigIndex &ConfigIndex, const FConfigurationWriterPendingChanges &PendingChanges)
{
    // Determine if we should insert using Windows newlines by default.
    TOptional<bool> bUsingWindowsNewlines;
    for (const auto &Line : ConfigIndex.PreSectionLines)
    {
        if (Line.bIsWindowsNewlineTerminated)
        {
            bUsingWindowsNewlines = true;
        }
        else if (!bUsingWindowsNewlines.IsSet())
        {
            bUsingWindowsNewlines = false;
        }
    }
    for (const auto &Section : ConfigIndex.Sections)
    {
        for (const auto &Line : Section.LinesInSection)
        {
            if (Line.bIsWindowsNewlineTerminated)
            {
                bUsingWindowsNewlines = true;
            }
            else if (!bUsingWindowsNewlines.IsSet())
            {
                bUsingWindowsNewlines = false;
            }
        }
    }
    if (!bUsingWindowsNewlines.IsSet())
    {
#if PLATFORM_WINDOWS
        bUsingWindowsNewlines = true;
#else
        bUsingWindowsNewlines = false;
#endif
    }

    auto AddEmptyLineToLastSectionIfNeeded = [&ConfigIndex, bUsingWindowsNewlines]() {
        if (ConfigIndex.Sections.Num() >= 1)
        {
            // Add an empty line to the last section for spacing if needed.
            auto &LastLine = ConfigIndex.Sections.Last().LinesInSection.Last();
            if (LastLine.Type == EConfigLineType::CommentOrWhitespace || !LastLine.PhysicalLine.IsEmpty() ||
                !LastLine.PhysicalLineTrailingComment.IsEmpty())
            {
                auto NewLine = FConfigLine(TEXT(""), TEXT(""), TEXT(""), *bUsingWindowsNewlines);
                NewLine.Type = EConfigLineType::CommentOrWhitespace;
                ConfigIndex.Sections.Last().LinesInSection.Add(NewLine);
            }
        }
    };
    auto AddOrGetSection = [&ConfigIndex, &AddEmptyLineToLastSectionIfNeeded, bUsingWindowsNewlines](
                               const FString &SectionName,
                               int &SectionIndexRef) -> FConfigIndexSection & {
        auto *SectionIndex = ConfigIndex.SectionIndex.Find(SectionName);
        if (SectionIndex == nullptr)
        {
            AddEmptyLineToLastSectionIfNeeded();
            auto InitialLines = TArray<FConfigLine>();
            {
                auto NewLine = FConfigLine(
                    FString::Printf(TEXT("[%s]"), *SectionName),
                    FString::Printf(TEXT("[%s]"), *SectionName),
                    TEXT(""),
                    *bUsingWindowsNewlines);
                NewLine.Type = EConfigLineType::Section;
                NewLine.SectionName = SectionName;
                InitialLines.Add(NewLine);
            }
            SectionIndex = &ConfigIndex.SectionIndex.Add(
                SectionName,
                ConfigIndex.Sections.Add(FConfigIndexSection{SectionName, InitialLines, TSet<FString>()}));
        }
        SectionIndexRef = *SectionIndex;
        return ConfigIndex.Sections[*SectionIndex];
    };
    auto EnsureSpacingAtEndOfSectionIfNeeded = [&ConfigIndex, bUsingWindowsNewlines](FConfigIndexSection &Section) {
        if (ConfigIndex.Sections.Num() >= 0 && &ConfigIndex.Sections.Last() != &Section &&
            (Section.LinesInSection.Last().Type != EConfigLineType::CommentOrWhitespace ||
             !Section.LinesInSection.Last().PhysicalLine.IsEmpty() ||
             !Section.LinesInSection.Last().PhysicalLineTrailingComment.IsEmpty()))
        {
            auto NewLine = FConfigLine(TEXT(""), TEXT(""), TEXT(""), *bUsingWindowsNewlines);
            NewLine.Type = EConfigLineType::CommentOrWhitespace;
            Section.LinesInSection.Add(NewLine);
        }
    };
    auto FindBestPlaceToAddInSection = [&ConfigIndex, bUsingWindowsNewlines](FConfigIndexSection &Section) {
        // Try to find the last non-whitespace/non-comment line in the section.
        TOptional<int32> ExistingLineIndex;
        for (int l = Section.LinesInSection.Num() - 1; l >= 0; l--)
        {
            if (Section.LinesInSection[l].Type == EConfigLineType::CommentOrWhitespace)
            {
                continue;
            }
            if (l == Section.LinesInSection.Num() - 1)
            {
                // Must use Add as the last entry is already content.
                break;
            }

            ExistingLineIndex = l + 1;
            if (!Section.LinesInSection[*ExistingLineIndex].PhysicalLine.IsEmpty() ||
                !Section.LinesInSection[*ExistingLineIndex].PhysicalLineTrailingComment.IsEmpty())
            {
                // The line immediately after the content is not empty. Create an empty new line if this section is not
                // the last section in the config file. This fixes spacing in a scenario where we're setting B=2 in this
                // kind of content:
                //
                // ---
                // [SomeSection]
                // A=1
                // // Some comment
                // [SecondSection]
                // ---
                //
                // becomes:
                //
                // ---
                // [SomeSection]
                // A=1
                // B=2
                //
                // // Some comment
                // [SecondSection]
                // ---
                if (ConfigIndex.Sections.Num() >= 0 && &ConfigIndex.Sections.Last() != &Section)
                {
                    auto NewLine = FConfigLine(TEXT(""), TEXT(""), TEXT(""), *bUsingWindowsNewlines);
                    NewLine.Type = EConfigLineType::CommentOrWhitespace;
                    Section.LinesInSection.Insert(NewLine, *ExistingLineIndex);
                }
            }
            break;
        }
        return ExistingLineIndex;
    };

    // Process string/bool upserts.
    for (const auto &Upsert : PendingChanges.Upserts)
    {
        // Get or create the section.
        int SectionIndex;
        auto &Section = AddOrGetSection(Upsert.Key.Section, SectionIndex);

        // Update or create the line.
        bool bDidSetLineAndIsNowRemovingDuplicates = false;
        for (int i = 0; i < Section.LinesInSection.Num(); i++)
        {
            if (Section.LinesInSection[i].SettingName != Upsert.Key.Key)
            {
                continue;
            }

            check(Upsert.Value.GetType() != EConfigurationSettingType::StringArray);

            if (Upsert.Value.GetType() == EConfigurationSettingType::Bool)
            {
                if (Section.LinesInSection[i].Type == EConfigLineType::Set)
                {
                    if (!bDidSetLineAndIsNowRemovingDuplicates)
                    {
                        if (Section.LinesInSection[i].SettingValue !=
                            (Upsert.Value.GetBool() ? TEXT("True") : TEXT("False")))
                        {
                            Section.LinesInSection[i].SettingValue =
                                Upsert.Value.GetBool() ? TEXT("True") : TEXT("False");
                            Section.LinesInSection[i].LogicalLine = FString::Printf(
                                TEXT("%s=%s"),
                                *Section.LinesInSection[i].SettingName,
                                *Section.LinesInSection[i].SettingValue);
                            Section.LinesInSection[i].PhysicalLine = Section.LinesInSection[i].LogicalLine;
                        }
                        bDidSetLineAndIsNowRemovingDuplicates = true;
                    }
                    else
                    {
                        Section.LinesInSection.RemoveAt(i--);
                    }
                }
                else
                {
                    check(
                        Section.LinesInSection[i].Type != EConfigLineType::CommentOrWhitespace &&
                        Section.LinesInSection[i].Type != EConfigLineType::Section);
                    Section.LinesInSection.RemoveAt(i--);
                }
            }
            else if (Upsert.Value.GetType() == EConfigurationSettingType::String)
            {
                if (Section.LinesInSection[i].Type == EConfigLineType::Set)
                {
                    if (!bDidSetLineAndIsNowRemovingDuplicates)
                    {
                        if (Section.LinesInSection[i].SettingValue != Upsert.Value.GetString())
                        {
                            Section.LinesInSection[i].SettingValue = Upsert.Value.GetString();
                            Section.LinesInSection[i].LogicalLine = FString::Printf(
                                TEXT("%s=%s"),
                                *Section.LinesInSection[i].SettingName,
                                *Section.LinesInSection[i].SettingValue);
                            Section.LinesInSection[i].PhysicalLine = Section.LinesInSection[i].LogicalLine;
                        }
                        bDidSetLineAndIsNowRemovingDuplicates = true;
                    }
                    else
                    {
                        Section.LinesInSection.RemoveAt(i--);
                    }
                }
                else
                {
                    check(
                        Section.LinesInSection[i].Type != EConfigLineType::CommentOrWhitespace &&
                        Section.LinesInSection[i].Type != EConfigLineType::Section);
                    Section.LinesInSection.RemoveAt(i--);
                }
            }
        }
        if (!bDidSetLineAndIsNowRemovingDuplicates)
        {
            check(Upsert.Value.GetType() != EConfigurationSettingType::StringArray);

            auto ExistingLineIndex = FindBestPlaceToAddInSection(Section);

            auto AddLine = [&Section, &ExistingLineIndex](const FConfigLine &NewLine) {
                if (ExistingLineIndex.IsSet())
                {
                    Section.LinesInSection.Insert(NewLine, (*ExistingLineIndex)++);
                }
                else
                {
                    Section.LinesInSection.Add(NewLine);
                }
            };

            if (Upsert.Value.GetType() == EConfigurationSettingType::Bool)
            {
                auto NewLineContent = FString::Printf(
                    TEXT("%s=%s"),
                    *Upsert.Key.Key,
                    Upsert.Value.GetBool() ? TEXT("True") : TEXT("False"));
                auto NewLine = FConfigLine(NewLineContent, NewLineContent, TEXT(""), *bUsingWindowsNewlines);
                NewLine.Type = EConfigLineType::Set;
                NewLine.SettingName = Upsert.Key.Key;
                NewLine.SettingValue = Upsert.Value.GetBool() ? TEXT("True") : TEXT("False");
                AddLine(NewLine);
                Section.ConfigKeysPresent.Add(Upsert.Key.Key);
            }
            else if (Upsert.Value.GetType() == EConfigurationSettingType::String)
            {
                auto NewLineContent = FString::Printf(TEXT("%s=%s"), *Upsert.Key.Key, *Upsert.Value.GetString());
                auto NewLine = FConfigLine(NewLineContent, NewLineContent, TEXT(""), *bUsingWindowsNewlines);
                NewLine.Type = EConfigLineType::Set;
                NewLine.SettingName = Upsert.Key.Key;
                NewLine.SettingValue = Upsert.Value.GetString();
                AddLine(NewLine);
                Section.ConfigKeysPresent.Add(Upsert.Key.Key);
            }

            // If we added a line, and this section is not the last section in the config file, make sure there's at
            // least one empty line at the end of this section so that there's spacing between sections.
            if (!ExistingLineIndex.IsSet())
            {
                EnsureSpacingAtEndOfSectionIfNeeded(Section);
            }
        }

        // Go through other sections that match the name and remove any entries for this key under those sections.
        for (int i = 0; i < ConfigIndex.Sections.Num(); i++)
        {
            if (i == SectionIndex)
            {
                // This is the section we actually updated.
                continue;
            }
            if (ConfigIndex.Sections[i].SectionName == Upsert.Key.Section)
            {
                // This is a duplicate section, go through and remove any entries that have the same key.
                for (int l = 0; l < ConfigIndex.Sections[i].LinesInSection.Num(); l++)
                {
                    auto &Line = ConfigIndex.Sections[i].LinesInSection[l];
                    if (Line.SettingName == Upsert.Key.Key)
                    {
                        ConfigIndex.Sections[i].LinesInSection.RemoveAt(l--);
                        ConfigIndex.Sections[i].ConfigKeysPresent.Remove(Upsert.Key.Key);
                    }
                }
            }
        }
    }

    // Process string array replacements.
    for (const auto &Replacement : PendingChanges.Replacements)
    {
        // Get or create the section.
        int SectionIndex;
        auto &Section = AddOrGetSection(Replacement.Key.Section, SectionIndex);

        // Check to see whether we already have an up-to-date version of the array. If we do, then we don't need to make
        // any config changes.
        bool bHasClear = false;
        bool bIsValueInvalid = false;
        int ArrayIndex = 0;
        TOptional<int> ExistingLineIndex;
        TMap<FString, FString> TrailingCommentsForValues;
        for (int i = 0; i < Section.LinesInSection.Num(); i++)
        {
            if (Section.LinesInSection[i].SettingName != Replacement.Key.Key)
            {
                continue;
            }

            if (!ExistingLineIndex.IsSet())
            {
                ExistingLineIndex = i;
            }

            if (Section.LinesInSection[i].Type == EConfigLineType::Clear)
            {
                TrailingCommentsForValues.Add(TEXT("__CLEAR__"), Section.LinesInSection[i].PhysicalLineTrailingComment);
                bHasClear = true;
            }
            else if (Section.LinesInSection[i].Type == EConfigLineType::ArrayAddUnique)
            {
                TrailingCommentsForValues.Add(
                    Section.LinesInSection[i].SettingValue,
                    Section.LinesInSection[i].PhysicalLineTrailingComment);

                if (ArrayIndex >= Replacement.Value.GetStringArray().Num() ||
                    Section.LinesInSection[i].SettingValue != Replacement.Value.GetStringArray()[ArrayIndex])
                {
                    bIsValueInvalid = true;
                }
                else
                {
                    ArrayIndex++;
                }
            }
            else
            {
                bIsValueInvalid = true;
            }
        }
        if (!bHasClear || ArrayIndex != Replacement.Value.GetStringArray().Num())
        {
            bIsValueInvalid = true;
        }

        // If this value is not valid, then update it in this section.
        if (bIsValueInvalid)
        {
            // Remove all existing entries from this section with this key name.
            for (int l = 0; l < Section.LinesInSection.Num(); l++)
            {
                if (Section.LinesInSection[l].SettingName == Replacement.Key.Key)
                {
                    if (ExistingLineIndex.IsSet() && l < *ExistingLineIndex)
                    {
                        (*ExistingLineIndex)--;
                    }
                    Section.LinesInSection.RemoveAt(l--);
                }
            }

            if (!ExistingLineIndex.IsSet())
            {
                ExistingLineIndex = FindBestPlaceToAddInSection(Section);
            }

            auto AddLine = [&Section, &ExistingLineIndex](const FConfigLine &NewLine) {
                if (ExistingLineIndex.IsSet())
                {
                    Section.LinesInSection.Insert(NewLine, (*ExistingLineIndex)++);
                }
                else
                {
                    Section.LinesInSection.Add(NewLine);
                }
            };

            // Add the clear.
            {
                auto NewLineContent = FString::Printf(TEXT("!%s=ClearArray"), *Replacement.Key.Key);
                auto NewLine = FConfigLine(
                    NewLineContent,
                    NewLineContent,
                    TrailingCommentsForValues.FindRef(TEXT("__CLEAR__")),
                    *bUsingWindowsNewlines);
                NewLine.Type = EConfigLineType::Clear;
                NewLine.SettingName = Replacement.Key.Key;
                NewLine.SettingValue = TEXT("ClearArray");
                AddLine(NewLine);
                Section.ConfigKeysPresent.Add(Replacement.Key.Key);
            }

            // Add the array entries.
            for (auto &Entry : Replacement.Value.GetStringArray())
            {
                auto NewLineContent = FString::Printf(TEXT("+%s=%s"), *Replacement.Key.Key, *Entry);
                auto NewLine = FConfigLine(
                    NewLineContent,
                    NewLineContent,
                    TrailingCommentsForValues.FindRef(Entry),
                    *bUsingWindowsNewlines);
                NewLine.Type = EConfigLineType::ArrayAddUnique;
                NewLine.SettingName = Replacement.Key.Key;
                NewLine.SettingValue = Entry;
                AddLine(NewLine);
                Section.ConfigKeysPresent.Add(Replacement.Key.Key);
            }

            // If we added a line, and this section is not the last section in the config file, make sure there's at
            // least one empty line at the end of this section so that there's spacing between sections.
            if (!ExistingLineIndex.IsSet())
            {
                EnsureSpacingAtEndOfSectionIfNeeded(Section);
            }
        }

        // Even if the value was valid in that previous section, still scan other sections and remove duplicate keys
        // (since they might override the otherwise valid value in this section).
        for (int i = 0; i < ConfigIndex.Sections.Num(); i++)
        {
            if (i == SectionIndex)
            {
                // This is the section we actually updated.
                continue;
            }
            if (ConfigIndex.Sections[i].SectionName == Replacement.Key.Section)
            {
                // This is a duplicate section, go through and remove any entries that have the same key.
                for (int l = 0; l < ConfigIndex.Sections[i].LinesInSection.Num(); l++)
                {
                    auto &Line = ConfigIndex.Sections[i].LinesInSection[l];
                    if (Line.SettingName == Replacement.Key.Key)
                    {
                        ConfigIndex.Sections[i].LinesInSection.RemoveAt(l--);
                        ConfigIndex.Sections[i].ConfigKeysPresent.Remove(Replacement.Key.Key);
                    }
                }
            }
        }
    }

    // Process removals.
    for (const auto &Removal : PendingChanges.Removals)
    {
        // Iterate through all sections and remove matching entries.
        for (auto &Section : ConfigIndex.Sections)
        {
            if (Section.ConfigKeysPresent.Contains(Removal.Key))
            {
                for (int l = 0; l < Section.LinesInSection.Num(); l++)
                {
                    auto &Line = Section.LinesInSection[l];
                    if (Line.SettingName == Removal.Key)
                    {
                        Section.LinesInSection.RemoveAt(l--);
                        Section.ConfigKeysPresent.Remove(Removal.Key);
                    }
                }
            }
        }
    }

    // Build the final result.
    FString FinalResult = TEXT("");
    for (const auto &PreSectionLine : ConfigIndex.PreSectionLines)
    {
        FinalResult += PreSectionLine.PhysicalLine;
        FinalResult += PreSectionLine.PhysicalLineTrailingComment;
        FinalResult += PreSectionLine.bIsWindowsNewlineTerminated ? TEXT("\r\n") : TEXT("\n");
    }
    for (const auto &Section : ConfigIndex.Sections)
    {
        for (const auto &Line : Section.LinesInSection)
        {
            FinalResult += Line.PhysicalLine;
            FinalResult += Line.PhysicalLineTrailingComment;
            FinalResult += Line.bIsWindowsNewlineTerminated ? TEXT("\r\n") : TEXT("\n");
        }
    }
    return FinalResult;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FApplyPendingChangesTest,
    "Redpoint.EOS.Editor.Config.ApplyPendingChanges",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);
bool FApplyPendingChangesTest::RunTest(const FString &Parameters)
{
    auto Plugin = IPluginManager::Get().FindPlugin("OnlineSubsystemRedpointEOS");
    if (!this->TestTrue(TEXT("Expected to find 'OnlineSubsystemRedpointEOS' plugin."), Plugin.IsValid()))
    {
        return true;
    }

    TArray<FString> ConfigTestFiles;
    IFileManager::Get().FindFiles(
        ConfigTestFiles,
        *(Plugin->GetBaseDir() / TEXT("Resources") / TEXT("ConfigTests")),
        TEXT(".txt"));
    this->TestTrue(
        TEXT("Expected to find at least one config test under Resources/ConfigTests"),
        ConfigTestFiles.Num() >= 1);

    for (const auto &File : ConfigTestFiles)
    {
        this->AddInfo(FString::Printf(TEXT("Testing with config file: '%s'"), *File));

        FString Contents;
        if (!this->TestTrue(
                TEXT("Expected to be able to read test file."),
                FFileHelper::LoadFileToString(
                    Contents,
                    *(Plugin->GetBaseDir() / TEXT("Resources") / TEXT("ConfigTests") / File))))
        {
            continue;
        }

        auto FirstDashIndex = Contents.Find(TEXT("---"));
        auto OldContents = Contents.Mid(0, FirstDashIndex);
        auto Temp = Contents.Mid(FirstDashIndex + 3);
        auto SecondDashIndex = Temp.Find(TEXT("---"));
        auto Rules = Temp.Mid(0, SecondDashIndex);
        auto NewContents = Temp.Mid(SecondDashIndex + 3);

        OldContents.TrimStartAndEndInline();
        OldContents += TEXT("\r\n");

        NewContents.TrimStartAndEndInline();
        NewContents += TEXT("\r\n");

        TArray<FString> RulesArray;
        Rules.ParseIntoArrayLines(RulesArray);

        FConfigurationWriterPendingChanges PendingChanges(
            TTuple<FString, EConfigurationFileType, FName>(TEXT(""), EConfigurationFileType::Engine, NAME_None));
        for (auto &Rule : RulesArray)
        {
            Rule.TrimStartAndEndInline();
            if (Rule.IsEmpty())
            {
                continue;
            }

            TArray<FString> RuleComponents;
            Rule.ParseIntoArray(RuleComponents, TEXT(":"));
            if (!this->TestTrue(
                    FString::Printf(
                        TEXT("Expected rule line '%s' in test file '%s' to have at least 2 components, but it had %d."),
                        *Rule,
                        *File,
                        RuleComponents.Num()),
                    RuleComponents.Num() >= 2))
            {
                continue;
            }
            if (!this->TestEqual(
                    FString::Printf(
                        TEXT("Expected rule line '%s' in test file '%s' to have single character first component, but "
                             "it was '%s'."),
                        *Rule,
                        *File,
                        *RuleComponents[0]),
                    RuleComponents[0].Len(),
                    1))
            {
                continue;
            }

            TCHAR RuleType = RuleComponents[0][0];
            int ExpectedComponentCount = 0;
            switch (RuleType)
            {
            case 'S':
            case 'B':
            case 'A':
                ExpectedComponentCount = 4;
                break;
            case 'R':
                ExpectedComponentCount = 3;
                break;
            default:
                break;
            }
            if (ExpectedComponentCount == 0)
            {
                this->TestTrue(
                    FString::Printf(
                        TEXT("Expected rule line '%s' in test file '%s' has an unknown rule type of '%s'."),
                        *Rule,
                        *File,
                        *RuleComponents[0].Mid(0, 1)),
                    false);
                continue;
            }
            if (!this->TestEqual(
                    FString::Printf(
                        TEXT(
                            "Expected rule line '%s' in test file '%s' to have at least %d components, but it had %d."),
                        *Rule,
                        *File,
                        ExpectedComponentCount,
                        RuleComponents.Num()),
                    RuleComponents.Num(),
                    ExpectedComponentCount))
            {
                continue;
            }

            switch (RuleType)
            {
            case 'S':
                PendingChanges.Upserts.Add(
                    FConfigurationSectionKey(RuleComponents[1], RuleComponents[2]),
                    FConfigurationSetting(RuleComponents[3]));
                break;
            case 'B':
                PendingChanges.Upserts.Add(
                    FConfigurationSectionKey(RuleComponents[1], RuleComponents[2]),
                    FConfigurationSetting(RuleComponents[3] == TEXT("True")));
                break;
            case 'A': {
                TArray<FString> Values;
                RuleComponents[3].ParseIntoArray(Values, TEXT(";;;"));
                PendingChanges.Replacements.Add(
                    FConfigurationSectionKey(RuleComponents[1], RuleComponents[2]),
                    FConfigurationSetting(Values));
                break;
            }
            case 'R':
                PendingChanges.Removals.Add(FConfigurationSectionKey(RuleComponents[1], RuleComponents[2]));
                break;
            default:
                checkf(false, TEXT("Unhandled rule type."));
                break;
            }
        }

        auto ConfigIndex = IndexConfigLines(ParseBufferToConfigLines(OldContents));
        auto ActualContents = ApplyPendingChanges(ConfigIndex, PendingChanges);

        this->TestEqual(
            FString::Printf(
                TEXT("The actual contents '%s' do not match the expected contents '%s'"),
                *ActualContents.TrimStartAndEnd().Replace(TEXT("\r\n"), TEXT("\n")),
                *NewContents.TrimStartAndEnd().Replace(TEXT("\r\n"), TEXT("\n"))),
            ActualContents.TrimStartAndEnd().Replace(TEXT("\r\n"), TEXT("\n")),
            NewContents.TrimStartAndEnd().Replace(TEXT("\r\n"), TEXT("\n")));
    }

    return true;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()
