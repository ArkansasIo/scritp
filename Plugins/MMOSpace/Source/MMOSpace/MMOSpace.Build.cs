using UnrealBuildTool;

public class MMOSpace : ModuleRules
{
	public MMOSpace(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"MMOCore",
			"OnlineSubsystem",
			"GameplayTags",
			"Json",
			"JsonUtilities",
			"HTTP"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"Slate",
			"SlateCore",
			"RenderCore",
			"RHI",
			"Networking",
			"Sockets"
		});
	}
}
