using UnrealBuildTool;

public class MMOUI : ModuleRules
{
	public MMOUI(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"UMG",
			"GameplayTags",
			"MMOCore",
			"MMOCharacter",
			"MMOInventory",
			"MMOCombat",
			"MMOQuest",
			"MMOParty",
			"MMOChat"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore"
		});
	}
}
