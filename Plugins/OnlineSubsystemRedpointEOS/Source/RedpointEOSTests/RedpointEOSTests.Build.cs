// Copyright June Rhodes. All Rights Reserved.

using UnrealBuildTool;

public class RedpointEOSTests : RedpointEOSCPlusPlusModuleRules
{
    public RedpointEOSTests(ReadOnlyTargetRules Target) : base("RET", Target)
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
                "OnlineSubsystem",
                "RedpointEOSSDK",
                "RedpointEOSAntiCheat",
                "RedpointEOSAPI",
                "RedpointEOSAsync",
                "RedpointEOSAuth",
                "RedpointEOSBuild",
                "RedpointEOSCompat",
                "RedpointEOSConfig",
                "RedpointEOSCore",
                "RedpointEOSRooms",
                "RedpointEOSInterfaces",
                "RedpointEOSIdentity",
                "RedpointEOSUserCache",
                "RedpointEOSNetworking",
                "RedpointEOSPresence",
                "RedpointEOSRooms",
                "RedpointEOSStorage",
                "RedpointEOSUserCache",
                "Json",
                "Sockets",
                "VoiceChat",
            });

            PublicDefinitions.Add("REDPOINT_EOS_ACCESS_INTERNALS=1");
        }
        /* PRECOMPILED REMOVE END */
    }
}