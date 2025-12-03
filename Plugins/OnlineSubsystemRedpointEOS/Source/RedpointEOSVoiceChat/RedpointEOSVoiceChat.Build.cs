// Copyright June Rhodes. All Rights Reserved.

using UnrealBuildTool;

public class RedpointEOSVoiceChat : RedpointEOSCPlusPlusModuleRules
{
    public RedpointEOSVoiceChat(ReadOnlyTargetRules Target) : base("REVC", Target)
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
                "VoiceChat",
                "RedpointEOSSDK",
                "RedpointEOSCore",
            });
        }
        /* PRECOMPILED REMOVE END */
    }
}