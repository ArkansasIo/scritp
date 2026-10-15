# Engine Version Support Matrix

A folder does not imply that an engine release exists or that this project has been ported.

| Target | Engine status | Project status | Next action |
|---|---|---|---|
| UE5.8 | Official release | Baseline scaffold; build not verified in this update | Clone licensed source and validate build |
| UE6 | In development; Early Access target announced for end of 2027 as of June 2026 | Planning only | Wait for official build/source access and migrate |
| UE7 | Not confirmed here | Documentation placeholder | Reassess after official announcement |
| UE8 | Not confirmed here | Documentation placeholder | Reassess after official announcement |
| UE9 | Not confirmed here | Documentation placeholder | Reassess after official announcement |
| UE10 | Not confirmed here | Documentation placeholder | Reassess after official announcement |

Status definitions:
- Baseline: selected reference version; build status tracked separately.
- Planning: architecture and test plan only.
- In progress: migration branch exists and CI is running.
- Validated: clean build, package and tests passed on recorded hardware/toolchain.
- Unsupported: a known blocking incompatibility exists.

Do not mark a version Validated without recorded build and test evidence.
