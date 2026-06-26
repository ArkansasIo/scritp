using UnrealBuildTool;
using System.Collections.Generic;

public class MMOEngineEditorTarget : TargetRules
{
	public MMOEngineEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("MMOEngine");
	}
}
