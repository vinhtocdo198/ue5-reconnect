// Copyright June Rhodes. All Rights Reserved.

using System;
using UnrealBuildTool;

public class OnlineSubsystemRedpointEOS : RedpointEOSCPlusPlusModuleRules
{
    public OnlineSubsystemRedpointEOS(ReadOnlyTargetRules Target) : base("OSRE", Target)
    {
        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core",
            "OnlineSubsystemUtils",
            "Projects",
            "Json",
            "VoiceChat",

            "RedpointEOSAPI",
            "RedpointEOSAuth",
            "RedpointEOSConfig",
            "RedpointEOSCore",
            "RedpointEOSRooms",
            "RedpointEOSPlatform",
            "RedpointEOSUserCache",
            "RedpointEOSStorage",

            // Now necessary for the EOSError.h header in Public.
            "OnlineSubsystem",

            // Now necessary as UEOSSubsystem is a UObject which depends on UUserWidget (and because it is a UObject, it can't be excluded for server-only builds).
            "UMG",
        });

        PrivateDependencyModuleNames.AddRange(new[]
        {
            "Core",

            "RedpointEOSAntiCheat",
            "RedpointEOSAsync",
            "RedpointEOSCompat",
            "RedpointEOSFriends",
            "RedpointEOSIdentity",
            "RedpointEOSInterfaces",
            "RedpointEOSLicenseValidation",
            "RedpointEOSNetworking",
            "RedpointEOSOrchestrator",
            "RedpointEOSPresence",
            "RedpointEOSSDK",

			// Now necessary for the Cloud Messaging Hub code.
			"WebSockets",
        });

        // Prevent implicit narrowing of types on Windows. We have to use these macros around any 
        // external headers because the engine is not compliant with these requirements, and turning 
        // them on globally causes the engine headers to fail the build.
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicDefinitions.Add("EOS_ENABLE_STRICT_WARNINGS=__pragma(warning(push))__pragma(warning(error:4244))__pragma(warning(error:4838))");
            PublicDefinitions.Add("EOS_DISABLE_STRICT_WARNINGS=__pragma(warning(pop))");
        }
        else
        {
            PublicDefinitions.Add("EOS_ENABLE_STRICT_WARNINGS=");
            PublicDefinitions.Add("EOS_DISABLE_STRICT_WARNINGS=");
        }

        /* PRECOMPILED REMOVE BEGIN */
        if (!bUsePrecompiled)
        {
            if (Target.bBuildDeveloperTools || (Target.Configuration != UnrealTargetConfiguration.Shipping && Target.Configuration != UnrealTargetConfiguration.Test))
            {
                PrivateDependencyModuleNames.Add("GameplayDebugger");
                PrivateDefinitions.Add("WITH_GAMEPLAY_DEBUGGER=1");
            }
            else
            {
                PrivateDefinitions.Add("WITH_GAMEPLAY_DEBUGGER=0");
            }

            PrivateDependencyModuleNames.AddRange(new[]
            {
                "CoreUObject",
                "Engine",
                "Sockets",
                "PacketHandler",
                "NetCore",
                "HTTP",
                "RedpointEOSInterfaces",
                "RedpointLibHydrogen",
                "AESGCMHandlerComponent",
                "ImageWrapper",
                "CoreOnline",
                "WebSockets",
            });

            if (Target.Type != TargetType.Server)
            {
                PublicDefinitions.Add("EOS_HAS_AUTHENTICATION=1");
                PublicDefinitions.Add("EOS_HAS_IMAGE_DECODING=1");

                PrivateDependencyModuleNames.AddRange(new[]
                {
                    // Required for image decoding.
                    "ImageWrapper",
                    "RHI",
                    "RenderCore",
                });
            }
            else
            {
                PublicDefinitions.Add("EOS_HAS_AUTHENTICATION=0");
                PublicDefinitions.Add("EOS_HAS_IMAGE_DECODING=0");
            }

            PrivateDefinitions.Add("ONLINESUBSYSTEMEOS_PACKAGE=1");

            if (RedpointEOSBuild.GetConfigBool(Target, "BUILDING_FREE_EDITION", false))
            {
                PrivateDefinitions.Add("EOS_IS_FREE_EDITION=1");
            }

            if (!string.IsNullOrWhiteSpace(Environment.GetEnvironmentVariable("BUILD_VERSION_NAME")))
            {
                PrivateDefinitions.Add("EOS_BUILD_VERSION_NAME=\"" + Environment.GetEnvironmentVariable("BUILD_VERSION_NAME") + "\"");
            }

            // Enable trace counters and stats by default in non-shipping builds.
            if (RedpointEOSBuild.GetConfigBool(Target, "ENABLE_TRACE", Target.Configuration != UnrealTargetConfiguration.Shipping))
            {
                PrivateDefinitions.Add("EOS_ENABLE_TRACE=1");
            }

            if (RedpointEOSBuild.GetConfigBool(Target, "ENABLE_STATE_DIAGNOSTICS", false))
            {
                PrivateDefinitions.Add("EOS_ENABLE_STATE_DIAGNOSTICS=1");
            }
        }
        /* PRECOMPILED REMOVE END */
    }
}