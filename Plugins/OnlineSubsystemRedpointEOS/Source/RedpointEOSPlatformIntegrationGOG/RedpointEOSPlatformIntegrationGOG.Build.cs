// Copyright June Rhodes. All Rights Reserved.

using System;
using UnrealBuildTool;

public class RedpointEOSPlatformIntegrationGOG : RedpointEOSCPlusPlusModuleRules
{
    public RedpointEOSPlatformIntegrationGOG(ReadOnlyTargetRules Target) : base("REPIG", Target)
    {
        PrivateDependencyModuleNames.AddRange(new[]
        {
            "Core",
            "CoreUObject",
            "RedpointEOSPlatform",
            "RedpointEOSPlatformIntegrationOSSv1",
            "RedpointEOSAuth",
            "RedpointEOSCore",
        });

        /* PRECOMPILED REMOVE BEGIN */

        if (Target.Platform == UnrealTargetPlatform.Win64 ||
            Target.Platform == UnrealTargetPlatform.Mac)
        {
            if (RedpointEOSBuild.GetConfigBool(Target, "ENABLE_GOG", false))
            {
                PrivateDefinitions.Add("EOS_GOG_ENABLED=1");

                PrivateDependencyModuleNames.AddRange(new[]
                {
                    "OnlineSubsystem",
                    "OnlineSubsystemUtils",
                    "OnlineSubsystemGOG",
                    "GalaxySDK",
                    "RedpointEOSSDK",
                    "Engine",
                });
            }
            else
            {
                PrivateDefinitions.Add("EOS_GOG_ENABLED=0");
            }
        }
        else
        {
            PrivateDefinitions.Add("EOS_GOG_ENABLED=0");
        }

        /* PRECOMPILED REMOVE END */
    }
}