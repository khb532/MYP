using UnrealBuildTool;
using System.Collections.Generic;

public class MYPTarget : TargetRules
{
    public MYPTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_3;
        ExtraModuleNames.AddRange(new string[] { "MYP" });
    }
}

