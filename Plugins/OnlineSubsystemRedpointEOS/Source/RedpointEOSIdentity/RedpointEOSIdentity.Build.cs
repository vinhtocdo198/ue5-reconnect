// Copyright June Rhodes. All Rights Reserved.

using System;
using UnrealBuildTool;

public class RedpointEOSIdentity : RedpointEOSCPlusPlusModuleRules
{
    public RedpointEOSIdentity(ReadOnlyTargetRules Target) : base("REID", Target)
    {
        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreOnline",
            "RedpointEOSAPI",
            "RedpointEOSCore",
            "RedpointEOSCompat",
            "RedpointEOSAuth",
            "RedpointEOSUserCache",
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Engine",
            "CoreUObject",
            "RedpointEOSSDK",
            "RedpointEOSPlatform",
        });
    }
}