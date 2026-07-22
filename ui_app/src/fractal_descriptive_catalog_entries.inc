// Generated reviewed overlay rows. Runtime data only; evidence is checked separately.
FRACTAL_REVIEWED_DESCRIPTION(
    newton, "newton",
    "Newton is the live engine selector for classical Newton basin fractal.",
    "z_{n+1}=z_n-P(z_n)/P'(z_n), with P from poly_coeffs (default z^3-1).",
    "first-order in z_n.",
    "The orbit converges when polynomial residual is below epsilon; it otherwise stops on a degenerate derivative, nonfinite state, or the iteration limit.",
    "Changing the polynomial or root layout can move, merge, or split basin features, so feature tracking requires mathematical continuation and exact viewport geometry.",
    "ui_app/src/fractal_catalog.h#kFractalCatalog", "ui_app/src/fractal_sample_device.inl#FractalSample",
    nullptr, nullptr)
FRACTAL_REVIEWED_DESCRIPTION(
    nova, "nova",
    "Nova is the live engine selector for Nova root-finding escape-time hybrid.",
    "z_{n+1}=z_n-alpha*P(z_n)/P'(z_n)+c, with z_0=0 and c=pixel coordinate.",
    "first-order in z_n with fixed additive c.",
    "The orbit converges when polynomial residual is below epsilon and is classified as escaped for invalid alpha, squared magnitude above 4, or nonfinite state; it otherwise reaches the iteration limit.",
    "At high zoom, a small dynamics-parameter change can move or reorganize visible structures substantially; compare against the exact viewport facts rather than inferring visual scale from numeric magnitude.",
    "ui_app/src/fractal_catalog.h#kFractalCatalog", "ui_app/src/fractal_sample_device.inl#FractalSample",
    nullptr, nullptr)
FRACTAL_REVIEWED_DESCRIPTION(
    mandelbrot, "mandelbrot",
    "Mandelbrot is the live engine selector for quadratic escape-time.",
    "z_{n+1}=z_n^2+c, z_0=0.",
    "first-order in z_n.",
    "escaped if |z_n|^2>4; otherwise interior/max-iteration classification.",
    "At high zoom, a small dynamics-parameter change can move or reorganize visible structures substantially; compare against the exact viewport facts rather than inferring visual scale from numeric magnitude.",
    "ui_app/src/fractal_catalog.h#kFractalCatalog", "ui_app/src/escape_time_direct_formulas.h#StepEscapeTimeDirectState",
    "ui_app/src/fractal_sample_device.inl#FractalSample", nullptr)
FRACTAL_REVIEWED_DESCRIPTION(
    julia, "julia",
    "Julia is the live engine selector for quadratic Julia set.",
    "The pixel coordinate initializes z, while the serialized Julia constant c supplies the fixed parameter in z_next = z squared + c.",
    "first-order in z_n.",
    "escaped if |z_n|^2>4; otherwise interior/max-iteration classification.",
    "At high zoom, a small dynamics-parameter change can move or reorganize visible structures substantially; compare against the exact viewport facts rather than inferring visual scale from numeric magnitude.",
    "ui_app/src/fractal_catalog.h#kFractalCatalog", "ui_app/src/escape_time_direct_formulas.h#StepEscapeTimeDirectState",
    "ui_app/src/fractal_sample_device.inl#FractalSample", nullptr)
FRACTAL_REVIEWED_DESCRIPTION(
    burning_ship, "burning_ship",
    "Burning Ship is the live engine selector for Burning Ship escape-time.",
    "The direct formula blends each component of z toward its absolute value by burning_ship_fold_mix, squares that folded value, and adds the pixel parameter c.",
    "first-order in z_n.",
    "escaped if |z_n|^2>4; otherwise interior/max-iteration classification.",
    "At high zoom, a small dynamics-parameter change can move or reorganize visible structures substantially; compare against the exact viewport facts rather than inferring visual scale from numeric magnitude.",
    "ui_app/src/fractal_catalog.h#kFractalCatalog", "ui_app/src/escape_time_direct_formulas.h#StepEscapeTimeDirectState",
    "ui_app/src/fractal_sample_device.inl#FractalSample", nullptr)
FRACTAL_REVIEWED_DESCRIPTION(
    multibrot, "multibrot",
    "Multibrot is the live engine selector for generalized polynomial escape-time.",
    "The direct formula computes z_next = z raised to the configured principal complex power plus c; a zero imaginary exponent reduces to the principal real-power path.",
    "first-order in z_n.",
    "escaped if |z_n|^2>4; otherwise interior/max-iteration classification.",
    "At high zoom, a small dynamics-parameter change can move or reorganize visible structures substantially; compare against the exact viewport facts rather than inferring visual scale from numeric magnitude.",
    "ui_app/src/fractal_catalog.h#kFractalCatalog", "ui_app/src/escape_time_direct_formulas.h#StepEscapeTimeDirectState",
    "ui_app/src/fractal_sample_device.inl#FractalSample", nullptr)
