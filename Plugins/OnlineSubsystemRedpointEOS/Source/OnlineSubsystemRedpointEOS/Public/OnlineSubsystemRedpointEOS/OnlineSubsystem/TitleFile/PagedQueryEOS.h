// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "OnlineSubsystemTypes.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3769627127, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::TitleFile)
{

class ONLINESUBSYSTEMREDPOINTEOS_API FPagedQueryEOS : public FPagedQuery
{
public:
    /** A list of tags to use for the title file query. If this is left empty, or if you don't use FPagedQueryEOS, then
     * it defaults to a list containing just the "Default" tag. */
    TArray<FString> Tags;

    /** Constructor for FPagedQueryEOS. Internally this sets InCount to -2 which is used by the EOS implementation to
     * detect that you are passing FPagedQueryEOS instead of a normal FPagedQuery. */
    FPagedQueryEOS(const TArray<FString> &InTags)
        : FPagedQuery(0, -2)
        , Tags(InTags){};

    /** Delete all the other constructors (including copy constructors). */
    FPagedQueryEOS() = delete;
    FPagedQueryEOS(int32 InStart, int32 InCount) = delete;
    UE_NONCOPYABLE(FPagedQueryEOS);

    /** Returns if it's safe-ish to cast the FPagedQuery to FPagedQueryEOS. */
    static bool IsFPagedQueryEOS(const FPagedQuery &InQuery)
    {
        return InQuery.Count == -2;
    }
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::TitleFile
{
REDPOINT_EOS_FILE_NS_EXPORT(
    3769627127,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::TitleFile,
    FPagedQueryEOS)
}

REDPOINT_EOS_CODE_GUARD_END()