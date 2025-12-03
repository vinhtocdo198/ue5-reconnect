// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Containers/UnrealString.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_EDITOR

namespace REDPOINT_EOS_FILE_NS_ID(1550610155, Redpoint::EOS::Core::Editor)
{

class REDPOINTEOSCORE_API FRedpointEOSSDKInfo
{
private:
    static TOptional<FRedpointEOSSDKInfo> Instance;
    TMap<FString, TArray<FString>> SearchPathsValue;
    TMap<FString, TArray<FString>> SupportedVersionsValue;

public:
    static const FRedpointEOSSDKInfo &Get();

    TArray<FString> SearchPaths(const FString &Name) const;
    TArray<FString> SupportedVersions(const FString &Name) const;
};

}

namespace Redpoint::EOS::Core::Editor
{
REDPOINT_EOS_FILE_NS_EXPORT(1550610155, Redpoint::EOS::Core::Editor, FRedpointEOSSDKInfo)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()