# Security Policy

## Scope

This repository contains scripts and instructions that configure a Windows development environment and enable optional remote-control features for Unreal Engine.

## Important deployment guidance

- Do not expose Unreal Remote Control, Python remote execution, or editor-control endpoints to the public internet.
- Permit access only from explicitly trusted hosts and private development networks.
- Use authentication where supported and validate access controls before enabling remote endpoints.
- Avoid broad inbound firewall rules. Prefer narrowly scoped rules and remove rules when they are no longer needed.
- Treat commands executed through Python remote execution as privileged development actions.
- Review scripts before running them, especially when using elevated PowerShell or installer privileges.
- Do not commit passwords, access tokens, private keys, machine-specific secrets, or private network details.

## Reporting a concern

Please report security issues privately to the repository maintainer rather than publishing details that could enable exploitation. Include the affected file, relevant configuration, impact, and a safe reproduction where possible.
