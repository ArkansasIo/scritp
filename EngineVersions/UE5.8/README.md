# Unreal Engine 5.8 — Baseline

Status: official UE5 release baseline. Build status is not verified by this documentation update.

Follow ENGINE_SOURCE_ACCESS.md and run Clone-UE5.8-Source.ps1. The engine checkout remains in the ignored LocalEngine directory.

## Integration rules
- Keep gameplay and MMO domain logic in project-owned C++ modules/plugins.
- Isolate engine-dependent calls behind narrow adapter interfaces.
- Version configuration, save schemas and asset migration scripts.
- Record engine branch/commit, compiler toolchain, target platforms, plugins and build settings for each validated build.
- Test editor, client, dedicated server and commandlet targets separately.

## Acceptance gates
- Source setup and editor build complete on a clean workstation.
- Project and plugins compile for editor and dedicated-server targets.
- Content cooks and packages for supported platforms.
- Gameplay, persistence, networking and performance tests pass.

References:
- https://www.unrealengine.com/news/unreal-engine-5-8-is-now-available
- https://dev.epicgames.com/documentation/unreal-engine/unreal-engine-5-8-release-notes
- https://dev.epicgames.com/documentation/unreal-engine/downloading-source-code-in-unreal-engine
