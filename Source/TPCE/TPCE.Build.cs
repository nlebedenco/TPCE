// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class TPCE: ModuleRules
{
	public TPCE(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bEnforceIWYU = true;

		PrivateIncludePaths.AddRange(
			new string[] 
			{
                "TPCE/Private"
            }
		);

		PublicDependencyModuleNames.AddRange(
			new string[] 
			{
				"Core",
				"CoreUObject",
				"Engine",
				"AnimationCore",
				"AnimGraphRuntime",
				"InputCore"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[] 
			{
				// UI
				"Slate",
				"SlateCore",
				"UMG"
            }
		);
	}
}
