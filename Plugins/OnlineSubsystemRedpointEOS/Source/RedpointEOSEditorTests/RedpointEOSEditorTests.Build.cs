// Copyright June Rhodes. All Rights Reserved.

using UnrealBuildTool;

public class RedpointEOSEditorTests : RedpointEOSCPlusPlusModuleRules
{
    public RedpointEOSEditorTests(ReadOnlyTargetRules Target) : base("REET", Target)
    {
        /* PRECOMPILED REMOVE BEGIN */
        if (!bUsePrecompiled)
        {
            PrivateDependencyModuleNames.AddRange(new[]
            {
                "Core",
                "Engine",
                "CoreUObject",
                "OnlineSubsystemRedpointEOS",
                "RedpointEOSAntiCheat",
                "RedpointEOSUtils",
                "RedpointEOSTests",
                "RedpointEOSCore",
                "RedpointEOSAPI",
                "RedpointEOSInterfaces",
                "RedpointEOSConfig",
                "RedpointEOSNetworking",
                "OnlineSubsystem",
                "OnlineSubsystemUtils",
                "RedpointEOSSDK",
                "Json",

                "UnrealEd",
                "LevelEditor",
                "AssetTools",
            });
        }
        /* PRECOMPILED REMOVE END */
    }
}