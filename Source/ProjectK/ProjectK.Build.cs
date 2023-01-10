// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ProjectK : ModuleRules
{
	public ProjectK(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"InputCore",
				"HeadMountedDisplay",
				"GameplayAbilities",
				"GameplayTags",
				"GameplayTasks",
			}
		);

	}
}