FRACTAL_REVIEWED_DESCRIPTION(
    phoenix, "phoenix",
    "Phoenix is the live engine selector for Phoenix two-step escape-time.",
    "z_{n+1}=z_n^2+c+p*z_{n-1}.",
    "second-order in (z_n,z_{n-1}).",
    "escaped if |z_n|^2>4 or state becomes nonfinite.",
    "At high zoom, a small dynamics-parameter change can move or reorganize visible structures substantially; compare against the exact viewport facts rather than inferring visual scale from numeric magnitude.",
    "ui_app/src/fractal_catalog.h#kFractalCatalog", "ui_app/src/escape_time_direct_formulas.h#StepEscapeTimeDirectState",
    "ui_app/src/fractal_sample_device.inl#FractalSample", nullptr)
FRACTAL_REVIEWED_DESCRIPTION(
    explaino, "explaino",
    "ExplainO is the live engine selector for seeded damped Newton on generated degree-4 polynomial.",
    "Starting from an ExplainO-warped pixel coordinate, the engine applies z_next = z - damping times P(z)/P'(z) to the current degree-four polynomial coefficients.",
    "first-order in z_n.",
    "The orbit converges when polynomial residual is below epsilon; it otherwise stops on a degenerate derivative, nonfinite state, or the iteration limit.",
    "At high zoom, a small dynamics-parameter change can move or reorganize visible structures substantially; compare against the exact viewport facts rather than inferring visual scale from numeric magnitude.",
    "ui_app/src/fractal_catalog.h#kFractalCatalog", "ui_app/src/fractal_sample_device.inl#FractalSample",
    nullptr, nullptr)
FRACTAL_REVIEWED_DESCRIPTION(
    explaino_y, "explaino_y",
    "ExplainO Y is the live engine selector for engineered multi-warp Newton composite.",
    "per-iteration re-warped Newton step on P, blended back into z with fixed mix, velocity carry, pixel-attractor pull, and soft radial clamp.",
    "second-order in (z_n,z_{n-1}) because velocity uses z_n-z_{n-1}.",
    "tracks best residual, accepts direct convergence at |P|<epsilon, otherwise snaps best point to nearest root for basin assignment.",
    "At high zoom, a small dynamics-parameter change can move or reorganize visible structures substantially; compare against the exact viewport facts rather than inferring visual scale from numeric magnitude.",
    "ui_app/src/fractal_catalog.h#kFractalCatalog", "ui_app/src/fractal_sample_device.inl#FractalSample",
    nullptr, nullptr)
FRACTAL_REVIEWED_DESCRIPTION(
    explaino_fp, "explaino_fp",
    "ExplainO FP is the live engine selector for adaptive-damped seeded Newton with fallback projection.",
    "z_{n+1}=z_n-[lambda/(1+|s_n|)]*s_n, where s_n=P/P' when derivative is usable and s_n=P otherwise; large-radius states are clamped back to radius 4.",
    "first-order in z_n.",
    "direct convergence at |P|<epsilon; if not converged by the limit, snap to nearest known root and still emit a basin classification.",
    "At high zoom, a small dynamics-parameter change can move or reorganize visible structures substantially; compare against the exact viewport facts rather than inferring visual scale from numeric magnitude.",
    "ui_app/src/fractal_catalog.h#kFractalCatalog", "ui_app/src/fractal_sample_device.inl#FractalSample",
    nullptr, nullptr)
FRACTAL_REVIEWED_DESCRIPTION(
    explaino_nova, "explaino_nova",
    "ExplainO Nova is the live engine selector for Nova on explaino-generated polynomial.",
    "Starting from z_0 = 0 with an ExplainO-warped pixel coordinate c, the engine applies z_next = z - alpha times damping times P(z)/P'(z) + c to the current polynomial coefficients.",
    "first-order in z_n with additive c.",
    "The orbit converges when polynomial residual is below epsilon and is classified as escaped for invalid alpha, squared magnitude above 4, or nonfinite state; it otherwise reaches the iteration limit.",
    "At high zoom, a small dynamics-parameter change can move or reorganize visible structures substantially; compare against the exact viewport facts rather than inferring visual scale from numeric magnitude.",
    "ui_app/src/fractal_catalog.h#kFractalCatalog", "ui_app/src/fractal_sample_device.inl#FractalSample",
    nullptr, nullptr)
FRACTAL_REVIEWED_DESCRIPTION(
    explaino_halley, "explaino_halley",
    "ExplainO Halley is the live engine selector for seeded damped Halley basin fractal.",
    "Starting from an ExplainO-warped pixel coordinate, the engine applies a damped Halley step using P, P', and P'' from the current polynomial coefficients.",
    "first-order in z_n.",
    "The orbit converges when polynomial residual is below epsilon; it otherwise stops on a degenerate Halley denominator, nonfinite state, or the iteration limit.",
    "At high zoom, a small dynamics-parameter change can move or reorganize visible structures substantially; compare against the exact viewport facts rather than inferring visual scale from numeric magnitude.",
    "ui_app/src/fractal_catalog.h#kFractalCatalog", "ui_app/src/fractal_sample_device.inl#FractalSample",
    nullptr, nullptr)
