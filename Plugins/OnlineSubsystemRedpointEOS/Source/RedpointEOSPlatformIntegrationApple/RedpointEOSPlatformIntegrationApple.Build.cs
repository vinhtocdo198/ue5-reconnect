// Copyright June Rhodes. All Rights Reserved.

using EpicGames.Core;
using System;
using UnrealBuildTool;

public class RedpointEOSPlatformIntegrationApple : RedpointEOSCPlusPlusModuleRules
{
    public RedpointEOSPlatformIntegrationApple(ReadOnlyTargetRules Target) : base("REPIA", Target)
    {
        PrivateDependencyModuleNames.AddRange(new[]
        {
            "Core",
            "CoreUObject",
            "Projects",
            "RedpointEOSAuth",
            "RedpointEOSPlatform",
        });

        /* PRECOMPILED REMOVE BEGIN */

        if (Target.Platform == UnrealTargetPlatform.IOS)
        {
            PrivateDefinitions.Add("REDPOINT_EOS_APPLE_ENABLED=1");
        }
        else
        {
            PrivateDefinitions.Add("REDPOINT_EOS_APPLE_ENABLED=0");
        }

        /* PRECOMPILED REMOVE END */
    }
}