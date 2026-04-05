# Viewer Host Workflow TODO

## Current Read (updated 2026-04-04)
- The clone already contains schema-driven binding, safe-mode schema loading, several advanced fractal modes, and high-precision view navigation.
- The clean headless viewer-model layer already exists in mainline and should be copied, not reinvented.
- The local spec intake set consistently enforces schema-first explicit surfaces and no implicit fallback.
- Mainline fractal viewer is broken due to CUDA arch mismatch (build targets sm_120/121, RTX 3090 needs sm_86). One-line fix in `build_vsdevcmd.cmd`.
- The slime relaxation engine is a deterministic energy-minimization rewriter — it's the structural analysis tool for imperative C++ extraction planning.
- Verdict: repair mainline viewer + use slime for data-driven extraction, then bootstrap `salticid-viewer` from analyzed boundaries.

## Sequencing (revised)

### Block 0: Mainline slime pre-work (do this first, in mainline)

See `spec_intake/SlimeCppStructuralAnalysis_MainlinePrework_SpecIntake.md` for the full spec.

1. **Phase A** — Harden existing slime framework (unit tests for busy_beaver, projection, relax edge cases, oracle trajectories, individual logic rules)
2. **Phase B** — C++ structural encoding (lightweight function-level parser for main.cpp, field encoding, decoder)
3. **Phase C** — Structural energy terms (module cohesion, interface width, size balance)
4. **Phase D** — Structural rewrite rules (reassign function, split module, merge fragment)
5. **Phase E** — Probe run on main.cpp -> extraction report artifact

### Block 1: Return here with extraction report

6. Review `main_cpp_extraction_report.md` for concrete module boundaries
7. Fix mainline CUDA arch (sm_86) and verify fractal viewer runs
8. Create `salticid-viewer` repo (or rename this clone) with:
   - Continuity surfaces: `HANDOFF_LOG.md`, `tools/handoff_append.py`, agent-to-agent protocol
   - Agent customization from this repo (`.github/` instructions, prompts)
   - Pre-loaded structural context from slime probe results
9. Copy mainline headless viewer-model files + tests as red-green baseline
10. Verify minimal headless test harness and build script

### Block 2: Extraction work (data-driven by slime results)

11. Extract modules following slime-proposed boundaries (viewer_input, viewport_model, schema_binding, hud, colormap, fractal_dispatch)
12. Wire model-driven viewport through `IdeWindowModel` and `ViewportState`
13. Only after model seam is stable: HUD, keyboard shortcuts, colormap cycling

## Agent Onboarding Plan (for Block 1)

When returning to bootstrap salticid-viewer, the new agent session starts with:
- This repo's `.github/copilot-instructions.md` (workspace rules)
- This repo's `.github/instructions/viewer-host-code.instructions.md` (code-scoped rules)
- This repo's `.github/prompts/viewer-host-next-slice.prompt.md` (phase planning ritual)
- The slime extraction report (concrete module boundaries + metrics)
- The spec intake docs (fractal types, binding surface, no-fallback directive)
- A session startup prompt that gives full context without requiring chat archaeology

Goal: the next agent can start extracting code in its first turn, not spend 30 minutes rediscovering state.

## Repo Customization Roadmap
- Already added: workspace instructions in `.github/copilot-instructions.md`.
- Already added: a code-scoped instruction in `.github/instructions/viewer-host-code.instructions.md`.
- Already added: a reusable slash prompt for phase planning in `.github/prompts/viewer-host-next-slice.prompt.md`.
- Add at Block 1: a session startup prompt pre-loaded with slime extraction results and key decisions.
- Add at Block 1: a repo-local read-only archaeology agent with tools limited to read, search, and todo.
- Add at Block 2: hooks for handoff append and verification tasks once deterministic commands exist.

## Hat-Rack Mapping
- Workspace instructions = always-on house rules.
- File instructions = local hats for a specific file type or concern.
- Prompts = reusable rituals.
- Custom agents = bounded specialist roles.
- Hooks = deterministic enforcement rails.

## Open Decisions (updated)
- ~~Should the first landing zone be this clone or a new salticid-viewer repo?~~ -> Decided: mainline slime pre-work first, then bootstrap from here.
- ~~Should handoff and checkpoint tooling be ported before any code extraction?~~ -> Decided: yes, at Block 1 boundary.
- Do you want a global Adam-wide instruction file after the repo-local wording feels right?
- Should the structural probe also cover `fractal_renderer.cu` and `fractal_types.h`, or just main.cpp?
- Should extracted modules live in `ui_app/src/modules/` or flat alongside main.cpp?