FRACTAL_REVIEWED_DESCRIPTION(
    explaino_dual, "explaino_dual",
    "ExplainO Dual Seed derives one root shape from the primary seed and one from the secondary seed, then blends the shapes by the bounded mix control before forming polynomial coefficients.",
    "Starting from an ExplainO-warped pixel coordinate, the engine applies the same damped Newton update as the base ExplainO selector to those derived polynomial coefficients.",
    "first-order in z_n.",
    "The orbit converges when polynomial residual is below epsilon; it otherwise stops on a degenerate derivative, nonfinite state, or the iteration limit.",
    "At high zoom, a small dynamics-parameter change can move or reorganize visible structures substantially; compare against the exact viewport facts rather than inferring visual scale from numeric magnitude.",
    "ui_app/src/fractal_derived_fields.cpp#ResolveExplainoSeedShape", "ui_app/src/fractal_sample_device.inl#FractalSample",
    nullptr, nullptr)
FRACTAL_REVIEWED_DESCRIPTION(
    explaino_mult, "explaino_mult",
    "ExplainO Multi Root derives four roots as two axis-aligned pairs split around the current seed shape by the bounded cluster-radius control.",
    "Starting from an ExplainO-warped pixel coordinate, the engine applies the same damped Newton update as the base ExplainO selector to the polynomial formed from those roots.",
    "first-order in z_n.",
    "The orbit converges when polynomial residual is below epsilon; it otherwise stops on a degenerate derivative, nonfinite state, or the iteration limit.",
    "At high zoom, a small dynamics-parameter change can move or reorganize visible structures substantially; compare against the exact viewport facts rather than inferring visual scale from numeric magnitude.",
    "ui_app/src/fractal_derived_fields.cpp#SetExplainoRootsForShape", "ui_app/src/fractal_sample_device.inl#FractalSample",
    nullptr, nullptr)
FRACTAL_REVIEWED_DESCRIPTION(
    explaino_phoenix, "explaino_phoenix",
    "ExplainO Phoenix is the live engine selector for seeded Newton plus Phoenix memory.",
    "Starting from an ExplainO-warped pixel coordinate, the engine subtracts an adaptively damped Newton step, adds the configured Phoenix coefficient times the previous z, and pulls states outside radius four back to that radius.",
    "The update is first-order in z when the configured Phoenix coefficient is zero and second-order in stored orbit state when that memory term is active.",
    "direct convergence at |P|<epsilon; otherwise residual-best orbit is later snapped to the nearest root for basin assignment.",
    "At high zoom, a small dynamics-parameter change can move or reorganize visible structures substantially; compare against the exact viewport facts rather than inferring visual scale from numeric magnitude.",
    "ui_app/src/fractal_catalog.h#kFractalCatalog", "ui_app/src/fractal_sample_device.inl#FractalSample",
    nullptr, nullptr)
FRACTAL_REVIEWED_DESCRIPTION(
    explaino_transcendental, "explaino_transcendental",
    "ExplainO Transcendental is the live engine selector for seeded Newton on transcendental function family.",
    "Starting from an ExplainO-warped pixel coordinate, the engine applies a damped Newton step to sin(z), exp(z)-1, or cosh(z), as selected by transcendental_func.",
    "first-order in z_n.",
    "The orbit converges when function residual is below epsilon; it otherwise stops on a degenerate derivative, nonfinite state, or the iteration limit.",
    "At high zoom, a small dynamics-parameter change can move or reorganize visible structures substantially; compare against the exact viewport facts rather than inferring visual scale from numeric magnitude.",
    "ui_app/src/fractal_catalog.h#kFractalCatalog", "ui_app/src/fractal_sample_device.inl#FractalSample",
    nullptr, nullptr)
FRACTAL_REVIEWED_DESCRIPTION(
    explaino_inertial, "explaino_inertial",
    "ExplainO Inertial is the live engine selector for seeded heavy-ball Newton.",
    "Starting from an ExplainO-warped pixel coordinate, the engine subtracts the configured damped Newton step and adds momentum_beta times the difference between the two stored prior orbit states.",
    "third-order in (z_n,z_{n-1},z_{n-2}).",
    "stop when |P(z_n)|<epsilon or when |P'(z_n)| is effectively zero.",
    "At high zoom, a small dynamics-parameter change can move or reorganize visible structures substantially; compare against the exact viewport facts rather than inferring visual scale from numeric magnitude.",
    "ui_app/src/fractal_catalog.h#kFractalCatalog", "ui_app/src/fractal_sample_device.inl#FractalSample",
    nullptr, nullptr)
