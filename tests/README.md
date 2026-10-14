# Setup Script Validation

These checks are designed to be run on Windows PowerShell 5.1 or PowerShell 7. Windows and Unreal Engine are not available in the repository connector environment, so the tests below are a plan until executed on a Windows runner.

## Manual smoke tests

1. Run `./Setup-UE5.8.ps1 -SkipFirewall` on a machine with UE 5.8 installed at the configured path.
2. Run the same command on a machine without UE 5.8. It must exit non-zero and identify the missing executable.
3. Run `./Setup-VS2026.ps1 -SkipInstall` on machines with and without MSVC installed.
4. Validate that every script reports a meaningful exit code and does not claim setup succeeded after a failed operation.
5. Review firewall rules created by any firewall-enabled run and verify that their remote-address scope is no broader than intended.
6. Verify Remote Control, Pixel Streaming, Multi-User Editing, and Python execution independently with plugins enabled only for the relevant test.

## Suggested automated tests

- Pester tests for path discovery, absent installations, environment variable composition, and error paths.
- Mock Windows registry, firewall cmdlets, and installer process invocation so automated tests do not mutate a developer workstation.
- CI linting with PSScriptAnalyzer and Markdown link validation.
- A Windows integration runner for non-destructive smoke tests; do not run privileged installation or firewall tests in ordinary CI.

## Acceptance criteria

- No firewall rule is added unless the operator explicitly requests it.
- No installer is launched in a check-only mode.
- A failed required check returns a non-zero process exit code.
- Existing PATH entries are preserved and duplicate entries are avoided.
- Logs explain the next recovery step without printing secrets.
