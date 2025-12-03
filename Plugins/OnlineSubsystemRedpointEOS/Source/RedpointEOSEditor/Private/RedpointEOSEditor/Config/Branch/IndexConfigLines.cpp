// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditor/Config/Branch/IndexConfigLines.h"

#if REDPOINT_EOS_UE_5_5_OR_LATER
#include "Misc/AutomationTest.h"
#include "RedpointEOSEditor/Config/Branch/ParseBufferToConfigLines.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if REDPOINT_EOS_UE_5_5_OR_LATER

namespace REDPOINT_EOS_FILE_NS_ID(4225613344, Redpoint::EOS::Editor::Config::Branch)
{
using namespace ::Redpoint::EOS::Editor::Config::Branch;

FConfigIndex IndexConfigLines(const TArray<FConfigLine> &ConfigLines)
{
    FConfigIndex Index;

    TOptional<FConfigIndexSection> CurrentSection;
    for (const auto &Line : ConfigLines)
    {
        if (Line.Type == EConfigLineType::Section)
        {
            if (CurrentSection.IsSet())
            {
                auto SectionIndex = Index.Sections.Add(CurrentSection.GetValue());
                Index.SectionIndex.Add(CurrentSection->SectionName, SectionIndex);
            }

            CurrentSection = FConfigIndexSection();
            CurrentSection->SectionName = Line.SectionName;
            CurrentSection->LinesInSection.Add(Line);
        }
        else if (!CurrentSection.IsSet())
        {
            Index.PreSectionLines.Add(Line);
        }
        else
        {
            CurrentSection->LinesInSection.Add(Line);
            if (Line.Type != EConfigLineType::CommentOrWhitespace)
            {
                CurrentSection->ConfigKeysPresent.Add(Line.SettingName);
            }
        }
    }
    if (CurrentSection.IsSet())
    {
        auto SectionIndex = Index.Sections.Add(CurrentSection.GetValue());
        Index.SectionIndex.Add(CurrentSection->SectionName, SectionIndex);
    }

    return Index;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FIndexConfigLinesTest,
    "Redpoint.EOS.Editor.Config.ConfigIndex",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);
bool FIndexConfigLinesTest::RunTest(const FString &Parameters)
{
    FString Contents = "\n\
\n\
[Section]\n\
// {Multiline\n\
[IgnoredSection]\n\
SomeKey=Value\n\
comment\n\
}\n\
\n\
ThisLine\\\n\
Continues   \n\
\n\
TheLineWith // AContinuing\\\n\
Comment \n\
  \n\
\n\
Value=Blah\n\
;Test\n\
!SomeArray=Clear\n\
+SomeArray=Entry1 \n\
+SomeArray=Entry2  \n\
+SomeArray=Entry3\n\
\n\
\n";
    Contents.ReplaceInline(TEXT("\r\n"), TEXT("\n"));

    auto ActualIndex = IndexConfigLines(ParseBufferToConfigLines(Contents));

    this->TestEqual(TEXT("Presection line count is 2"), ActualIndex.PreSectionLines.Num(), 2);

    if (this->TestTrue(TEXT("Section 'Section' is present"), ActualIndex.SectionIndex.Find(TEXT("Section")) != nullptr))
    {
        auto &Section = ActualIndex.Sections[ActualIndex.SectionIndex.FindChecked(TEXT("Section"))];
        this->TestEqual(
            TEXT("Section lines contains [Section] as first entry"),
            Section.LinesInSection[0].Type,
            EConfigLineType::Section);

        this->TestTrue(TEXT("Section contains 'Value' key"), Section.ConfigKeysPresent.Contains(TEXT("Value")));
        this->TestTrue(TEXT("Section contains 'SomeArray' key"), Section.ConfigKeysPresent.Contains(TEXT("SomeArray")));
        this->TestEqual(TEXT("Section contains 2 unique key names"), Section.ConfigKeysPresent.Num(), 2);

        this->TestEqual(
            TEXT("Section line contains Value=Blah set as 9th entry"),
            Section.LinesInSection[8].Type,
            EConfigLineType::Set);
        this->TestEqual(
            TEXT("Section line contains Value=Blah set as 9th entry"),
            Section.LinesInSection[8].SettingName,
            TEXT("Value"));
        this->TestEqual(
            TEXT("Section line contains Value=Blah set as 9th entry"),
            Section.LinesInSection[8].SettingValue,
            TEXT("Blah"));

        this->TestEqual(
            TEXT("Section line contains !SomeArray=Clear as 11th entry"),
            Section.LinesInSection[10].Type,
            EConfigLineType::Clear);
        this->TestEqual(
            TEXT("Section line contains !SomeArray=Clear as 11th entry"),
            Section.LinesInSection[10].SettingName,
            TEXT("SomeArray"));
        this->TestEqual(
            TEXT("Section line contains !SomeArray=Clear as 11th entry"),
            Section.LinesInSection[10].SettingValue,
            TEXT("Clear"));

        for (int i = 0; i < 2; i++)
        {
            this->TestEqual(
                TEXT("Section line contains +SomeArray=Entry{1+i} as {12+i}th entry"),
                Section.LinesInSection[11 + i].Type,
                EConfigLineType::ArrayAddUnique);
            this->TestEqual(
                TEXT("Section line contains +SomeArray=Entry{1+i} as {12+i}th entry"),
                Section.LinesInSection[11 + i].SettingName,
                TEXT("SomeArray"));
            this->TestEqual(
                TEXT("Section line contains +SomeArray=Entry{1+i} as {12+i}th entry"),
                Section.LinesInSection[11 + i].SettingValue,
                FString::Printf(TEXT("Entry%d"), 1 + i));
        }
    }

    return true;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()
