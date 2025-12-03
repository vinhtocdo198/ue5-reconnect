// Copyright June Rhodes. All Rights Reserved.

using EpicGames.Core;
using System;
using System.IO;
using UnrealBuildTool;

public class OnlineSubsystemBlueprints : RedpointEOSCPlusPlusModuleRules
{
    public OnlineSubsystemBlueprints(ReadOnlyTargetRules Target) : base("OnlineSubsystemBlueprints", Target)
    {
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "OnlineSubsystemUtils",
                "RedpointEOSInterfaces",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "OnlineSubsystem",
                "Json",
                "VoiceChat",
                "CoreOnline",
                "RedpointEOSCore",
            }
        );
    }
}