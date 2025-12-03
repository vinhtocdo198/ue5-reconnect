// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditor/Config/Branch/ParseBufferToConfigLines.h"

#if REDPOINT_EOS_UE_5_5_OR_LATER
#include "Misc/AutomationTest.h"
#include "RedpointEOSEditor/Config/Branch/ParseOneLineExtended.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if REDPOINT_EOS_UE_5_5_OR_LATER

namespace REDPOINT_EOS_FILE_NS_ID(25103830, Redpoint::EOS::Editor::Config::Branch)
{
using namespace ::Redpoint::EOS::Editor::Config::Branch;

TArray<FConfigLine> ParseBufferToConfigLines(const FString &Contents)
{
    static TMap<TCHAR, EConfigLineType> CommandLookup = {
        {'\0', EConfigLineType::Set},
        {'-', EConfigLineType::Remove},
        {'+', EConfigLineType::ArrayAddUnique},
        {'.', EConfigLineType::ArrayAdd},
        {'!', EConfigLineType::Clear},
        {'@', EConfigLineType::ArrayOfStructKey},
        {'*', EConfigLineType::POCArrayOfStructKey},
    };

    // Mostly adapted from FillFileFromBuffer in ConfigCacheIni.cpp, which is a function unavailable to us that would
    // otherwise let us do this parsing work.
    TArray<FConfigLine> ConfigLines;
    TOptional<FString> CurrentSectionName;
    FName CurrentKeyName;
    FStringView Buffer = Contents;
    const TCHAR *Ptr = Buffer.GetData();
    TStringBuilder<128> LogicalLine;
    TStringBuilder<128> PhysicalLine;
    TStringBuilder<128> PhysicalLineTrailingComment;
    bool IsWindowsNewlineTerminated = false;
    bool Done = false;
    while (!Done && Ptr - Buffer.GetData() < Buffer.Len())
    {
        // Grab the next line.
        int32 LinesConsumed = 0;
        ParseOneLineExtended(
            &Ptr,
            LogicalLine,
            PhysicalLine,
            PhysicalLineTrailingComment,
            LinesConsumed,
            IsWindowsNewlineTerminated);
        if (Ptr == nullptr || *Ptr == 0)
        {
            Done = true;
        }
        TCHAR *LogicalStart = const_cast<TCHAR *>(*LogicalLine);

        // Strip trailing spaces from the current line.
        while (*LogicalStart && FChar::IsWhitespace(LogicalStart[FCString::Strlen(LogicalStart) - 1]))
        {
            LogicalStart[FCString::Strlen(LogicalStart) - 1] = TEXT('\0');
        }

        // Track the new line.
        auto ConfigLine = FConfigLine(
            LogicalStart,
            PhysicalLine.ToString(),
            PhysicalLineTrailingComment.ToString(),
            IsWindowsNewlineTerminated);

        // If this config line has a trailing comment, move trailing whitespace from physical line to the comment, so
        // that when we adjust values we maintain the spacing that the developer desired.
        if (!ConfigLine.PhysicalLineTrailingComment.TrimStartAndEnd().IsEmpty())
        {
            auto TrimmedPhysicalLine = ConfigLine.PhysicalLine.TrimEnd();
            auto TrailingCharacterCount = ConfigLine.PhysicalLine.Len() - TrimmedPhysicalLine.Len();
            if (TrailingCharacterCount > 0)
            {
                ConfigLine.PhysicalLineTrailingComment.InsertAt(
                    0,
                    ConfigLine.PhysicalLine.Mid(ConfigLine.PhysicalLine.Len() - TrailingCharacterCount));
                ConfigLine.PhysicalLine = TrimmedPhysicalLine;
                ConfigLine.LogicalLine.TrimEndInline();
            }
        }

        // Detect empty lines.
        if (LogicalStart[0] == '\0')
        {
            // Add the line.
            ConfigLine.Type = EConfigLineType::CommentOrWhitespace;
            ConfigLines.Add(ConfigLine);
            continue;
        }

        // Detect sections.
        if (*LogicalStart == '[' && LogicalStart[FCString::Strlen(LogicalStart) - 1] == ']')
        {
            // Remove the brackets.
            LogicalStart++;
            LogicalStart[FCString::Strlen(LogicalStart) - 1] = TEXT('\0');

            // Start tracking this section. We don't unify sections like FillFileFromBuffer because we care about
            // mutating the config, not actually using it for evaluation.
            CurrentSectionName = LogicalStart;
            CurrentKeyName = NAME_None;

            // Add the line.
            ConfigLine.Type = EConfigLineType::Section;
            ConfigLine.SectionName = CurrentSectionName.GetValue();
            ConfigLines.Add(ConfigLine);
            continue;
        }

        // Capture lines that are comments, then continue.
        if (*LogicalStart == (TCHAR)';')
        {
            // Add the line.
            ConfigLine.Type = EConfigLineType::CommentOrWhitespace;
            ConfigLines.Add(ConfigLine);
            continue;
        }

        // We're in a section; process the lines.
        if (CurrentSectionName.IsSet() && *LogicalStart)
        {
            // Capture lines that don't have values as comments.
            TCHAR *PropertyValue = FCString::Strstr(LogicalStart, TEXT("="));
            if (PropertyValue == nullptr)
            {
                // Add the line.
                ConfigLine.Type = EConfigLineType::CommentOrWhitespace;
                ConfigLines.Add(ConfigLine);
                continue;
            }

            // LogicalStart is the start of the property name, PropertyValue is the start of the property value.

            // Terminate the property name, and advance the property value pointer past the =.
            *PropertyValue++ = TEXT('\0');

            // Strip leading whitespace from the property name.
            while (*LogicalStart && FChar::IsWhitespace(*LogicalStart))
            {
                LogicalStart++;
            }

            // Figure out what kind of operation this is.
            TCHAR Cmd = LogicalStart[0];
            if (Cmd == '+' || Cmd == '-' || Cmd == '.' || Cmd == '!' || Cmd == '@' || Cmd == '*')
            {
                LogicalStart++;
            }
            else
            {
                Cmd = TEXT('\0');
            }
            EConfigLineType *LineType = CommandLookup.Find(Cmd);
            if (LineType == nullptr)
            {
                // This is an unknown command, treat it as a comment/whitespace.
                ConfigLine.Type = EConfigLineType::CommentOrWhitespace;
                ConfigLines.Add(ConfigLine);
                continue;
            }
            ConfigLine.Type = *LineType;

            // Strip trailing spaces from the property name.
            while (*LogicalStart && FChar::IsWhitespace(LogicalStart[FCString::Strlen(LogicalStart) - 1]))
            {
                LogicalStart[FCString::Strlen(LogicalStart) - 1] = TEXT('\0');
            }

            // Assign the setting name.
            ConfigLine.SettingName = LogicalStart;

            // Strip leading whitespace from the property value.
            while (*PropertyValue && FChar::IsWhitespace(*PropertyValue))
            {
                PropertyValue++;
            }

            // Strip trailing whitespace from the property value.
            while (*PropertyValue && FChar::IsWhitespace(PropertyValue[FCString::Strlen(PropertyValue) - 1]))
            {
                PropertyValue[FCString::Strlen(PropertyValue) - 1] = TEXT('\0');
            }

            // Process the logical property value.
            FString ProcessedValue;
            if (*PropertyValue == '\"')
            {
                FParse::QuotedString(PropertyValue, ProcessedValue);
            }
            else
            {
                ProcessedValue = PropertyValue;
            }
            ConfigLine.SettingValue = PropertyValue;

            // Add the setting line.
            ConfigLines.Add(ConfigLine);
            continue;
        }
    }

    return ConfigLines;
}

// Make sure that ParseBufferToConfigLines behaves as we expect.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FParseBufferToConfigLinesTest,
    "Redpoint.EOS.Editor.Config.ConfigParse",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);