FRACTAL_REVIEWED_DESCRIPTION(
    explaino_julia, "explaino_julia",
    "ExplainO Julia is the live engine selector for warp-started Julia bridge.",
    "The engine iterates z_next = z squared + c_J from an ExplainO-warped pixel coordinate; c_J is either the configured custom ExplainO Julia constant, the first available ExplainO root, or the documented fallback constant.",
    "first-order in z_n.",
    "The orbit is classified as escaped when squared magnitude exceeds 4 or the state becomes nonfinite; otherwise it reaches the iteration limit.",
    "At high zoom, a small dynamics-parameter change can move or reorganize visible structures substantially; compare against the exact viewport facts rather than inferring visual scale from numeric magnitude.",
    "ui_app/src/fractal_catalog.h#kFractalCatalog", "ui_app/src/fractal_sample_device.inl#FractalSample",
    nullptr, nullptr)
FRACTAL_REVIEWED_DESCRIPTION(
    explaino_rational, "explaino_rational",
    "ExplainO Rational is the live engine selector for seeded Newton on Laurent-perturbed polynomial.",
    "Starting from an ExplainO-warped pixel coordinate, the engine applies damped Newton iteration to F(z)=P(z)+alpha/z with derivative F'(z)=P'(z)-alpha/z squared.",
    "first-order in z_n.",
    "The orbit converges when residual is below epsilon; it otherwise stops at the pole neighborhood, on a degenerate derivative, nonfinite state, or the iteration limit.",
    "At high zoom, a small dynamics-parameter change can move or reorganize visible structures substantially; compare against the exact viewport facts rather than inferring visual scale from numeric magnitude.",
    "ui_app/src/fractal_catalog.h#kFractalCatalog", "ui_app/src/fractal_sample_device.inl#FractalSample",
    nullptr, nullptr)
FRACTAL_REVIEWED_DESCRIPTION(
    multicorn, "multicorn",
    "Multicorn is the live engine selector for antiholomorphic multicorn.",
    "z_{n+1}=conj(z_n)^p+c using integer power p=multibrot_power.",
    "first-order in z_n.",
    "escaped if |z_n|^2>4; otherwise interior/max-iteration classification.",
    "At high zoom, a small dynamics-parameter change can move or reorganize visible structures substantially; compare against the exact viewport facts rather than inferring visual scale from numeric magnitude.",
    "ui_app/src/fractal_catalog.h#kFractalCatalog", "ui_app/src/escape_time_direct_formulas.h#StepEscapeTimeDirectState",
    "ui_app/src/fractal_sample_device.inl#FractalSample", nullptr)
FRACTAL_REVIEWED_DESCRIPTION(
    halley, "halley",
    "Halley is the live engine selector for classical Halley basin fractal.",
    "z_{n+1}=z_n-[2P(z_n)P'(z_n)]/[2(P'(z_n))^2-P(z_n)P''(z_n)].",
    "first-order in z_n.",
    "The orbit converges when polynomial residual is below epsilon; it otherwise stops on a degenerate Halley denominator, nonfinite state, or the iteration limit.",
    "Changing the polynomial or root layout can move, merge, or split basin features, so feature tracking requires mathematical continuation and exact viewport geometry.",
    "ui_app/src/fractal_catalog.h#kFractalCatalog", "ui_app/src/fractal_sample_device.inl#FractalSample",
    nullptr, nullptr)
FRACTAL_REVIEWED_DESCRIPTION(
    collatz, "collatz",
    "Collatz is the live engine selector for analytic Collatz escape-time.",
    "The engine evaluates the analytic Collatz extension z_next = ((2 + 7z) - transition_strength times (2 + 5z) cos(pi z)) / 4.",
    "first-order in z_n.",
    "The orbit is classified as escaped when squared magnitude exceeds 10000 or the state becomes nonfinite; otherwise it reaches the iteration limit.",
    "At high zoom, a small dynamics-parameter change can move or reorganize visible structures substantially; compare against the exact viewport facts rather than inferring visual scale from numeric magnitude.",
    "ui_app/src/fractal_catalog.h#kFractalCatalog", "ui_app/src/escape_time_specialized_formulas.h#StepCollatzEscapeState",
    "ui_app/src/fractal_sample_device.inl#FractalSample", nullptr)
FRACTAL_REVIEWED_DESCRIPTION(
    explaino_collatz, "explaino_collatz",
    "ExplainO Collatz is the live engine selector for seeded Newton on analytic Collatz fixed-point residual.",
    "Newton iteration on F(z)=[(2+3z)-(2+5z)cos(pi z)]/4 with derivative computed analytically, using warp-started z_0.",
    "first-order in z_n.",
    "stop when |F(z_n)|<epsilon; stop early on derivative degeneracy or nonfinite state.",
    "At high zoom, a small dynamics-parameter change can move or reorganize visible structures substantially; compare against the exact viewport facts rather than inferring visual scale from numeric magnitude.",
    "ui_app/src/fractal_catalog.h#kFractalCatalog", "ui_app/src/fractal_sample_device.inl#FractalSample",
    nullptr, nullptr)
