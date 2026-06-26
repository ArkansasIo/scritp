using UnrealBuildTool;
using System.Collections.Generic;

public class MMOEngineTarget : TargetRules
{
	public MMOEngineTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("MMOEngine");
	}
}
