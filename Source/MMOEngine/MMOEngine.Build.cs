using UnrealBuildTool;

public class MMOEngine : ModuleRules
{
    public MMOEngine(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "UMG",
            "GameplayTags",
            "Networking",
            "OnlineSubsystem",
            "OnlineSubsystemUtils",
            "AIModule",
            "NavigationSystem",
            "Niagara",
            "PhysicsCore",
            "DeveloperSettings"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Slate",
            "SlateCore",
            "RenderCore",
            "RHI"
        });
    }
}
