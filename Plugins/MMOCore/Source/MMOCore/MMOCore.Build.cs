using UnrealBuildTool;

public class MMOCore : ModuleRules
{
	public MMOCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"GameplayTags",
			"OnlineSubsystem",
			"OnlineSubsystemUtils",
			"Networking",
			"DeveloperSettings"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore"
		});

		PublicIncludePathModuleNames.AddRange(new string[] { });

		PrivateIncludePathModuleNames.AddRange(new string[] { });

		DynamicallyLoadedModuleNames.AddRange(new string[] { });

		if (Target.Type == TargetType.Editor)
		{
			PrivateDependencyModuleNames.AddRange(new string[]
			{
				"UnrealEd",
				"BlueprintGraph",
				"KismetCompiler"
			});
		}
	}
}