FRACTAL_REVIEWED_DESCRIPTION(
    explaino_collatz_direct, "explaino_collatz_direct",
    "ExplainO Collatz Direct applies the direct analytic Collatz escape-time map from an ExplainO-warped starting coordinate.",
    "After the seed, phase, and warp controls establish z_0, the engine evaluates z_next = ((2 + 7z) - transition_strength times (2 + 5z) cos(pi z)) / 4.",
    "For fixed warp controls and transition strength, the orbit is first-order in z.",
    "The orbit escapes at squared radius 10000, a nonfinite state, or the iteration limit.",
    "Both the initial warp and the Collatz transition strength can move high-zoom structures; their numeric magnitudes do not bound visual displacement.",
    "ui_app/src/fractal_catalog.h#kFractalCatalog", "ui_app/src/fractal_sample_device.inl#FractalSample",
    nullptr, nullptr)
FRACTAL_REVIEWED_DESCRIPTION(
    mcmullen, "mcmullen",
    "McMullen is the live engine selector for McMullen rational escape-time family.",
    "The engine iterates z_next = z to integer power m plus lambda times z to negative integer power n, with m, n, and lambda resolved from the selected preset or bounded custom controls.",
    "first-order in z_n.",
    "The orbit is classified as escaped when it reaches the pole neighborhood near zero, exceeds the specialized squared radius 10000, becomes nonfinite, or exhausts the iteration limit.",
    "At high zoom, a small dynamics-parameter change can move or reorganize visible structures substantially; compare against the exact viewport facts rather than inferring visual scale from numeric magnitude.",
    "ui_app/src/fractal_catalog.h#kFractalCatalog", "ui_app/src/escape_time_specialized_formulas.h#StepMcMullenEscapeState",
    "ui_app/src/fractal_sample_device.inl#FractalSample", nullptr)
FRACTAL_REVIEWED_DESCRIPTION(
    lambda_map, "lambda",
    "Lambda is the live engine selector for complex logistic / lambda map.",
    "z_{n+1}=lambda*z_n*(1-z_n).",
    "first-order in z_n.",
    "escaped if |z_n|^2>4 or the state becomes nonfinite.",
    "At high zoom, a small dynamics-parameter change can move or reorganize visible structures substantially; compare against the exact viewport facts rather than inferring visual scale from numeric magnitude.",
    "ui_app/src/fractal_catalog.h#kFractalCatalog", "ui_app/src/escape_time_direct_formulas.h#StepEscapeTimeDirectState",
    "ui_app/src/fractal_sample_device.inl#FractalSample", nullptr)
FRACTAL_REVIEWED_DESCRIPTION(
    explaino_lambda, "explaino_lambda",
    "ExplainO Lambda is the live engine selector for warp-started explaino lambda bridge.",
    "z_{n+1}=lambda*z_n*(1-z_n), with z_0=warp_start(coord,seed,phase,strength).",
    "first-order in z_n.",
    "escaped if |z_n|^2>4 or state becomes nonfinite.",
    "At high zoom, a small dynamics-parameter change can move or reorganize visible structures substantially; compare against the exact viewport facts rather than inferring visual scale from numeric magnitude.",
    "ui_app/src/fractal_catalog.h#kFractalCatalog", "ui_app/src/fractal_sample_device.inl#FractalSample",
    nullptr, nullptr)
FRACTAL_REVIEWED_DESCRIPTION(
    explaino_rational_escape, "explaino_rational_escape",
    "ExplainO Rational Escape is the live engine selector for warp-started explaino Laurent escape-time.",
    "The engine iterates z_next = P(z) divided by z raised to a serialized denominator power clamped from 1 through 6, starting from an ExplainO-warped pixel coordinate.",
    "first-order in z_n.",
    "The orbit is classified as escaped at the pole near zero, when squared magnitude exceeds 10000, or when the state becomes nonfinite; otherwise it reaches the iteration limit.",
    "At high zoom, a small dynamics-parameter change can move or reorganize visible structures substantially; compare against the exact viewport facts rather than inferring visual scale from numeric magnitude.",
    "ui_app/src/fractal_catalog.h#kFractalCatalog", "ui_app/src/fractal_sample_device.inl#FractalSample",
    nullptr, nullptr)
FRACTAL_REVIEWED_DESCRIPTION(
    spider, "spider",
    "Spider is the live engine selector for Spider escape-time.",
    "The engine advances z_next = z squared + c and then advances the parameter state as c_next = clamp(spider_feedback, -2, 2) times c plus z_next.",
    "The recurrence is first-order in the augmented state pair (z_n, c_n).",
    "escaped if |z_n|^2>4 or the state becomes nonfinite.",
    "At high zoom, a small dynamics-parameter change can move or reorganize visible structures substantially; compare against the exact viewport facts rather than inferring visual scale from numeric magnitude.",
    "ui_app/src/fractal_catalog.h#kFractalCatalog", "ui_app/src/escape_time_direct_formulas.h#StepEscapeTimeDirectState",
    "ui_app/src/fractal_sample_device.inl#FractalSample", nullptr)
