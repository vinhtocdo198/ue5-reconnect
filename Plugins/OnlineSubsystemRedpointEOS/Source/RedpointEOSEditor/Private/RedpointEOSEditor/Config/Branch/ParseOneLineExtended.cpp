// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditor/Config/Branch/ParseOneLineExtended.h"

#if REDPOINT_EOS_UE_5_5_OR_LATER
#include "Misc/AutomationTest.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if REDPOINT_EOS_UE_5_5_OR_LATER

namespace REDPOINT_EOS_FILE_NS_ID(3824934982, Redpoint::EOS::Editor::Config::Branch)
{

// Make sure that ParseOneLineExtended behaves as we expect.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FParseOneLineExtendedTest,
    "Redpoint.EOS.Editor.Config.LineParse",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);
bool FParseOneLineExtendedTest::RunTest(const FString &Parameters)
{
    FString Contents = "\n\
\n\
[Section]\n\
// {Multiline\n\
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
\n\
\n";
    Contents.ReplaceInline(TEXT("\r\n"), TEXT("\n"));
    using FExpectedLine = TTuple<FString, FString, FString>;
    TArray<FExpectedLine> ExpectedLines = {
        FExpectedLine(TEXT(""), TEXT(""), TEXT("")),
        FExpectedLine(TEXT(""), TEXT(""), TEXT("")),
        FExpectedLine(TEXT("[Section]"), TEXT("[Section]"), TEXT("")),
        FExpectedLine(TEXT(""), TEXT(""), TEXT("// {Multiline\ncomment\n}")),
        FExpectedLine(TEXT(""), TEXT(""), TEXT("")),
        FExpectedLine(TEXT("ThisLine Continues   "), TEXT("ThisLine\\\nContinues   "), TEXT("")),
        FExpectedLine(TEXT(""), TEXT(""), TEXT("")),
        FExpectedLine(TEXT("TheLineWith "), TEXT("TheLineWith "), TEXT("// AContinuing\\\nComment ")),
        FExpectedLine(TEXT("  "), TEXT("  "), TEXT("")),
        FExpectedLine(TEXT(""), TEXT(""), TEXT("")),
        FExpectedLine(TEXT("Value=Blah"), TEXT("Value=Blah"), TEXT("")),
        FExpectedLine(TEXT(";Test"), TEXT(";Test"), TEXT("")),
        FExpectedLine(TEXT(""), TEXT(""), TEXT("")),
        FExpectedLine(TEXT(""), TEXT(""), TEXT("")),
    };

    int32 ExpectedIndex = 0;
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

        if (this->TestTrue(
                FString::Printf(TEXT("Expected not to exceed %d lines."), ExpectedLines.Num()),
                ExpectedIndex < ExpectedLines.Num()))
        {
            {
                auto Expected = ExpectedLines[ExpectedIndex].Get<0>();
                FString Actual = LogicalLine.ToString();

                this->TestEqual(
                    FString::Printf(
                        TEXT("Expected ParseOneLineExtended to return logical value '%s' at index %d, but got logical "
                             "value '%s' instead."),
                        *Expected,
                        ExpectedIndex,
                        *Actual),
                    Actual,
                    Expected);
            }
            {
                auto Expected = ExpectedLines[ExpectedIndex].Get<1>();
                FString Actual = PhysicalLine.ToString();

                this->TestEqual(
                    FString::Printf(
                        TEXT("Expected ParseOneLineExtended to return physical value '%s' at index %d, but got "
                             "physical value '%s' instead."),
                        *Expected,
                        ExpectedIndex,
                        *Actual),
                    Actual,
                    Expected);
            }
            {
                auto Expected = ExpectedLines[ExpectedIndex].Get<2>();
                FString Actual = PhysicalLineTrailingComment.ToString();

                this->TestEqual(
                    FString::Printf(
                        TEXT("Expected ParseOneLineExtended to return physical trailing comment '%s' at index %d, but "
                             "got physical trailing comment '%s' instead."),
                        *Expected,
                        ExpectedIndex,
                        *Actual),
                    Actual,
                    Expected);
            }
            {
                this->TestFalse(
                    FString::Printf(
                        TEXT("Expected ParseOneLineExtended to return IsWindowsNewlineTerminated = false at index %d, "
                             "but got true."),
                        ExpectedIndex),
                    IsWindowsNewlineTerminated);
            }
            ExpectedIndex++;
        }
    }

    this->TestEqual(
        FString::Printf(
            TEXT("Expected to process %d lines, but only processed %d lines."),
            ExpectedLines.Num(),
            ExpectedIndex),
        ExpectedIndex,
        ExpectedLines.Num());

    return true;
}

