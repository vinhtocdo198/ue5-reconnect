// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class LobbyTest : ModuleRules
{
	public LobbyTest(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"LobbyTest",
			"LobbyTest/Variant_Platforming",
			"LobbyTest/Variant_Platforming/Animation",
			"LobbyTest/Variant_Combat",
			"LobbyTest/Variant_Combat/AI",
			"LobbyTest/Variant_Combat/Animation",
			"LobbyTest/Variant_Combat/Gameplay",
			"LobbyTest/Variant_Combat/Interfaces",
			"LobbyTest/Variant_Combat/UI",
			"LobbyTest/Variant_SideScrolling",
			"LobbyTest/Variant_SideScrolling/AI",
			"LobbyTest/Variant_SideScrolling/Gameplay",
			"LobbyTest/Variant_SideScrolling/Interfaces",
			"LobbyTest/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
