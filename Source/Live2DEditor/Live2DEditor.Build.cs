using UnrealBuildTool;

public class Live2DEditor : ModuleRules
{
    public Live2DEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore", 
                "Live2D",
                "UnrealEd",
                "Json",
                "JsonUtilities"
            }
        );
    }
}