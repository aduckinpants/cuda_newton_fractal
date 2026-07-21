# Diagnostics Panel And Reset Color Authority Docs Phased Plan

## Explicit User Asks

- PR and merge the current `explaino_multibrot_root_trap` branch upstream before this docs pass.
- Produce a source-grounded Engineering Diagnostics Panel research/specification writeup from the attached outline.
- Produce a source-grounded Reset All / Color Pipeline state-authority bug report from the attached outline.
- Do not implement the Engineering Diagnostics Panel.
- Do not fix the Reset All / Color Pipeline defect.
- Keep this as documentation and git maintenance only.

## Current Phase

Complete - Phase 6 hostile audit, receipts, rearward review, push, and clean-tree closure are proven.

## Phase Checklist

- [x] PR and merge `codex/explaino-multibrot-root-trap` to `master`.
- [x] Fast-forward local `master` to the merged upstream head.
- [x] Phase 1: create this docs-only plan/contract and lock the active slice.
- [x] Phase 2: research current repo capabilities for the Engineering Diagnostics Panel specification.
- [x] Phase 3: write the Engineering Diagnostics Panel research/specification report.
- [x] Phase 4: research the Reset All / Color Pipeline authority mismatch.
- [x] Phase 5: write the Reset All / Color Pipeline bug report.
- [x] Phase 6: hostile audit, validation receipts, contract proof receipt, rearward review, push, clean tree, and stop.

## Scope

In scope:

- Documentation under `docs/notes/`.
- Contract and plan surfaces for this docs-only pass.
- Read-only source inspection and lightweight validation commands.
- Handoff log and proof artifacts.

Out of scope:

- Production code changes.
- UI changes.
- Runtime probe implementation.
- Engineering panel implementation.
- Reset All bug repair.
- New tests for either future work item.
- Any change to Salticid mainline.

## Proof Ledger

| Item | Evidence |
| --- | --- |
| Multibrot branch PR | `https://github.com/aduckinpants/cuda_newton_fractal/pull/1` merged. |
| Upstream merge | `origin/master` advanced to `19b466c01f95026e810f42173af7372a1077d457`. |
| Local branch | `codex/diagnostics-panel-reset-authority-docs` from merged `master`. |
| Slice lock | `viewer_host_begin_work_slice.py` opened `ck:47331aa6`. |
| Engineering Diagnostics spec | `docs/notes/engineering_diagnostics_panel_RESEARCH_SPEC.md`. |
| Reset All bug report | `docs/notes/reset_all_color_pipeline_authority_bug_REPORT.md`. |
| Validation receipt | `artifacts/hooks/viewer_host_validation_receipts/c770cef7ab1a05fcb1c4b74482042cb98c0c671c.json`. |
| Contract proof receipt | `artifacts/hooks/viewer_host_contract_proof_receipts/c770cef7ab1a05fcb1c4b74482042cb98c0c671c.json`. |
| Rearward review | `artifacts/hooks/viewer_host_rearward_review/c770cef7ab1a05fcb1c4b74482042cb98c0c671c.json` reports `ok`. |

## Hostile Audit

- Status: complete

Audit questions:

- Did this pass remain documentation-only?
- Did the Engineering Diagnostics writeup distinguish Engineering Diagnostics from Science Mode, ordinary viewport UI, and agent tooling?
- Did the bug report distinguish configured/draft state from effective render authority?
- Did the bug report avoid claiming a renderer math defect without source evidence?
- Did this pass avoid implementing either future feature or repair?

## Audit Passes

- [x] Pass 1 - found the first docs-only contract draft did not match repo contract schema and repaired it before locking the slice.
- [x] Pass 2 - found a source-confirmed bug-report finding: Color Pipeline graph receipt currently writes `active_execution` from draft row enabled state, not effective renderer contribution.
- [x] Pass 3 - reread the docs and found no implementation instructions, production-code edits, or claims that rendered pixels are wrong.
- [x] Pass 4 - clean re-read after the docs and contract repairs found no additional real defect, scope leak, stale implementation ask, or unsupported closure claim.

## Audit Findings

- [x] Finding 1: the initial docs-only contract used unsupported `workflow_type=docs_only`, unsupported `command_exit` evidence, and missing allowed-scope files. Repaired by using `workflow_only`, validator-backed assertions, and a valid docs scope before locking the slice.
- [x] Finding 2: the Reset All research confirmed a concrete provenance risk in `ui_app/src/color_pipeline_graph_receipt.h`: `active_execution` is currently emitted as `row.enabled`, which can be true for retained draft rows that did not contribute to a classic basin rendered frame. Captured this as the core future RED target in the bug report.

## Stop Point

Stop after both requested writeups are committed, receipted, rearward-reviewed, pushed, and the worktree is clean. Preplanned sliced work is exhausted at that point; stop for replan before implementing the Engineering Diagnostics Panel or repairing the Reset All / Color Pipeline defect.
