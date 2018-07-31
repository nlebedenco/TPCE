// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class TPCEEditor: ModuleRules
{
	public TPCEEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bEnforceIWYU = true;
        
		PrivateIncludePaths.AddRange(
			new string[] 
			{
				"TPCEEditor/Private"
			});

		PublicDependencyModuleNames.AddRange(
			new string[] 
			{
				"Core",
				"CoreUObject",
				"InputCore",
				"Engine",
				"UnrealEd"
			});

		PrivateDependencyModuleNames.AddRange(
			new string[] 
			{
				"Slate",
				"SlateCore",
				"EditorStyle",
				"PropertyEditor",
				"AnimationModifiers",
                "AnimGraph",
                "BlueprintGraph",
                "TPCE"
			});
	}
}
