using System;
using UnrealBuildTool;

public class Unbuilt : ModuleRules
{
	public Unbuilt(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core", 
			"CoreUObject",
			"Engine", 
			"InputCore", 
			"EnhancedInput",
			"Chaos",
			"ChaosSolverEngine",
			"GeometryCollectionEngine",
			"FieldSystemEngine",
			"UMG",
			"Niagara",
			"NavigationSystem",
			"AIModule",
        });

		PrivateDependencyModuleNames.AddRange(new string[] {
			"Slate",
			"SlateCore"
		} );
	}
}
