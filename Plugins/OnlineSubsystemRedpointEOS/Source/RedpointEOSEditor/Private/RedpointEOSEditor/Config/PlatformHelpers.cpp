// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditor/Config/PlatformHelpers.h"

#include "Misc/DataDrivenPlatformInfoRegistry.h"
#include "PlatformInfo.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1786651394, Redpoint::EOS::Editor::Config)
{

TArray<FName> GetAllPlatformNames()
{
    TArray<FName> Results;
#if DDPI_HAS_EXTENDED_PLATFORMINFO_DATA
    for (const auto &PlatformInfo : PlatformInfo::GetPlatformInfoArray())
    {
        if (PlatformInfo->VanillaInfo->PlatformType == EBuildTargetType::Game &&
            PlatformInfo->VanillaInfo->Name.ToString() != "AllDesktop")
        {
            if (PlatformInfo->VanillaInfo->Name.ToString().EndsWith("NoEditor"))
            {
                Results.Add(FName(*PlatformInfo->VanillaInfo->Name.ToString().Mid(
                    0,
                    PlatformInfo->VanillaInfo->Name.ToString().Len() - 8)));
            }
            else
            {
                Results.Add(PlatformInfo->VanillaInfo->Name);
            }
        }
    }
#endif
    return Results;
}

TArray<FName> GetConfidentialPlatformNames()
{
    TArray<FName> Results;
#if DDPI_HAS_EXTENDED_PLATFORMINFO_DATA
    for (const auto &PlatformInfo : PlatformInfo::GetPlatformInfoArray())
    {
        const FDataDrivenPlatformInfo &PlatInfo =
            FDataDrivenPlatformInfoRegistry::GetPlatformInfo(PlatformInfo->VanillaInfo->Name);

        if (PlatformInfo->VanillaInfo->PlatformType == EBuildTargetType::Game &&
            (PlatInfo.bIsConfidential || PlatformInfo->VanillaInfo->Name.IsEqual(FName(TEXT("Switch")))))
        {
            Results.Add(PlatformInfo->VanillaInfo->Name);
        }
    }
#endif
    return Results;
}

}

REDPOINT_EOS_CODE_GUARD_END()