// Make sure that ParseOneLineExtended behaves as we expect.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FOnlineSubsystemEOS_Editor_Config_LineParse,
    "Redpoint.EOS.Editor.Config.LineParse",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);
bool FOnlineSubsystemEOS_Editor_Config_LineParse::RunTest(const FString &Parameters)
{
    FString Contents = "\n\
\n\
[Section]\n\
// {Multiline\n\
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
\n\
\n";
    Contents.ReplaceInline(TEXT("\r\n"), TEXT("\n"));
    using FExpectedLine = TTuple<FString, FString, FString>;
    TArray<FExpectedLine> ExpectedLines = {
        FExpectedLine(TEXT(""), TEXT(""), TEXT("")),
        FExpectedLine(TEXT(""), TEXT(""), TEXT("")),
        FExpectedLine(TEXT("[Section]"), TEXT("[Section]"), TEXT("")),
        FExpectedLine(TEXT(""), TEXT(""), TEXT("// {Multiline\ncomment\n}")),
        FExpectedLine(TEXT(""), TEXT(""), TEXT("")),
        FExpectedLine(TEXT("ThisLine Continues   "), TEXT("ThisLine\\\nContinues   "), TEXT("")),
        FExpectedLine(TEXT(""), TEXT(""), TEXT("")),
        FExpectedLine(TEXT("TheLineWith "), TEXT("TheLineWith "), TEXT("// AContinuing\\\nComment ")),
        FExpectedLine(TEXT("  "), TEXT("  "), TEXT("")),
        FExpectedLine(TEXT(""), TEXT(""), TEXT("")),
        FExpectedLine(TEXT("Value=Blah"), TEXT("Value=Blah"), TEXT("")),
        FExpectedLine(TEXT(";Test"), TEXT(";Test"), TEXT("")),
        FExpectedLine(TEXT(""), TEXT(""), TEXT("")),
        FExpectedLine(TEXT(""), TEXT(""), TEXT("")),
    };

    int32 ExpectedIndex = 0;
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

        if (this->TestTrue(
                FString::Printf(TEXT("Expected not to exceed %d lines."), ExpectedLines.Num()),
                ExpectedIndex < ExpectedLines.Num()))
        {
            {
                auto Expected = ExpectedLines[ExpectedIndex].Get<0>();
                FString Actual = LogicalLine.ToString();

                this->TestEqual(
                    FString::Printf(
                        TEXT("Expected ParseOneLineExtended to return logical value '%s' at index %d, but got logical "
                             "value '%s' instead."),
                        *Expected,
                        ExpectedIndex,
                        *Actual),
                    Actual,
                    Expected);
            }
            {
                auto Expected = ExpectedLines[ExpectedIndex].Get<1>();
                FString Actual = PhysicalLine.ToString();

                this->TestEqual(
                    FString::Printf(
                        TEXT("Expected ParseOneLineExtended to return physical value '%s' at index %d, but got "
                             "physical value '%s' instead."),
                        *Expected,
                        ExpectedIndex,
                        *Actual),
                    Actual,
                    Expected);
            }
            {
                auto Expected = ExpectedLines[ExpectedIndex].Get<2>();
                FString Actual = PhysicalLineTrailingComment.ToString();

                this->TestEqual(
                    FString::Printf(
                        TEXT("Expected ParseOneLineExtended to return physical trailing comment '%s' at index %d, but "
                             "got physical trailing comment '%s' instead."),
                        *Expected,
                        ExpectedIndex,
                        *Actual),
                    Actual,
                    Expected);
            }
            {
                this->TestFalse(
                    FString::Printf(
                        TEXT("Expected ParseOneLineExtended to return IsWindowsNewlineTerminated = false at index %d, "
                             "but got true."),
                        ExpectedIndex),
                    IsWindowsNewlineTerminated);
            }
            ExpectedIndex++;
        }
    }

    this->TestEqual(
        FString::Printf(
            TEXT("Expected to process %d lines, but only processed %d lines."),
            ExpectedLines.Num(),
            ExpectedIndex),
        ExpectedIndex,
        ExpectedLines.Num());

    return true;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()
