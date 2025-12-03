// Copyright June Rhodes. All Rights Reserved.

using UnrealBuildTool;

public class RedpointEOSUtils : RedpointEOSCPlusPlusModuleRules
{
    public RedpointEOSUtils(ReadOnlyTargetRules Target) : base("REU", Target)
    {
        /* PRECOMPILED REMOVE BEGIN */
        if (!bUsePrecompiled)
        {
            PrivateDependencyModuleNames.AddRange(new[]
            {
                "Core",
                "Engine",
                "CoreUObject",
                "OnlineSubsystem",
                "OnlineSubsystemUtils",
                "OnlineSubsystemRedpointEOS",
            });
        }
        /* PRECOMPILED REMOVE END */
    }
}