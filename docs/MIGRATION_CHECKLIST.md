# Unreal Engine Migration Checklist

Copy this checklist into a version-specific migration issue or pull request.

## Preparation
- [ ] Confirm target engine is an official build and license terms permit use.
- [ ] Record source branch/commit, build ID, compiler/toolchain and SDKs.
- [ ] Create a migration branch and preserve the last validated branch.
- [ ] Back up project assets and save data; verify restore.
- [ ] Inventory plugins, C++ modules, Blueprints, shaders, build scripts and platforms.
- [ ] Review official release notes and breaking-change documentation.

## Compile and content
- [ ] Build editor target from a clean workspace.
- [ ] Build client and dedicated-server targets.
- [ ] Compile all project plugins against the target SDK.
- [ ] Migrate/resave assets only after backup and review.
- [ ] Run asset validation and redirector checks.
- [ ] Cook and package every supported platform.
- [ ] Confirm no undocumented/private engine APIs were introduced.

## Runtime and services
- [ ] Test dedicated-server startup and shutdown.
- [ ] Test login, matchmaking, replication and region handoff.
- [ ] Test save/load and database schema compatibility.
- [ ] Test world streaming, NPC simulation and server authority.
- [ ] Test reconnect, crash recovery and rolling deployment.
- [ ] Verify telemetry, logs, security and access controls.

## Performance and sign-off
- [ ] Run fixed CPU, GPU, RAM, storage and network benchmarks.
- [ ] Record median and p95/p99 frame, tick and request latency where applicable.
- [ ] Record memory footprint, crashes, throughput and power.
- [ ] Compare with the last validated baseline.
- [ ] Document regressions, workarounds and rollback procedure.
- [ ] Obtain code review and attach test evidence before marking support.
