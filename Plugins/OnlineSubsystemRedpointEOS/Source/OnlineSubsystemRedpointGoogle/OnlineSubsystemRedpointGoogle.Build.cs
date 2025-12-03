// Copyright June Rhodes. All Rights Reserved.

using UnrealBuildTool;

public class OnlineSubsystemRedpointGoogle : RedpointEOSCPlusPlusModuleRules
{
    public OnlineSubsystemRedpointGoogle(ReadOnlyTargetRules Target) : base("OSRG", Target)
    {
        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core",
        });

        /* PRECOMPILED REMOVE BEGIN */
        if (!bUsePrecompiled)
        {
            if (!RedpointEOSBuild.GetConfigBool(Target, "ENABLE_GOOGLE", true))
            {
                PublicDefinitions.Add("EOS_GOOGLE_ENABLED=0");
                return;
            }

            PublicDefinitions.Add("EOS_GOOGLE_ENABLED=1");

            PrivateDependencyModuleNames.AddRange(new[]
            {
                "CoreUObject",
                "Projects",
                "Engine",
                "OnlineSubsystemUtils",
                "OnlineSubsystem",
                "RedpointEOSInterfaces",
                "RedpointEOSPlatform",
                "Launch",
                "Json",

                // Pull in WeakPtrHelpers.
                "OnlineSubsystemRedpointEOS",
            });
        }
        /* PRECOMPILED REMOVE END */
    }
}