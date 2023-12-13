/*!
 * BlueprintToRSTDoc
 *
 * Copyright (c) 2020-2023 nutti
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

using UnrealBuildTool;

public class BlueprintToRSTDoc : ModuleRules
{
	public BlueprintToRSTDoc(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(new string[]{});
		PrivateIncludePaths.AddRange(new string[]{"BlueprintToRSTDoc/Public"});
		PublicDependencyModuleNames.AddRange(new string[]{"Core"});
		PrivateDependencyModuleNames.AddRange(new string[]{"Projects", "CoreUObject", "Engine", "Slate", "SlateCore",
			"BlueprintGraph", "ToolMenus", "Kismet", "DesktopPlatform", "InputCore"});
		DynamicallyLoadedModuleNames.AddRange(new string[]{});
	}
}