bool FParseBufferToConfigLinesTest::RunTest(const FString &Parameters)
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

    auto ActualConfigLines = ParseBufferToConfigLines(Contents);

    TArray<EConfigLineType> ExpectedLineTypes = {
        EConfigLineType::CommentOrWhitespace,
        EConfigLineType::CommentOrWhitespace,
        EConfigLineType::Section,
        EConfigLineType::CommentOrWhitespace,
        EConfigLineType::CommentOrWhitespace,
        EConfigLineType::CommentOrWhitespace,
        EConfigLineType::CommentOrWhitespace,
        EConfigLineType::CommentOrWhitespace,
        EConfigLineType::CommentOrWhitespace,
        EConfigLineType::CommentOrWhitespace,
        EConfigLineType::Set,
        EConfigLineType::CommentOrWhitespace,
        EConfigLineType::Clear,
        EConfigLineType::ArrayAddUnique,
        EConfigLineType::ArrayAddUnique,
        EConfigLineType::ArrayAddUnique,
        EConfigLineType::CommentOrWhitespace,
        EConfigLineType::CommentOrWhitespace};

    if (this->TestEqual(TEXT("Number of config lines is incorrect."), ExpectedLineTypes.Num(), ActualConfigLines.Num()))
    {
        for (int i = 0; i < ExpectedLineTypes.Num(); i++)
        {
            this->TestEqual(TEXT("Mismatched line type"), ActualConfigLines[i].Type, ExpectedLineTypes[i]);
        }

        this->TestEqual(
            TEXT("Trailing whitespace is assigned to comment"),
            ActualConfigLines[7].LogicalLine,
            TEXT("TheLineWith"));
        this->TestEqual(
            TEXT("Trailing whitespace is assigned to comment"),
            ActualConfigLines[7].PhysicalLine,
            TEXT("TheLineWith"));
        this->TestEqual(
            TEXT("Trailing whitespace is assigned to comment"),
            ActualConfigLines[7].PhysicalLineTrailingComment,
            TEXT(" // AContinuing\\\nComment "));

        this->TestEqual(TEXT("Section name is correct"), ActualConfigLines[2].SectionName, TEXT("Section"));
        this->TestEqual(TEXT("Set key name is correct"), ActualConfigLines[10].SettingName, TEXT("Value"));
        this->TestEqual(TEXT("Set key value is correct"), ActualConfigLines[10].SettingValue, TEXT("Blah"));
        this->TestEqual(TEXT("Array clear name is correct"), ActualConfigLines[12].SettingName, TEXT("SomeArray"));
        this->TestEqual(TEXT("Array clear value is correct"), ActualConfigLines[12].SettingValue, TEXT("Clear"));
        this->TestEqual(TEXT("Array add 1 name is correct"), ActualConfigLines[13].SettingName, TEXT("SomeArray"));
        this->TestEqual(TEXT("Array add 1 value is correct"), ActualConfigLines[13].SettingValue, TEXT("Entry1"));
        this->TestEqual(TEXT("Array add 2 name is correct"), ActualConfigLines[14].SettingName, TEXT("SomeArray"));
        this->TestEqual(TEXT("Array add 2 value is correct"), ActualConfigLines[14].SettingValue, TEXT("Entry2"));
        this->TestEqual(TEXT("Array add 3 name is correct"), ActualConfigLines[15].SettingName, TEXT("SomeArray"));
        this->TestEqual(TEXT("Array add 3 value is correct"), ActualConfigLines[15].SettingValue, TEXT("Entry3"));
    }

    return true;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()
