// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if REDPOINT_EOS_UE_5_5_OR_LATER

namespace REDPOINT_EOS_FILE_NS_ID(3824934982, Redpoint::EOS::Editor::Config::Branch)
{

/**
 * An alternate version of FParse::LineExtended that preserves newlines as it parses.
 */
template <class T>
bool ParseOneLineExtended(
    const TCHAR **Stream,
    T &LogicalResult,
    T &PhysicalResult,
    T &PhysicalTrailingLineComment,
    int32 &LinesConsumed,
    bool &IsWindowsNewlineTerminated)
{
    bool bGotStream = false;
    bool bIsQuoted = false;
    bool bIgnoringRestOfLine = false;
    int32 BracketDepth = 0;

    IsWindowsNewlineTerminated = false;

    LogicalResult.Reset();
    PhysicalResult.Reset();
    PhysicalTrailingLineComment.Reset();
    LinesConsumed = 0;

    while (**Stream != TEXT('\0') && ((**Stream != TEXT('\n') && **Stream != TEXT('\r')) || BracketDepth > 0))
    {
        // Start of comments.
        if (!bIsQuoted && (*Stream)[0] == TEXT('/') && (*Stream)[1] == TEXT('/'))
        {
            bIgnoringRestOfLine = true;
        }

        // Command chaining.
        if (!bIsQuoted && **Stream == TEXT('|'))
        {
            break;
        }

        bGotStream = true;

        // bracketed line break
        if (**Stream == TEXT('\n') || **Stream == TEXT('\r'))
        {
            checkSlow(BracketDepth > 0);

            if (!bIgnoringRestOfLine)
            {
                LogicalResult += TEXT(' ');
                PhysicalResult += **Stream;
            }
            else
            {
                PhysicalTrailingLineComment += **Stream;
            }
            LinesConsumed++;
            (*Stream)++;
            if (**Stream == TEXT('\n') || **Stream == TEXT('\r'))
            {
                if (!bIgnoringRestOfLine)
                {
                    PhysicalResult += **Stream;
                }
                else
                {
                    PhysicalTrailingLineComment += **Stream;
                }
                (*Stream)++;
            }
        }
        // allow line break if the end of the line is a backslash
        else if (!bIsQuoted && (*Stream)[0] == TEXT('\\') && ((*Stream)[1] == TEXT('\n') || (*Stream)[1] == TEXT('\r')))
        {
            if (!bIgnoringRestOfLine)
            {
                LogicalResult += TEXT(' ');
                PhysicalResult += FStringView(*Stream, 2);
            }
            else
            {
                PhysicalTrailingLineComment += FStringView(*Stream, 2);
            }
            LinesConsumed++;
            (*Stream) += 2;
            if (**Stream == TEXT('\n') || **Stream == TEXT('\r'))
            {
                if (!bIgnoringRestOfLine)
                {
                    PhysicalResult += **Stream;
                }
                else
                {
                    PhysicalTrailingLineComment += **Stream;
                }
                (*Stream)++;
            }
        }
        // check for starting or ending brace
        else if (!bIsQuoted && **Stream == TEXT('{'))
        {
            BracketDepth++;
            if (!bIgnoringRestOfLine)
            {
                PhysicalResult += **Stream;
            }
            else
            {
                PhysicalTrailingLineComment += **Stream;
            }
            (*Stream)++;
        }
        else if (!bIsQuoted && **Stream == TEXT('}') && BracketDepth > 0)
        {
            BracketDepth--;
            if (!bIgnoringRestOfLine)
            {
                PhysicalResult += **Stream;
            }
            else
            {
                PhysicalTrailingLineComment += **Stream;
            }
            (*Stream)++;
        }
        // specifically consume escaped backslashes and quotes within quoted strings
        else if (
            bIsQuoted && !bIgnoringRestOfLine && (*Stream)[0] == TEXT('\\') &&
            ((*Stream)[1] == TEXT('\"') || (*Stream)[1] == TEXT('\\')))
        {
            if (!bIgnoringRestOfLine)
            {
                LogicalResult += FStringView(*Stream, 2);
                PhysicalResult += FStringView(*Stream, 2);
            }
            else
            {
                PhysicalTrailingLineComment += FStringView(*Stream, 2);
            }
            (*Stream) += 2;
        }
        else
        {
            bIsQuoted = bIsQuoted ^ (**Stream == TEXT('\"'));

            // Got stuff.
            if (!bIgnoringRestOfLine)
            {
                LogicalResult += **Stream;
                PhysicalResult += **Stream;
            }
            else
            {
                PhysicalTrailingLineComment += **Stream;
            }
            (*Stream)++;
        }
    }
    if (**Stream == 0)
    {
        if (bGotStream)
        {
            LinesConsumed++;
        }
    }
    else
    {
        // Eat up exactly one CR/LF.
        if (**Stream == TEXT('\r') || **Stream == TEXT('\n'))
        {
            LinesConsumed++;
            if (**Stream == TEXT('\r'))
            {
                IsWindowsNewlineTerminated = true;
                (*Stream)++;
            }
            if (**Stream == TEXT('\n'))
            {
                (*Stream)++;
            }
        }
    }

    return **Stream != TEXT('\0') || bGotStream;
}

}

namespace Redpoint::EOS::Editor::Config::Branch
{
REDPOINT_EOS_FILE_NS_EXPORT(3824934982, Redpoint::EOS::Editor::Config::Branch, ParseOneLineExtended);
}

#endif

REDPOINT_EOS_CODE_GUARD_END()
