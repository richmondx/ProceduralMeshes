// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ProceduralMeshes : ModuleRules
{
	public ProceduralMeshes(TargetInfo Target)
	{
		
		PublicIncludePaths.AddRange(new string[] { "ProceduralMeshes/Public" });

        PrivateIncludePaths.AddRange(new string[] { "ProceduralMeshes/Private" });

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "ProceduralMeshComponent" });


        PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
// 				"Slate",
// 				"SlateCore",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(new string[] {  });
    }
}
