<img width="1670" height="940" alt="image" src="https://github.com/user-attachments/assets/f84bb0d6-1127-4390-ac99-29a9800c6829" />
# AutoAimAssist

### Mathematical Targeting & Screen-Space Interpolation for Unreal Engine

**AutoAimAssist** is a C++ gameplay component designed to explore mathematical aim assistance and target tracking techniques in Unreal Engine.

The system combines 3D world-space projection, 2D screen-space vector calculations, and interpolation algorithms to determine smooth and controlled aim adjustments toward selected targets.

The primary focus is to demonstrate how mathematical transformations and interpolation techniques can be applied to real-time gameplay systems.

---

## Overview

In 3D environments, targets exist in world space, while aiming interfaces and crosshairs operate in screen space.

AutoAimAssist bridges these two coordinate systems by projecting selected target positions onto the screen and calculating the displacement required to align the crosshair with the projected target.

Instead of applying an instantaneous correction, the system uses interpolation to produce gradual and controlled aim movement.

## Mathematical Model

The targeting algorithm is based on three fundamental mathematical operations.

### 1. World-to-Screen Projection

The selected target position, typically obtained from a skeletal bone transform, is projected from 3D world coordinates into 2D screen coordinates.

\[
S = ProjectWorldToScreen(H)
\]

Where:

- **H** — Target position in world space.
- **S** — Projected target position in screen space.

This transformation relies on the camera's view and projection matrices.

### 2. Screen-Space Vector Calculation

After obtaining the projected target position, the system calculates the displacement between the target and the current crosshair.

\[
\Delta = S-C
\]

Where:

- **S** — Projected target position.
- **C** — Current crosshair position.
- **Δ** — Required screen-space displacement.

**Example:**

\[
C=(960,540)
\]

\[
S=(1120,420)
\]

Therefore:

\[
\Delta=(160,-120)
\]

The resulting vector represents the direction and magnitude of the required correction.

### 3. Smooth Aim Interpolation

To avoid instantaneous position changes, the system applies Linear Interpolation (LERP).

\[
C_{new}=C+(S-C)\cdot\alpha
\]

Where:

- **C** — Current crosshair position.
- **S** — Projected target position.
- **α** — Interpolation factor.
- **C_new** — Updated crosshair position.

For example, with an interpolation factor of 0.1:

\[
C_{new}=(960,540)+(160,-120)\cdot0.1
\]

\[
\boxed{C_{new}=(976,528)}
\]

This produces a gradual transition toward the target rather than an immediate snap.

### Frame-Rate Independent Interpolation

For consistent interpolation behavior across different frame rates, a time-dependent interpolation factor can be calculated using:

\[
\alpha=1-e^{-k\Delta t}
\]

Where:

- **k** — Interpolation speed.
- **Δt** — Frame delta time.

This approach provides more consistent smoothing regardless of frame rate.

---

## System Workflow

The general targeting pipeline follows these steps:

1. Retrieve the selected target's world-space position.
2. Project the target position into screen coordinates.
3. Obtain the current crosshair position.
4. Calculate the screen-space displacement vector.
5. Determine the interpolation factor.
6. Calculate the updated aim position.
7. Apply the resulting adjustment through the appropriate gameplay or camera system.

---

## Core Technologies

| Technology | Purpose |
|---|---|
| C++ | Core implementation |
| Unreal Engine | Gameplay framework |
| World-to-Screen Projection | Coordinate transformation |
| Vector Mathematics | Screen-space displacement |
| Linear Interpolation | Smooth aim adjustment |
| Delta Time | Frame-rate independent smoothing |
| Skeletal Transforms | Target position acquisition |

## Potential Applications

The underlying mathematical techniques can be extended to several gameplay systems:

- Aim assistance and targeting mechanics.
- Dynamic camera tracking.
- Target-following systems.
- Lock-on camera mechanics.
- Screen-space interaction systems.
- Automated camera positioning.
- Real-time coordinate transformation.

## Project Structure

```text
AutoAimAssist/
│
├── Source/
│   ├── AimHackComponent.h
│   └── AimHackComponent.cpp
│
└── README.md
```

*The structure may evolve as the component develops.*

## Development Philosophy

AutoAimAssist is an exploration of the relationship between mathematical concepts and practical game programming.

The objective is not simply to produce a targeting component, but to understand and implement the mathematical foundations behind real-time coordinate transformations, vector operations, and smooth interpolation.

The project emphasizes:

- Mathematical correctness.
- Modular C++ architecture.
- Real-time performance.
- Reusable gameplay functionality.
- Clear and maintainable implementation.

---

## Author

**JSyther**

C++ Programmer | Unreal Engine Developer | Gameplay Systems Engineering

**Project:** AutoAimAssist  
**Language:** C++  
**Platform:** Unreal Engine

---

*Built through mathematics, implemented with C++.*
