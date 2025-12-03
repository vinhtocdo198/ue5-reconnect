// Copyright June Rhodes. All Rights Reserved.

using UnrealBuildTool;

public class RedpointEOSGauntlet : RedpointEOSCPlusPlusModuleRules
{
    public RedpointEOSGauntlet(ReadOnlyTargetRules Target) : base("REG", Target)
    {
        /* PRECOMPILED REMOVE BEGIN */
        if (!bUsePrecompiled)
        {
            PrivateDependencyModuleNames.AddRange(new[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "Gauntlet",
                "Json",
                "OnlineSubsystem",
                "RedpointEOSAsync",
            });
        }
        /* PRECOMPILED REMOVE END */
    }
}