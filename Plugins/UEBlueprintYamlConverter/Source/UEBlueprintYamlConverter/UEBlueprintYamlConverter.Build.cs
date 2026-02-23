// Copyright (c) UE YAML
using UnrealBuildTool;

public class UEBlueprintYamlConverter : ModuleRules
{
    public UEBlueprintYamlConverter(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine"
        });

        // Editor-only dependencies guarded by bBuildEditor, so the module can still parse in non-editor contexts
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "UnrealEd",
                "Projects",
                "AssetRegistry",
                "Slate",
                "SlateCore",
                "AssetTools",
                "Kismet",
                "KismetCompiler",
                "BlueprintGraph",
                "EditorSubsystem",
                "LevelEditor",
                "Json",
                "JsonUtilities",
                "AutomationController",
                "AutomationTest"
            });
        }
        else
        {
            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "Projects",
                "Json",
                "JsonUtilities"
            });
        }
    }
}
