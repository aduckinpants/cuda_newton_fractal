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
    magnet, "magnet",
    "Magnet Type I is the live engine selector for bounded 2D rational escape-time Magnet Type I.",
    "The pixel coordinate is c and the configured Magnet seed initializes z; the squared rational update ((z squared - 1 + c) / (2z - 2 + c)) squared is blended from the prior z by relaxation clamped to 0.05 through 1.5.",
    "For fixed c, seed, and relaxation, the Magnet recurrence is first-order in z.",
    "Iteration converges when squared distance from z to the unit attractor is below epsilon squared, or otherwise stops at the configured Magnet bailout, a nonfinite state, or the iteration limit.",
    "At high zoom, a small dynamics-parameter change can move or reorganize visible structures substantially; compare against the exact viewport facts rather than inferring visual scale from numeric magnitude.",
    "ui_app/src/fractal_catalog.h#kFractalCatalog", "ui_app/src/escape_time_direct_formulas.h#StepEscapeTimeDirectState",
    "ui_app/src/fractal_sample_device.inl#FractalSample", nullptr)
