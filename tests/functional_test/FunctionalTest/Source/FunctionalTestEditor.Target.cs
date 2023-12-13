/*!
 * FunctionalTest
 *
 * Copyright (c) 2020-2023 nutti
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

using UnrealBuildTool;
using System.Collections.Generic;

public class FunctionalTestEditorTarget : TargetRules
{
	public FunctionalTestEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V4;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_3;
		ExtraModuleNames.Add("FunctionalTest");
	}
}
