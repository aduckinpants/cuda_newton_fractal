# Viewer-Host Agent Terminal Protocol

## Problem

VS Code keeps terminal sessions alive for the workspace, and large build/test transcripts can destabilize the editor long before the underlying command is the real problem.

This repo already has deterministic wrappers and build/profile surfaces for that risk. Use them on purpose instead of rediscovering the failure mode mid-slice.

## Rules

### 1. No persistent background terminals for one-shot commands

Use foreground terminal runs for finite commands.

Wrong:
```text
run_in_terminal(..., mode=async)
```

Right:
```text
run_in_terminal(..., mode=sync)
```

### 2. Background terminals are only for genuinely long-running processes

Acceptable examples:
- a broker/viewer process that must stay alive while other work happens
- a deliberate watch loop

If a background terminal is no longer needed, kill it immediately.

### 3. Keep terminal count low

- Reuse the shared shell when possible.
- Prefer one heavy command at a time.
- Treat roughly three active terminals as the practical ceiling.
- If one lane is contaminated by quoting, stale state, or interactive residue, refresh that lane instead of spawning more.

### 4. Prefer the repo-local logged-command wrapper for large output

When a command can emit a large transcript, prefer:

```text
py -3.14 tools/viewer_host_run_logged_command.py --label "<label>" --log artifacts/<name>.log -- <command ...>
```

Use this instead of relying on raw `run_in_terminal` output for long builds, validation profiles, or runtime probes.

### 5. Build and validation commands stay foreground

These are finite jobs and must not be left behind as background shells:
- `ui_app/build_tests_vsdevcmd.cmd`
- `ui_app/build_vsdevcmd.cmd`
- `verify: profile ...` rails
- focused `py -3.14 -m pytest ...` commands

### 6. Crash-safe mode is mandatory after terminal instability

If a command destabilizes VS Code, the chat session, or the terminal wrapper, switch to crash-safe mode for the rest of the session:
- no parallel terminal work
- prefer checked-in tasks or logged-command wrappers
- redirect large output to `artifacts/` and inspect the file instead of streaming it
- do not retry the same destabilizing command path more than once without changing approach

## Recovery Checklist

1. Kill terminals that are no longer needed.
2. Confirm the repo state with `py -3.14 tools/viewer_host_repo_status.py`.
3. Resume with one bounded command at a time.
4. Prefer the logged-command wrapper or task surface before retrying a heavy validation lane.