FRACTAL_REVIEWED_DESCRIPTION(
    celtic_mandelbrot, "celtic_mandelbrot",
    "Celtic Mandelbrot is the live engine selector for Celtic Mandelbrot escape-time.",
    "The engine squares z, blends the real quadratic term toward its absolute value by celtic_abs_mix, preserves the 2xy imaginary term, and adds c.",
    "first-order in z_n.",
    "escaped if |z_n|^2>4 or the state becomes nonfinite.",
    "At high zoom, a small dynamics-parameter change can move or reorganize visible structures substantially; compare against the exact viewport facts rather than inferring visual scale from numeric magnitude.",
    "ui_app/src/fractal_catalog.h#kFractalCatalog", "ui_app/src/escape_time_direct_formulas.h#StepEscapeTimeDirectState",
    "ui_app/src/fractal_sample_device.inl#FractalSample", nullptr)
FRACTAL_REVIEWED_DESCRIPTION(
    perpendicular_burning_ship, "perpendicular_burning_ship",
    "Perpendicular Burning Ship is the live engine selector for Perpendicular Burning Ship escape-time.",
    "The engine blends the real component toward its absolute value by perpendicular_fold_mix for the 2xy term, retains x squared minus y squared, and adds c.",
    "first-order in z_n.",
    "escaped if |z_n|^2>4 or the state becomes nonfinite.",
    "At high zoom, a small dynamics-parameter change can move or reorganize visible structures substantially; compare against the exact viewport facts rather than inferring visual scale from numeric magnitude.",
    "ui_app/src/fractal_catalog.h#kFractalCatalog", "ui_app/src/escape_time_direct_formulas.h#StepEscapeTimeDirectState",
    "ui_app/src/fractal_sample_device.inl#FractalSample", nullptr)
FRACTAL_REVIEWED_DESCRIPTION(
    explaino_joy, "explaino_joy",
    "ExplainO Joy is the live engine selector for Joy-coupled explaino Newton.",
    "The engine blends the Newton step P/P' with the root-critical step P'/P'' by joy_coupling, applies configured adaptive damping, adds optional Phoenix memory, and pulls states outside radius four back to that radius.",
    "The update is first-order in z when the shared Phoenix coefficient is zero and second-order in stored orbit state when that memory term is active.",
    "direct convergence at |P|<epsilon; otherwise best-residual orbit is snapped to the nearest available root for basin assignment.",
    "At high zoom, a small dynamics-parameter change can move or reorganize visible structures substantially; compare against the exact viewport facts rather than inferring visual scale from numeric magnitude.",
    "ui_app/src/fractal_catalog.h#kFractalCatalog", "ui_app/src/fractal_sample_device.inl#FractalSample",
    nullptr, nullptr)
FRACTAL_REVIEWED_DESCRIPTION(
    explaino_fold, "explaino_fold",
    "ExplainO Fold is the live engine selector for folded-step explaino Newton.",
    "newtonStep=P/P'; foldedStep=(|Re newtonStep|,|Im newtonStep|); combined=(1-alpha)*newtonStep+alpha*foldedStep; z_{n+1}=z_n-damp_n*combined+p*z_{n-1}.",
    "The update is first-order in z when the shared Phoenix coefficient is zero and second-order in stored orbit state when that memory term is active.",
    "direct convergence at |P|<epsilon; otherwise best-residual orbit is snapped to the nearest available root for basin assignment.",
    "At high zoom, a small dynamics-parameter change can move or reorganize visible structures substantially; compare against the exact viewport facts rather than inferring visual scale from numeric magnitude.",
    "ui_app/src/fractal_catalog.h#kFractalCatalog", "ui_app/src/fractal_sample_device.inl#FractalSample",
    nullptr, nullptr)
FRACTAL_REVIEWED_DESCRIPTION(
    explaino_bell, "explaino_bell",
    "ExplainO Bell is the live engine selector for measurement/reaction explaino Newton.",
    "decompose newtonStep along the P-phase direction; attenuate the parallel measurement channel by beta so combinedStep = newtonStep - beta*s_parallel; then apply adaptive damping and optional Phoenix memory.",
    "The update is first-order in z when the shared Phoenix coefficient is zero and second-order in stored orbit state when that memory term is active.",
    "direct convergence at |P|<epsilon; otherwise best-residual orbit is snapped to the nearest available root for basin assignment.",
    "At high zoom, a small dynamics-parameter change can move or reorganize visible structures substantially; compare against the exact viewport facts rather than inferring visual scale from numeric magnitude.",
    "ui_app/src/fractal_catalog.h#kFractalCatalog", "ui_app/src/fractal_sample_device.inl#FractalSample",
    nullptr, nullptr)
