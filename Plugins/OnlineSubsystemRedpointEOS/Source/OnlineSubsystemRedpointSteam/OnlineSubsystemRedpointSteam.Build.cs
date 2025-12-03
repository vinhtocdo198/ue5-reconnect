// Copyright June Rhodes. All Rights Reserved.

using System;
using UnrealBuildTool;

public class OnlineSubsystemRedpointSteam : RedpointEOSCPlusPlusModuleRules
{
    public OnlineSubsystemRedpointSteam(ReadOnlyTargetRules Target) : base("OSRS", Target)
    {
        /* PRECOMPILED REMOVE BEGIN */
        if (!bUsePrecompiled)
        {
            PrivateDependencyModuleNames.AddRange(new[]
            {
                "Core",
                "CoreUObject",
                "Projects",
                "Json",
                "HTTP",
                "Engine",
                "OnlineSubsystem",
                "OnlineSubsystemUtils",
                "RedpointEOSInterfaces",
                "RedpointEOSCompat",

                // Pull in WeakPtrHelpers.
                "OnlineSubsystemRedpointEOS",
            });

            AddSteamLibraries(Target, this);
        }
        /* PRECOMPILED REMOVE END */
    }

    /* PRECOMPILED REMOVE BEGIN */
    public static void AddSteamLibraries(ReadOnlyTargetRules Target, ModuleRules Module)
    {
        var bSteamEnabled = false;
        if (Target.Platform == UnrealTargetPlatform.Win64 ||
            Target.Platform == UnrealTargetPlatform.Mac ||
            Target.Platform == UnrealTargetPlatform.Linux)
        {
            var bEnableByConfig = RedpointEOSBuild.GetConfigBool(Target, "ENABLE_STEAM", false, out var bExplicitlySetByConfig);
#if UE_5_1_OR_LATER
			if ((Target.bIsEngineInstalled && Target.Platform != UnrealTargetPlatform.Mac) || bEnableByConfig)
#else
            if (Target.bIsEngineInstalled || bEnableByConfig)
#endif
            {
                bSteamEnabled = true;
            }
            else
            {
                // NOTE: By default, we don't include Steam if the developer isn't using an
                // installed version from the Epic Games launcher. That is because as
                // far as I can tell, source builds don't have OnlineSubsystemSteam
                // available by default, which will make the plugin fail to load when launching
                // the editor.
                //
                // As of Unreal Engine 5.1, we also don't enable Steam by default on macOS
                // as installed engine builds are missing the required Steamworks libraries
                // to compile Steam support in the engine.
                //
                // If you want to enable support, add the following to your .Target.cs file:
                //
                // ProjectDefinitions.Add("ONLINE_SUBSYSTEM_EOS_ENABLE_STEAM=1");
                //
                if (!bExplicitlySetByConfig)
                {
#if UE_5_1_OR_LATER
					Console.WriteLine("WARNING: Steam authentication for EOS will not be enabled by default, because you are using a source version of the engine or targeting macOS on Unreal Engine 5.1 or later. Read the instructions in OnlineSubsystemRedpointSteam.Build.cs on how to enable support or hide this warning.");
#else
                    Console.WriteLine("WARNING: Steam authentication for EOS will not be enabled by default, because you are using a source version of the engine. Read the instructions in OnlineSubsystemRedpointSteam.Build.cs on how to enable support or hide this warning.");
#endif
                }
                bSteamEnabled = false;
            }
        }
        else
        {
            bSteamEnabled = false;
        }

        if (bSteamEnabled)
        {
            Module.PrivateDefinitions.Add("REDPOINT_EOS_STEAM_ENABLED=1");

            Module.PrivateDependencyModuleNames.AddRange(new[]
            {
                // For encrypted app ticket authentication.
                "OnlineSubsystem",
                "OnlineSubsystemUtils",
                "OnlineSubsystemSteam",
                "SteamShared",
            });

            Module.AddEngineThirdPartyPrivateStaticDependencies(Target, "Steamworks");
        }

        if (bSteamEnabled)
        {
            // Legacy define, retained for compatibility with any other modules calling AddSteamLibraries.
            Module.PrivateDefinitions.Add("EOS_STEAM_ENABLED=1");
        }
        else
        {
            // Legacy define, retained for compatibility with any other modules calling AddSteamLibraries.
            Module.PrivateDefinitions.Add("EOS_STEAM_ENABLED=0");
        }
    }
    /* PRECOMPILED REMOVE END */
}