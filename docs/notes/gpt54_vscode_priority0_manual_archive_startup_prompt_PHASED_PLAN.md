# GPT-5.4 VS Code Priority 0 Startup Prompt

## Current Phase

Phase 2 complete - the paste-ready startup prompt is landed, then repaired to close the bootstrap-exit loophole that let a fresh GPT-5.4 session review the prompt itself instead of executing the blocker lane; the tightened prompt is ready for docs-slice validation and clean checkpoint closure.

## Phase Checklist

- [x] Phase 1 - create a docs-only slice for the GPT-5.4 VS Code startup prompt and restate the user ask as a checked-in deliverable
- [x] Phase 2 - write the final paste-ready prompt with explicit anti-lie rules, exact bootstrap reads/commands, strict scope lock, hostile-review requirements, and closure chain; then tighten it so bootstrap cannot devolve into prompt-review or plan-only drift

## Explicit User Asks

- [done] Implement the startup-prompt plan instead of leaving it in chat.
- [done] Make the anti-lie rule explicit: any agent is assumed to be a liar until machine proof says otherwise.
- [done] Make hostile review explicitly assume the agent, its claims, its tests, and its summaries are guilty by default.
- [done] Target only the first lane from the restart inventory: the manual historical archive blocker.
- [done] Make the result one paste-ready GPT-5.4 VS Code startup prompt with exact repo rules, exact commands, exact forbidden scope expansion, and exact closure requirements.
- [done] Repair the prompt flaw that let GPT-5.4 spend the next turn reviewing the startup prompt instead of executing the blocker lane.

## Presumption Loop

The controlling risk is not missing wording; it is accidental softening and bootstrap drift. A generic "be careful" startup prompt would fail the user ask because the entire point is to hard-code the distrust posture that future VS Code sessions must inherit. The startup prompt is only good if it is explicit about what counts as a lie, explicit that all agents are distrusted by default, explicit that the only active lane is the historical manual archive blocker, explicit that hostile review must audit both implementation claims and summary claims for overstatement, and explicit that bootstrap may not turn into prompt-review, plan-only, or memory-note drift. Any softened language, multi-lane ambiguity, workflow/product-state mixing, or bootstrap-exit ambiguity would make the startup prompt unsafe as a bootstrap surface.

The cheapest disconfirming path is to write the prompt against the checked-in restart inventory and repo rules, then reread it as if a future GPT-5.4 session were trying to evade scope, overstate proof, or substitute explanation for evidence. If the prompt leaves any path for vague success criteria, chat-memory authority, or a broadened "while I'm here" session, it is not done.

## Presumption Evidence

- `docs/notes/advanced_color_feature_restart_inventory.md` already fixes the first active lane as `Priority 0 - Manual historical archive blocker`.
- `AGENTS.md` and `AGENT_WORKING_PROTOCOL.md` already define the mandatory bootstrap reads, contract workflow, validation chain, and closure requirements that the startup prompt must name directly.
- The user's additional requirement is not discoverable from repo docs alone: the startup prompt must explicitly call lying what it is, must say any agent is assumed to be a liar until proven otherwise, must keep hostile review in that frame, and must explicitly forbid the post-bootstrap drift pattern where the agent reviews the prompt instead of doing the blocker work.

## Proof Ledger

- Authority reread: `AGENTS.md`, `AGENT_WORKING_PROTOCOL.md`, `docs/notes/advanced_color_feature_restart_inventory.md`, `docs/notes/manual_explaino_inertial_reload_repair_PHASED_PLAN.md`, and `docs/notes/advanced_color_library_foundation_CLOSURE_MATRIX.md` were re-read before drafting the prompt.
- Deliverable target: `docs/notes/gpt54_vscode_priority0_manual_archive_startup_prompt.md` is the checked-in paste-ready startup prompt for future GPT-5.4 VS Code sessions.
- Prompt hardening: the final prompt now names the exact runtime proof commands from the manual inertial repair slice instead of referring to that rail abstractly, tells the future session exactly when to reuse versus revise the existing slice surfaces, and adds an explicit bootstrap-exit rule plus second-response rule so the agent cannot truthfully spend the next turn reviewing the startup prompt itself.

## Hostile Audit

- Status: done
- Required posture: assume the first prompt draft is still lying by omission until it proves the anti-lie standard is blunt enough, the first-task scope is locked tightly enough, and the repo-authority/bootstrap/closure chain is concrete enough to stop scope drift or false-green summaries.

## Audit Passes

- [done] Pass 1 - re-read the startup-prompt requirements against the restart inventory and repo workflow rules to pin the exact first lane, bootstrap reads, command surfaces, and closure chain.
- [done] Pass 2 - reread the finished prompt as if a future GPT-5.4 session were trying to soften the anti-lie rule, broaden scope, or replace proof with narrative; repaired the remaining ambiguity by naming the exact runtime proof commands and the exact reuse-versus-revise slice rule.
- [done] Pass 3 - hostile-read the prompt against the observed failure mode where GPT-5.4 uses bootstrap as permission to review the prompt or write a plan instead of doing blocker work; repaired that loophole with an explicit bootstrap-exit rule, forbidden post-bootstrap behaviors, and a second-response execution rule.
- [done] Pass 4 - clean reread the tightened prompt as a future-session bootstrap surface; no additional real defect found after checking anti-lie bluntness, single-lane scope lock, exact bootstrap commands, exact runtime witnesses, bootstrap-exit constraints, and closure-chain wording.

## Audit Findings

- [done] Real deliverable defect found before drafting: a normal startup prompt would have softened the core requirement into generic caution language, but the user explicitly requires the prompt to define lying, define any agent as distrusted by default, and treat every claim as guilty until proven.
- [done] Real prompt defect found and repaired during hostile reread: the first draft referred to the manual-archive runtime proof rail abstractly, which left too much room for a future session to swap in the wrong witness. The repaired prompt now names the exact runtime commands and the exact rule for reusing versus revising the existing manual-repair slice surfaces.
- [done] Real prompt defect found from live use and repaired: the prompt let the next session spend its second turn reviewing the startup prompt, writing a plan, and saving memory notes because it did not explicitly define that behavior as noncompliance. The repaired prompt now makes bootstrap a precondition rather than the task, forbids prompt-review drift, and requires the second response to either execute the blocker lane or name one specific blocker.
- [done] Clean reread result: no additional ambiguity or scope leak remained after the final pass; the repaired prompt keeps the anti-lie rule blunt, treats every claim as guilty by default, blocks prompt-review drift after bootstrap, and keeps the session hard-locked to the historical manual archive blocker unless it explicitly stops as blocked.

## Notes

- Expected owner files for this docs-only slice:
  - `docs/contracts/gpt54_vscode_priority0_manual_archive_startup_prompt.contract.json`
  - `docs/notes/gpt54_vscode_priority0_manual_archive_startup_prompt_PHASED_PLAN.md`
  - `docs/notes/gpt54_vscode_priority0_manual_archive_startup_prompt.md`
  - `HANDOFF_LOG.md`
- Non-goals for this slice:
  - do not change runtime code
  - do not reopen the blocker itself
  - do not broaden the startup prompt into a generic multi-lane workflow template

## Resume Point

Validate the tightened startup prompt note, checkpoint it cleanly, and use that note directly for future GPT-5.4 VS Code sessions on the first blocker lane.