FRACTAL_REVIEWED_DESCRIPTION(
    explaino_ripple, "explaino_ripple",
    "ExplainO Ripple is the live engine selector for ripple-kicked explaino Newton.",
    "baseline explaino Newton step plus a sinusoidal perpendicular kick whose phase depends on iteration count and arg(P'(z_n)); adaptive damping and optional Phoenix memory remain in the update.",
    "The update is first-order in z when the shared Phoenix coefficient is zero and second-order in stored orbit state when that memory term is active.",
    "direct convergence at |P|<epsilon; otherwise best-residual orbit is snapped to the nearest available root for basin assignment.",
    "At high zoom, a small dynamics-parameter change can move or reorganize visible structures substantially; compare against the exact viewport facts rather than inferring visual scale from numeric magnitude.",
    "ui_app/src/fractal_catalog.h#kFractalCatalog", "ui_app/src/fractal_sample_device.inl#FractalSample",
    nullptr, nullptr)
FRACTAL_REVIEWED_DESCRIPTION(
    explaino_splice, "explaino_splice",
    "ExplainO Splice is the live engine selector for alternating-polynomial explaino Newton.",
    "alternate between primary polynomial P_A and offset-seed splice polynomial P_B while keeping the explaino-style Newton update shape; P_B is regenerated from seed+splice_offset into poly_coeffs_b.",
    "second-order in (z_n,z_{n-1}) when Phoenix memory is active; otherwise first-order in z_n with alternating function state.",
    "convergence is still judged against the active polynomial residual and the basin assignment falls back to the primary explaino roots.",
    "At high zoom, a small dynamics-parameter change can move or reorganize visible structures substantially; compare against the exact viewport facts rather than inferring visual scale from numeric magnitude.",
    "ui_app/src/fractal_catalog.h#kFractalCatalog", "ui_app/src/fractal_sample_device.inl#FractalSample",
    nullptr, nullptr)
FRACTAL_REVIEWED_DESCRIPTION(
    explaino_vortex, "explaino_vortex",
    "ExplainO Vortex is the live engine selector for step-rotated explaino Newton.",
    "take the explaino Newton step s_n=P/P', rotate it by angle vortex_strength*arg(s_n), then apply explaino-style adaptive damping and optional Phoenix memory.",
    "The update is first-order in z when the shared Phoenix coefficient is zero and second-order in stored orbit state when that memory term is active.",
    "direct convergence at |P|<epsilon; otherwise best-residual orbit is snapped to the nearest available root for basin assignment.",
    "At high zoom, a small dynamics-parameter change can move or reorganize visible structures substantially; compare against the exact viewport facts rather than inferring visual scale from numeric magnitude.",
    "ui_app/src/fractal_catalog.h#kFractalCatalog", "ui_app/src/fractal_sample_device.inl#FractalSample",
    nullptr, nullptr)
FRACTAL_REVIEWED_DESCRIPTION(
    explaino_tension, "explaino_tension",
    "ExplainO Tension is the live engine selector for second-root-pull explaino Newton.",
    "z_{n+1}=z_n-damp_n*P/P'+T*(r_second-z_n)/|r_second-z_n|^2+p*z_{n-1}, where r_second is the second-closest explaino root.",
    "The update is first-order in z when the shared Phoenix coefficient is zero and second-order in stored orbit state when that memory term is active.",
    "direct convergence at |P|<epsilon; otherwise best-residual orbit is snapped to the nearest available root for basin assignment.",
    "At high zoom, a small dynamics-parameter change can move or reorganize visible structures substantially; compare against the exact viewport facts rather than inferring visual scale from numeric magnitude.",
    "ui_app/src/fractal_catalog.h#kFractalCatalog", "ui_app/src/fractal_sample_device.inl#FractalSample",
    nullptr, nullptr)
FRACTAL_REVIEWED_DESCRIPTION(
    explaino_balance_void, "explaino_balance_void",
    "ExplainO Balance Void is a dedicated seeded Newton branch with balance-void, symmetry-tension, and field-curvature biases derived from the two nearest roots.",
    "The engine forms a Newton step, applies configured adaptive damping, adds midpoint attraction, perpendicular symmetry, and tangential curvature biases from the nearest-root pair, and pulls states outside radius four back to that radius.",
    "The recurrence is first-order in the current z state for fixed roots and controls.",
    "The orbit accepts residual convergence; otherwise it retains the best-residual iteration, stops on nonfinite state or the iteration limit, and assigns the nearest available root.",
    "Configured bias axes identify enabled recurrence terms but do not by themselves establish visible contribution or spatially localize their effect in a captured frame.",
    "ui_app/src/fractal_catalog.h#kFractalCatalog", "ui_app/src/fractal_sample_device.inl#FractalSample",
    nullptr, nullptr)
