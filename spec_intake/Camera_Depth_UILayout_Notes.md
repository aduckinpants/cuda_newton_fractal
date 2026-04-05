# Camera, Depth, and UI Layout — Open Issues & Future Plans

Status: notes / planning (not ready for implementation)

## Camera Behavior

The camera_behavior enum offers manual, complexity, orbit, entropy, and off —
but only "manual" and "off" have any real effect. The auto-dive path in
ApplyAutoDivePerFrame does a simple per-frame zoom-in scaled by dive_speed;
it ignores the selected behavior entirely (complexity/orbit/entropy all do
the same thing: zoom toward center).

### What's Missing

- **Complexity gradient**: Should steer the view toward regions of high
  iteration-count variance. Requires a per-frame analysis pass on the
  iteration buffer to compute a gradient direction.
- **Orbit attractor**: Should move the camera toward a periodic orbit or
  fixed point of the iteration. Needs root/orbit detection in the kernel.
- **Entropy seeker**: Should steer toward regions of maximum information
  content (Shannon entropy of the iteration histogram). Requires a histogram
  reduction pass.

### Recommendation

Implement complexity gradient first — it's the simplest (just a weighted
center-of-mass of iteration count). Orbit and entropy are more exotic and
can wait.

---

## Depth Algorithm

The current "depth" is just iteration count normalized to [0, max_iter].
For Newton-family fractals this is often very shallow (most pixels converge
in < 20 iterations) so the depth map is nearly uniform.

### Better Depth Ideas

- **Distance estimation**: Use the analytic derivative of the iterate to
  compute an estimated distance to the nearest root/boundary. This produces
  smooth, high-contrast depth maps.
- **Potential function**: For escape-time fractals, the potential function
  (log of the escape radius) gives a smooth height field.
- **Min-distance-to-root**: Track the closest approach to any known root
  during iteration. This produces sharp basin-boundary depth.

### Dependencies

Distance estimation requires computing f'(z) alongside f(z) in the kernel.
The Newton kernel already computes f'(z) for the Newton step, so the
incremental cost is low: just track min|f(z)/f'(z)| across iterations and
output it as a float mask channel.

---

## UI Layout Rethink

### Current Pain Points

- Top-down linear layout wastes horizontal space (buttons, sliders all
  full-width even when content is small).
- Prev/Next seed buttons now inline (fixed this session) but other button
  groups still stack vertically.
- Many controls hidden behind `visible_if` predicates, so the panel collapses
  dramatically when switching fractal types — can feel disorienting.

### Possible Directions

1. **Compact two-column layout**: Keep schema-driven rendering but add a
   `layout` hint per control (`"layout": "inline"` or `"layout": "half_width"`)
   that the renderer respects via SameLine + SetNextItemWidth.

2. **Tabbed panels instead of collapsing headers**: ImGui::BeginTabBar /
   BeginTabItem. Fractal/Color/View/Render as tabs. Reduces scroll distance.

3. **Dockable sub-windows**: Let the user undock panels (ImGui docking
   branch). Very powerful but requires the docking-enabled ImGui build.

### Recommendation

Start with option 1 (layout hints in schema) — minimal code change, maximum
schema-driven flexibility. Option 2 (tabs) is a good follow-up. Option 3
needs ImGui docking which is a larger dependency change.

---

## Missing Controls from Older Binding

From user feedback, the older version had:
- Fine camera controls (pan step size, zoom step, rotation step)
- Default fractal type set to explaino (currently defaults to newton)
- Possibly additional seed manipulation controls

### Action Items

- Add `pan_step`, `zoom_step`, `rotation_step` as View panel controls
  (these would be used by keyboard shortcuts / button-based navigation).
- Consider defaulting fractal_type to explaino in the schema (change
  default from "newton" to "explaino") since that's the primary use case.
- Audit the old binding surface (if available) for any other lost controls.
