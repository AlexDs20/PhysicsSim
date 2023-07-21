# Constrained dynamics
In theory, it is possible to write exactly the dynamics of constrained system like coupled pendulum.

## Main methods
- Generalized coordinates
    - problem specific
    - satisfy the constrained by construction
- Solve for constraint force

## Solve for constraints
- If constraint not satisfied
    - move objects so that it is satisfied
    - also need to adjust the velocity, not just position
        - v &larra; v + g * dt
        - p &larr; x
        - x &larr; x + v * dt
        - x &larr; satisfyConstraint(x)
        - v &larr; (x - p) / dt

