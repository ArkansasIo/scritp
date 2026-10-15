# Obtain Unreal Engine 5.8 Source Code

Unreal Engine source is distributed by Epic under its applicable license terms; it is not an unrestricted public source archive. Link your Epic Games and GitHub accounts, accept the invitation to join the @EpicGames GitHub organization, and accept the applicable license terms first.

Official guide: https://dev.epicgames.com/documentation/unreal-engine/downloading-source-code-in-unreal-engine

## Clone locally on Windows

Run from PowerShell at the repository root:

    .\Clone-UE5.8-Source.ps1 -Branch 5.8

The default destination is LocalEngine/UnrealEngine-5.8. It is ignored by Git so licensed engine source is not accidentally committed here. If branch 5.8 is unavailable to your account, inspect official repository refs after signing in and pass the exact release branch with -Branch. Do not silently substitute a moving development branch.

## Build

From the local checkout, follow Epic's current instructions. The standard Windows workflow usually includes:

    cd LocalEngine/UnrealEngine-5.8
    .\Setup.bat
    .\GenerateProjectFiles.bat

Open the generated solution and build the appropriate Editor target using the compiler/toolchain version documented for that release. Dependency downloads and compilation can require substantial disk space and time.

## Limitations

- This repository does not contain or redistribute Epic's engine source or binaries.
- The script attempts a clone; it does not claim that cloning or building succeeded until run and verified.
- Keep engine source, generated binaries, Intermediate, Saved and DerivedDataCache out of this repository.
- Verify branch names, toolchain requirements and license terms against official documentation.
