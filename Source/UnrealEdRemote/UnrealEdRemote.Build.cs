// Copyright (c) 2014 Moritz Wundke

namespace UnrealBuildTool.Rules
{
	public class UnrealEdRemote : ModuleRules
	{
		public UnrealEdRemote(TargetInfo Target)
		{
			PublicIncludePaths.AddRange(
				new string[] {
					// ... add public include paths required here ...
				}
				);

			PrivateIncludePaths.AddRange(
				new string[] {
					"UnrealEdRemote/Private"
					// ... add other private include paths required here ...
				}
				);

            PublicIncludePathModuleNames.AddRange(
                new string[]
				{
					"Settings"
				});

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
                    "CoreUObject",
                    "Engine",
					// ... add other public dependencies that you statically link with here ...
				}
				);

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
                    "Networking",
                    "Sockets",
					// ... add private dependencies that you statically link with here ...
				}
				);

			DynamicallyLoadedModuleNames.AddRange(
				new string[]
				{
					// ... add any modules that your module loads dynamically here ...
				}
				);
		}
	}
}