FRACTAL_REVIEWED_DESCRIPTION(
    counterfactual_pair, "counterfactual_pair",
    "Counterfactual Pair compares two Newton orbits initialized at a sampled coordinate and a configured world-absolute or view-relative offset.",
    "Each member independently applies Newton's method to the selected cubic or quartic polynomial while the engine accumulates their mean separation.",
    "Each orbit is first-order in its own z state; the pair classifier additionally retains cross-orbit separation statistics.",
    "The pair is classified as reconverged at one root, same-root drift, different-root basin swap, or unresolved according to both orbit terminations, nearest roots, and the configured reconvergence ratio.",
    "The output is a synthetic pair class and mean-gap signal; it is not direct evidence that two distinct visible structures occupy the same image location.",
    "ui_app/src/fractal_catalog.h#kFractalCatalog", "ui_app/src/fractal_sample_device.inl#FractalSample",
    nullptr, nullptr)
FRACTAL_REVIEWED_DESCRIPTION(
    explaino_counterfactual_pair, "explaino_counterfactual_pair",
    "ExplainO Counterfactual Pair compares two damped Newton orbits initialized from an ExplainO-warped sample and a configured world-absolute or view-relative offset.",
    "Each member independently applies damped Newton iteration to the current ExplainO polynomial; nearest-root classification uses explicit ExplainO roots when present and otherwise the polynomial's unit-root family.",
    "Each orbit is first-order in its own z state; the pair classifier additionally retains cross-orbit separation statistics.",
    "The pair is classified as reconverged at one root, same-root drift, different-root basin swap, or unresolved according to both orbit terminations, nearest roots, and the configured reconvergence ratio.",
    "The initial warp and pair offset can both move the comparison at high zoom; the synthetic class does not spatially localize either orbit beyond the sampled coordinate.",
    "ui_app/src/fractal_catalog.h#kFractalCatalog", "ui_app/src/fractal_sample_device.inl#FractalSample",
    nullptr, nullptr)
FRACTAL_REVIEWED_DESCRIPTION(
    projection_and_flow, "projection_and_flow",
    "Projection and Flow applies Newton motion followed by radial projection onto a configured circle and classifies root identity together with transient projection pressure.",
    "After each unconstrained Newton step, a nonconverged finite state is projected to the target radius; the engine records projection pressure as the radial correction magnitude.",
    "The projected orbit is first-order in z, with accumulated peak and last projection pressure retained for final classification.",
    "A state can converge before or after the free Newton step, stop on a degenerate derivative or nonfinite state, or reach the iteration limit; the final class combines nearest root with one of four transient-pressure bands plus an unresolved class.",
    "Pressure bands summarize the difference between peak and final projection pressure and do not identify a spatial force field outside this sampled recurrence.",
    "ui_app/src/fractal_catalog.h#kFractalCatalog", "ui_app/src/fractal_sample_device.inl#FractalSample",
    nullptr, nullptr)
FRACTAL_REVIEWED_DESCRIPTION(
    explaino_projection_and_flow, "explaino_projection_and_flow",
    "ExplainO Projection and Flow applies adaptive damped Newton motion from an ExplainO-warped sample, then radially projects nonconverged states onto a configured circle.",
    "The free step uses the current ExplainO polynomial and damping divided by one plus Newton-step magnitude; finite nonconverged states are then projected to the target radius.",
    "The projected orbit is first-order in z, with accumulated peak and last projection pressure retained for final classification.",
    "A state can converge before or after the free step, stop on a degenerate derivative or nonfinite state, or reach the iteration limit; the final class combines nearest root with transient-pressure band.",
    "The initial warp, damping, target radius, and pressure threshold can all reorganize high-zoom structure; the pressure class does not by itself spatially localize a cause.",
    "ui_app/src/fractal_catalog.h#kFractalCatalog", "ui_app/src/fractal_sample_device.inl#FractalSample",
    nullptr, nullptr)
FRACTAL_REVIEWED_DESCRIPTION(
    magnet, "magnet",
    "Magnet Type I is the live engine selector for bounded 2D rational escape-time Magnet Type I.",
    "The pixel coordinate is c and the configured Magnet seed initializes z; the squared rational update ((z squared - 1 + c) / (2z - 2 + c)) squared is blended from the prior z by relaxation clamped to 0.05 through 1.5.",
    "For fixed c, seed, and relaxation, the Magnet recurrence is first-order in z.",
    "Iteration converges when squared distance from z to the unit attractor is below epsilon squared, or otherwise stops at the configured Magnet bailout, a nonfinite state, or the iteration limit.",
    "At high zoom, a small dynamics-parameter change can move or reorganize visible structures substantially; compare against the exact viewport facts rather than inferring visual scale from numeric magnitude.",
    "ui_app/src/fractal_catalog.h#kFractalCatalog", "ui_app/src/escape_time_direct_formulas.h#StepEscapeTimeDirectState",
    "ui_app/src/fractal_sample_device.inl#FractalSample", nullptr)
