# Unreal Engine Development Setup Scripts

This repository contains Windows PowerShell setup helpers and connection notes for Unreal Engine 5.8 development.

## Repository contents

- `Setup-UE5.8.ps1` — verifies an Unreal Engine installation and configures user environment variables. It can also attempt to add optional Windows Firewall rules.
- `Setup-VS2026.ps1` — checks for Visual Studio and MSVC C++ tooling and can invoke the Visual Studio Installer to add development workloads.
- `UE5.8_Connection_Setup.md` — notes for Remote Control, Pixel Streaming, Python scripting, Live Coding, and Multi-User Editing.

## Requirements

- Windows 10/11 with PowerShell.
- Unreal Engine 5.8 installed at the path configured in the script, or update the script for your installation path.
- Visual Studio 2026 and the required C++ workload if compiling UE C++ projects.
- Administrator rights only for changes that require elevation, such as firewall changes or Visual Studio installation.

## Usage

Review each script before running it. Run from a PowerShell terminal:

```powershell
# Check Unreal Engine setup without modifying firewall rules
.\Setup-UE5.8.ps1 -SkipFirewall

# Check Visual Studio toolchain without launching an installer
.\Setup-VS2026.ps1 -SkipInstall
```

The scripts currently assume default installation paths. Adjust those paths to match the local machine before use.

## Security notes

- Do not expose Unreal Remote Control, Python execution, or other editor-control endpoints directly to the public internet.
- Restrict network access to trusted hosts and private development networks. Use appropriate authentication and firewall rules.
- Review all firewall changes before approving them. The UE setup script attempts to create inbound rules unless `-SkipFirewall` is supplied.
- Python remote execution can run editor-side code. Enable it only in trusted environments and do not accept commands from untrusted clients.
- These scripts change user environment settings; the Visual Studio helper can launch an installer and download several gigabytes when installation is needed.

## Known limitations

- The scripts have not been validated on every Windows configuration or Unreal Engine installation layout.
- `Setup-VS2026.ps1` checks for MSVC directories but does not fully validate every workload/component before deciding no installation is necessary.
- `UE5.8_Connection_Setup.md` is an operational note, not a guarantee that all plugin paths, command lines, endpoints, or ports apply to every engine build. Verify settings against the version installed and the official documentation.

## Verification

No Windows PowerShell execution, Unreal Editor launch, network connection, or build test has been run as part of this documentation update.

## Official references

- [Unreal Engine 5.8 announcement](https://www.unrealengine.com/news/unreal-engine-5-8-is-now-available)
- [Visual Studio 2026 release notes](https://learn.microsoft.com/en-us/visualstudio/releases/2026/release-notes)


## Engine source and future-version migration workspace

This repository includes a local source-access helper and documentation-first workspaces for UE5.8 and future migration targets UE6–UE10.

- [Source access and local clone instructions](ENGINE_SOURCE_ACCESS.md)
- [Clone UE5.8 source locally](Clone-UE5.8-Source.ps1)
- [Engine version workspace index](EngineVersions/README.md)
- [Migration architecture](docs/ENGINE_MIGRATION_ARCHITECTURE.md)
- [Engine version support matrix](docs/ENGINE_VERSION_MATRIX.md)
- [Migration checklist](docs/MIGRATION_CHECKLIST.md)
- [Source licensing and repository policy](docs/UNREAL_SOURCE_LICENSE_AND_REPOSITORY_POLICY.md)

**Important:** Epic's licensed engine source is cloned locally after account authorization and is not stored in this repository. UE6–UE10 folders are planning scaffolds; future-version APIs are not fabricated.
