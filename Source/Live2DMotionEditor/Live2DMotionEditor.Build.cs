using UnrealBuildTool;

public class Live2DMotionEditor : ModuleRules
{
    public Live2DMotionEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "Core",
                "CoreUObject",
                "Json",
                "Slate",
                "SlateCore",
                "Engine",
                "InputCore",
                "UnrealEd", // for FAssetEditorManager
                "KismetWidgets",
                "Kismet",  // for FWorkflowCentricApplication
                "PropertyEditor",
                "RenderCore",
                "ContentBrowser",
                "WorkspaceMenuStructure",
                "EditorStyle",
                "EditorWidgets",
                "Projects",
                "AssetRegistry",
                "Live2D"
            }
        );
    }
}