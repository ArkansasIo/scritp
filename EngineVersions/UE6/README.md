# Unreal Engine 6 — Migration Workspace

Status: future migration target; verify official release and SDK availability before implementation.

Epic announced UE6 is in development and targeted Early Access for the end of 2027 as of June 2026. This is a target, not a guaranteed final release date. See https://www.unrealengine.com/news/state-of-unreal-2026-top-news.

Planned work:
1. Create an isolated migration branch after an official build/source branch is available to this account.
2. Compile project-owned plugins against the actual SDK; do not guess API names.
3. Inventory C++, Blueprint, build, rendering, networking and asset changes.
4. Evaluate new workflows only where documented and relevant.
5. Compare cooked assets, dedicated-server behavior, performance and save compatibility against UE5.8.

Exit criteria: reproducible build, passing automated tests, documented migration issues and benchmark comparison. This is a planning scaffold, not UE6 source code.
