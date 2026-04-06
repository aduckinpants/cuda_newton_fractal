# Lambda / Quadratic Conjugacy — Design Note

## Summary

The Lambda (logistic) map and the quadratic (Mandelbrot/Julia) family are related
by an affine conjugacy. This note records the relationship, explains what it means
for the viewer, and proposes whether Lambda should remain a separate slot or converge.

## The Conjugacy

The logistic map iterates:

    z_{n+1} = lambda * z * (1 - z)

The quadratic family iterates:

    w_{n+1} = w^2 + c

They are related by the substitution:

    w = lambda * (1/2 - z)   =>   z = 1/2 - w/lambda

Under this change of variable the logistic map becomes:

    w_{n+1} = w^2 + c   where   c = lambda*(1 - lambda)/4

The inverse gives:

    lambda = (1 +/- sqrt(1 - 4c)) / 2

## Implications

1. The Lambda parameter plane is a linear reparameterization of the Mandelbrot
   c-plane. The connectedness locus (filled Julia sets connected) is identical up
   to this affine map. Structure is the same; coordinates differ.

2. The *dynamic* plane of the Lambda map for a fixed lambda is conjugate to the
   Julia set for the corresponding c. Center and scale differ by the affine map.

3. The default lambda = 2.9685855 - 0.27446103i was computed from the classic
   Julia parameter c = -0.7 + 0.27015i via the conjugacy:
       lambda = (1 + sqrt(1 - 4c)) / 2
   giving a visually interesting connected Julia set in Lambda coordinates.

## Decision: Keep Lambda as a Separate Slot

Lambda stays as a distinct fractal type for these reasons:

- **Pedagogical clarity.** The logistic map is a canonical entry point in discrete
  dynamics courses. Having it as a named type makes it immediately recognizable to
  users who learned it that way.
- **Coordinate frame.** The dynamic plane is centered at z = 1/2 (the critical
  point of z*(1-z)) rather than at 0. This gives a different visual default even
  though the topology is the same.
- **Explaino bridge.** The Explaino-Lambda variant seeds z via the Explaino
  polynomial warp surface rather than using z = coord. The logistic iteration
  z -> lambda*z*(1-z) is simpler and cheaper than z -> z^2 + c because it avoids
  the Julia constant lookup, making it a natural match for the warp-start seam.
- **Future growth.** Real-valued logistic map visualizations (bifurcation diagrams,
  Lyapunov exponents over the real lambda axis) are natural extensions that do not
  have a clean analog in the quadratic formulation.

Lambda should NOT be merged into the Julia type with a hidden coordinate transform.

## Explaino-Lambda Contract

`explaino_lambda` is an Explaino escape-time family:

- Initial z comes from `explaino_warp_start(coord, seed, phase, strength)`.
- Iteration is `z_{n+1} = lambda * z * (1 - z)`.
- Uses `lambda_real`, `lambda_imag` from KernelParams (shared with plain Lambda).
- Escape-time coloring (smooth_escape default). No basin coloring.
- max_iter 1200, exposure 1.4, view center (0.5, 0) zoom 4.5.
