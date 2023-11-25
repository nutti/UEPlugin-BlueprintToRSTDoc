// Some copyright should be here...

using UnrealBuildTool;

public class BlueprintToRSTDoc : ModuleRules
{
	public BlueprintToRSTDoc(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(new string[]{});
		PrivateIncludePaths.AddRange(new string[]{"BlueprintToRSTDoc/Public"});
		PublicDependencyModuleNames.AddRange(new string[]{"Core"});
		PrivateDependencyModuleNames.AddRange(new string[]{
			"Projects",
			"CoreUObject",
			"Engine",
			"Slate",
			"SlateCore",
			"BlueprintGraph",
		});
		DynamicallyLoadedModuleNames.AddRange(new string[]{});
	}
}
