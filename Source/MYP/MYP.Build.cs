// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MYP : ModuleRules
{
	public MYP(ReadOnlyTargetRules Target) : base(Target)
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
			"MYP",
			"MYP/Variant_Platforming",
			"MYP/Variant_Platforming/Animation",
			"MYP/Variant_Combat",
			"MYP/Variant_Combat/AI",
			"MYP/Variant_Combat/Animation",
			"MYP/Variant_Combat/Gameplay",
			"MYP/Variant_Combat/Interfaces",
			"MYP/Variant_Combat/UI",
			"MYP/Variant_SideScrolling",
			"MYP/Variant_SideScrolling/AI",
			"MYP/Variant_SideScrolling/Gameplay",
			"MYP/Variant_SideScrolling/Interfaces",
			"MYP/Variant_SideScrolling/UI"
		});
		
		// 모듈 포함 경로 정리: Public/Private 및 모듈 루트(MYP.h 접근) 추가
		PublicIncludePaths.AddRange(new string[]
		{
			System.IO.Path.Combine(ModuleDirectory, "Public"),
		});

		PrivateIncludePaths.AddRange(new string[]
		{
			System.IO.Path.Combine(ModuleDirectory, "Private"),
			ModuleDirectory, // 모듈 루트에 있는 MYP.h 접근용
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
