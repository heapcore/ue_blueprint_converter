using UnrealBuildTool;
using System.Collections.Generic;

public class UEBlueprintYamlConverterTestTarget : TargetRules
{
    public UEBlueprintYamlConverterTestTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

        ExtraModuleNames.AddRange( new string[] { } );
    }
}
