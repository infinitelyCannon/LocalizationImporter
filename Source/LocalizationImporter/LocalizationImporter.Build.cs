// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class LocalizationImporter : ModuleRules
{
	public LocalizationImporter(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateIncludePaths.AddRange(
			new string[] {
				"LocalizationImporter/Private"
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core"
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"UnrealEd",
				"ToolMenus",
				"LevelEditor",
				"EditorStyle",
				"EditorWidgets",
				"Localization",
				"LocalizationCommandletExecution",
				"SourceControl",
				"Projects",
				"DesktopPlatform",
				"ApplicationCore",
				"InputCore"
			}
			);
	}
}
