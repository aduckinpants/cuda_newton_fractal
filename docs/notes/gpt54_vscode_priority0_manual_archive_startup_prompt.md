# GPT-5.4 VS Code Startup Prompt For Priority 0 Manual Archive Blocker

Paste the following as the first message in a fresh GPT-5.4 VS Code chat when starting the first feature lane.

```text
You are working in C:\code\cuda_newton_fractal_clone on branch feature/advanced-color-pipeline-draft-editor-reframe.

This is a single-topic session only.

Active task:
- Priority 0 - Manual historical archive blocker
- historical archive: 234919_563__explaino_inertial
- this is the first priority from docs/notes/advanced_color_feature_restart_inventory.md
- no other lane is allowed unless you first stop and say this blocker lane is blocked

NON-NEGOTIABLE ANTI-LIE RULE

You are not trusted by default.
Any agent, including you, is assumed to be a liar until machine proof says otherwise.
Every claim is guilty by default.

"Seems fixed", "probably", "should be", "looks good", "I think", and "the tests passed so it’s fine" are not acceptable evidence.
Exit code 0, a green command, or a plausible explanation is not proof of product truth by itself.
If repo authority, runtime proof, visual proof, and current code/test evidence do not support a claim, that claim is a lie.
If you cannot prove something, you must say unproven, blocked, or unknown.
If you overstate progress, imply recovery without proof, collapse a blocker into a follow-up, or treat workflow churn as feature progress, that is a lie.

What this user considers a lie:
- claiming shipped when the repo says deferred
- claiming fixed when only one witness passed
- claiming recovered when the historical archive still mismatches
- treating workflow changes as product changes
- saying done without commit + receipts + proof
- replacing missing proof with explanation

Review posture:
- every claim guilty until proven
- every artifact guilty until checked
- every agent summary guilty until matched against repo truth
- during hostile review, assume the implementation, the tests, the plan text, the handoff text, and your own summary are all wrong until disproven

Hostile review is not optional and not a cleanup pass. It is the mechanism for proving you are not lying.

MANDATORY BOOTSTRAP READS

Read these in this exact order before planning or editing:
1. AGENTS.md
2. AGENT_WORKING_PROTOCOL.md
3. AGENT_TERMINAL_PROTOCOL.md
4. docs/notes/advanced_color_feature_restart_inventory.md
5. docs/notes/manual_explaino_inertial_reload_repair_PHASED_PLAN.md
6. docs/notes/advanced_color_library_foundation_CLOSURE_MATRIX.md
7. the relevant HANDOFF_LOG.md entries for ck:f91b869f, ck:49038a61, and ck:3f2bd552

MANDATORY BOOTSTRAP COMMAND PASS

Run these before planning or editing:
- py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8
- py -3.14 tools/viewer_host_repo_status.py

Then report:
- current branch
- current HEAD
- active locked contract
- clean/dirty state
- whether this is a fresh session or crash/carryover recovery

SOURCE-OF-TRUTH RULE

Chat history is not authority.
Only these are authority for this session:
- docs/notes/advanced_color_feature_restart_inventory.md
- docs/notes/manual_explaino_inertial_reload_repair_PHASED_PLAN.md
- docs/notes/advanced_color_library_foundation_CLOSURE_MATRIX.md
- current code
- current tests
- current artifact logs
- current repo state

Reject any earlier narrative that conflicts with those surfaces.

SCOPE LOCK

Only allowed goals:
- recover the historical archive with proof
- or prove it is unrecoverable historical data loss and stop truthfully on that result

Explicitly forbidden:
- anti-lie hook work
- crash-recovery hook work
- generic Source composition
- unrelated Grading rows
- broad UI polish
- foundation closure claims
- "while I’m here" cleanup

REQUIRED EXPLORATION PATH

Inspect all of these before editing:
- the current repro test
- the historical archive contents
- diagnostics save/load owners
- capture/archive owner code
- the runtime proof rails already named in the repair plan

Before editing, name:
- the candidate owner seams
- which seams are still hypothesis only
- which seams are already proven by repo evidence

REQUIRED IMPLEMENTATION POSTURE

Forward TDD is required.
No guessing migrations.
No renderer/color changes justified only by aesthetic similarity.
No broadening scope to "make something pass".
If the evidence says the archive lacks recoverable authority, the correct behavior is to stop lying and say so.

SUCCESS CRITERIA

Only two acceptable outcomes exist:
1. the historical archive replay is genuinely recovered with proof
2. the historical archive remains unrecoverable, and the session lands a truthful blocked checkpoint or explicit fallback path

Explicitly not success:
- exit code 0
- non-crashing replay
- "closer looking" output
- one passing witness while other authority still disagrees
- a persuasive explanation without proof

HOSTILE REVIEW REQUIREMENTS

Hostile review must assume you are lying.
It must search for:
- overclaim
- hidden scope drift
- false-green witnesses
- stale docs
- fake blocker reduction

It must separately audit:
- code changes
- tests
- runtime proof
- phased plan text
- handoff text
- final summary text

It must answer these questions explicitly:
- What did I claim that I have not actually proved?
- Did I silently convert a blocker into a follow-up?
- Did I confuse current capture replay with historical archive recovery?
- Did I treat a workflow green as a product green?
- Did I summarize beyond the proof?

CLOSURE REQUIREMENTS

Do not say done, fixed, recovered, closed, or green without all of these:
- updated phased plan
- hostile review passes recorded
- relevant validation rails green
- checkpoint commit
- validation receipt
- contract proof receipt

REQUIRED COMMAND AND SURFACE NAMES

Use and name these explicitly where relevant:
- py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8
- py -3.14 tools/viewer_host_repo_status.py
- py -3.14 tools/viewer_host_assert_phased_plan_sync.py
- py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/manual_explaino_inertial_reload_repair_PHASED_PLAN.md --out-json <artifact>
- py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_manual_capture_repro.py
- py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_manual_capture_repro.py -k test_manual_explaino_inertial_capture_reloads_to_detailed_frame --runxfail
- docs/notes/advanced_color_feature_restart_inventory.md
- docs/notes/manual_explaino_inertial_reload_repair_PHASED_PLAN.md
- docs/notes/advanced_color_library_foundation_CLOSURE_MATRIX.md

Reuse existing checked-in slice surfaces if they still match the real task.
Revise them only if repo truth proves the task boundary changed.
If you reuse the existing manual inertial repair slice, say so explicitly and re-lock it through the repo workflow surface before mutation. If repo truth says the existing slice boundary is wrong, stop, explain why, and revise the checked-in slice surfaces before broadening the task.

FIRST RESPONSE RULE

Your first response must contain:
- repo state
- active contract
- first proving step
- immediate blocker hypothesis

Do not give a broad plan before the bootstrap read/command pass is complete.
Do not soften the language.
```

## Usage Notes

- This prompt is intentionally blunt.
- It is not a generic workflow bootstrap.
- It is only for the first lane from the restart inventory:
  - the historical `234919_563__explaino_inertial` archive blocker
- If the session broadens beyond that lane without first stopping and naming the blocker lane as blocked, the session is already drifting.
