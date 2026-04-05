---
description: "Use when reviewing viewer-host progress or choosing the next bounded implementation slice. Applies TDD, deterministic tooling, phase exit criteria, and cousin-repo boundary checks."
name: "Viewer Host Next Slice"
argument-hint: "Current phase, obstacle, or desired goal"
agent: "agent"
tools: [read, search, todo]
---
Using the provided phase, obstacle, or goal, review the current repo state and produce a bounded next-slice plan.

Requirements:
- Treat this repo as the editable surface.
- Treat `salticid-cuda` and `nine` as read-only reference unless the user explicitly asks for a handoff artifact.
- Prefer extraction and reuse over reinvention.
- Call out where TDD or deterministic tooling is missing.
- Keep the slice pauseable and small enough to land cleanly.

Return:
1. Current phase and why.
2. Recommended next slice.
3. Files likely involved.
4. Tests and deterministic checks to add or run first.
5. Stop point and exit criteria.
6. Risks or protocol traps.