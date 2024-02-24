// Copyright Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class SimpleMeshComponent : ModuleRules
	{
		public SimpleMeshComponent(ReadOnlyTargetRules Target) : base(Target)
		{

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
                    "Core",
                    "CoreUObject",
                    "Engine",
                    "MeshDescription",
                    "RenderCore",
                    "RHI",
                    "StaticMeshDescription",
                    "PhysicsCore"
                }
				);
		}
	}
}
