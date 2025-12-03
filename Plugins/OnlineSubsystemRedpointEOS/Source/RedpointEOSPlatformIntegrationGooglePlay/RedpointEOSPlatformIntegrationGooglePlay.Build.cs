// Copyright June Rhodes. All Rights Reserved.

using UnrealBuildTool;

public class RedpointEOSPlatformIntegrationGooglePlay : RedpointEOSCPlusPlusModuleRules
{
    public RedpointEOSPlatformIntegrationGooglePlay(ReadOnlyTargetRules Target) : base("REPIGP", Target)
    {
        PrivateDependencyModuleNames.AddRange(new[]
        {
            "Core",
            "CoreUObject",
            "RedpointEOSCore",
            "RedpointEOSAuth",
            "RedpointEOSPlatform",
            "RedpointEOSPlatformIntegrationOSSv1",
            "OnlineSubsystem",
            "OnlineSubsystemUtils",
        });

        /* PRECOMPILED REMOVE BEGIN */

        if (Target.Platform == UnrealTargetPlatform.Android)
        {
            if (RedpointEOSBuild.GetConfigBool(Target, "ENABLE_GOOGLE", true))
            {
                PrivateDependencyModuleNames.AddRange(new[]
                {
                    "OnlineSubsystemRedpointGoogle",
                });

                PrivateDefinitions.Add("EOS_GOOGLE_PLAY_ENABLED=1");
            }
            else
            {
                PrivateDefinitions.Add("EOS_GOOGLE_PLAY_ENABLED=0");
            }
        }
        else
        {
            PrivateDefinitions.Add("EOS_GOOGLE_PLAY_ENABLED=0");
        }

        /* PRECOMPILED REMOVE END */
    }
}