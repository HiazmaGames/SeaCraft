// Copyright 2011-2014 UFNA, LLC. All Rights Reserved.

using UnrealBuildTool;

public class SeaCraft : ModuleRules
{
	public SeaCraft(TargetInfo Target)
    {
        PublicDependencyModuleNames.AddRange(
            new string[] {
                "Core",
                "CoreUObject",
                "Engine",
                "InputCore",
                "OnlineSubsystem",
                "OnlineSubsystemUtils" 
            });

        PrivateDependencyModuleNames.AddRange(new string[] { "VaOceanPlugin" });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.Add("Slate");
		
		PrivateDependencyModuleNames.Add("OnlineSubsystem");
		if ((Target.Platform == UnrealTargetPlatform.Win32) || (Target.Platform == UnrealTargetPlatform.Win64))
		{
			if (UEBuildConfiguration.bCompileSteamOSS == true)
			{
				DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");
			}
		}
	}
}
