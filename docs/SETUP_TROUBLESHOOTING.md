# Setup Troubleshooting

## Unreal Engine path not found

The setup script currently expects:

`C:\\Program Files\\Epic Games\\UE_5.8`

If Engine is installed elsewhere, edit `$ueRoot` in `Setup-UE5.8.ps1` before running it. Verify that `Engine\\Binaries\\Win64\\UnrealEditor.exe` exists.

## Missing engine folders

The script reports missing engine directories as warnings. Confirm that the installation is complete through Epic Games Launcher or the source-build procedure appropriate to your installation. Do not create empty folders to silence a warning.

## Firewall changes

Use `-SkipFirewall` to avoid firewall changes while troubleshooting. If firewall rules are needed, run an elevated PowerShell session and confirm that each port is required by the enabled plugin and your actual network topology. Limit remote-control ports to trusted source addresses where possible.

## Visual Studio or MSVC not found

1. Run `Setup-VS2026.ps1 -SkipInstall` to perform the script's checks without launching an installer.
2. Open Visual Studio Installer and verify that the intended Visual Studio instance is selected.
3. Install the appropriate C++ workload and MSVC toolchain components for Unreal Engine development.
4. Re-run the verification script after installation.

The current script checks whether an MSVC tools directory exists; that check alone does not prove that every required SDK and workload is installed.

## Remote Control connection fails

- Confirm Remote Control is enabled in the project and the editor has been restarted where required.
- Confirm the configured port and server process match the connection instructions.
- Use `Test-NetConnection <host> -Port <port>` for TCP connectivity checks.
- Check Windows Firewall and network segmentation.
- Do not publish editor-control endpoints directly to the internet. Prefer a trusted private network or VPN and authentication supported by the service.

## Pixel Streaming does not load

Pixel Streaming setup differs across engine versions and deployments. Verify the bundled signalling/web server layout for the installed engine version and follow that version's official documentation. Check signalling logs, browser console messages, WebRTC connectivity, and HTTPS requirements for the deployment.

## Python remote execution does not connect

Verify the correct Unreal Python/editor scripting plugin settings and the supported remote-execution mechanism for the installed version. The HTTP example in the connection guide should not be assumed to represent the built-in Python remote execution protocol. Never expose Python execution to untrusted networks.

## Report useful diagnostics

When reporting a failure, include Windows and Unreal Engine versions, the exact command, the full error message, whether PowerShell was elevated, and any relevant sanitized logs. Remove usernames, internal IPs, tokens, and other sensitive data before sharing.
