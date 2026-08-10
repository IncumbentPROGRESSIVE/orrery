# Orrery

A multi-language celestial mechanics simulation engine with a real-time interactive solar system renderer.

## Architecture

| Component | Language | Role |
|---|---|---|
| `cpp/` | C++ | Real-time SDL2 renderer, leapfrog integrator, full solar system |
| `rust/` | Rust | Headless n-body benchmark, RK4 integrator, energy drift analysis |
| `kotlin/` | Kotlin | ASCII terminal preview, orbit tracing, simulation stats |

The three implementations are independent. Only the C++ renderer is interactive.

## Project Structure

```
orrery/
├── cpp/
│   ├── src/
│   │   ├── main.cpp              # Simulation loop, bodies, input handling
│   │   ├── renderer.h / .cpp     # Software rasterizer, textures, HUD
│   │   └── orbit_integrator.h   # Leapfrog integrator, BodyState
│   └── CMakeLists.txt
├── rust/
│   └── src/main.rs              # RK4 simulation, energy drift benchmark
└── kotlin/
    └── src/Main.kt              # ASCII renderer, orbit tracer
```

## Solar System Bodies

### Planets
8 planets initialized from real orbital elements (semi-major axis, eccentricity, inclination, argument of periapsis, longitude of ascending node, mean anomaly). Initial Cartesian state vectors are computed via full Kepler equation solving (Newton's method, 50 iterations) and rotation matrices.

### Dwarf Planets
| Body | Semi-major axis | Eccentricity |
|---|---|---|
| Ceres | 2.77 AU | 0.076 |
| Pluto | 39.5 AU | 0.249 |
| Haumea | 43.3 AU | 0.159 |
| Makemake | 45.8 AU | 0.156 |
| Eris | 67.8 AU | 0.441 |
| Sedna | ~506 AU | 0.846 |
| Quaoar | 43.7 AU | 0.039 |

### Comets
- Halley's Comet — e = 0.967, i = 162°, rendered with a glowing tail
- Spawnable random comets (C key) — eccentric orbits, randomized inclination

### Moons
| Planet | Moons |
|---|---|
| Earth | Moon |
| Mars | Phobos, Deimos |
| Jupiter | Io, Europa, Ganymede, Callisto |
| Saturn | Enceladus, Rhea, Titan |
| Uranus | Miranda, Ariel, Umbriel, Titania, Oberon |
| Neptune | Triton (retrograde — negative period) |
| Pluto | Charon |

Moon labels are hidden below 8× zoom to avoid clutter.

### Small Bodies
- **Asteroid belt** — 600 bodies (2.1–3.3 AU), orbiting via Kepler's third law (`ω = √(GM/a³)`), with Kirkwood gaps at the 4:1, 3:1, 5:2, 7:3, and 2:1 Jupiter resonances
- **Jupiter Trojans** — 80 bodies at L4 (+60°), 80 at L5 (−60°) relative to Jupiter's current angle
- **Kuiper belt** — 400 icy bodies (30–50 AU), bluish tint

## Rendering

### Projection
Distances are compressed using a square-root warp so the outer solar system (Neptune at 30 AU, Sedna at 506 AU) remains visible alongside the inner planets:

```
warped_radius = √(r / max_radius) × pixel_radius
```

### Pipeline (9 layers, drawn in order)
1. Background — procedural nebula + starfield, cached at startup via `memcpy` each frame
2. Orbit guides — dotted ellipses sampled from orbital elements
3. Asteroid/Kuiper belt particles
4. Orbital trails — fade with age, adaptive recording interval scales with time speed
5. Comet tails — fan of additive-blended lines pointing away from the Sun
6. Glows — cubic-falloff additive glow around bright bodies
7. Rings — tilted ellipse with brightness variation (Saturn)
8. Bodies — procedural spherical textures with Lambertian shading, or flat AA circles
9. Labels + HUD — bitmap font, legend, speed/elapsed time overlay

### Procedural Textures
Each planet uses fBm (fractal Brownian motion) noise mapped onto a sphere with UV coordinates derived from the surface normal. Features include:
- Sun: granulation + bright spots
- Earth: continents, clouds, polar caps
- Jupiter: banded atmosphere + Great Red Spot
- Mars: terrain variation + polar caps
- Saturn/Uranus/Neptune: banded atmospheres

## Controls

| Key / Input | Action |
|---|---|
| `Space` | Pause / resume |
| `←` / `→` | Halve / double time speed (0.125× – 16×) |
| Scroll wheel | Zoom (0.2× – 32×) |
| Click + drag | Pan view |
| Double-click | Follow body (auto-zooms to 16×) |
| `R` | Reset zoom / pan / follow |
| `O` | Toggle orbit guides |
| `C` | Spawn random comet |
| `H` | Toggle help overlay |
| `Esc` | Quit |

## Building & Running

### C++ (main renderer)

Requires SDL2:
```bash
brew install sdl2   # macOS
```

```bash
cd cpp
cmake -B build && cmake --build build
./build/orrery_renderer
```

### Rust (physics benchmark)
Runs a 1-year RK4 simulation of the solar system and reports energy drift:
```bash
cd rust && cargo build --release && cargo run --release
```

### Kotlin (ASCII preview)
Renders a terminal snapshot of initial body positions:
```bash
cd kotlin && ./gradlew run
```

## Physics

- **Leapfrog integrator** — symplectic, used in the C++ real-time renderer (dt = 7200 s, 30 steps/frame)
- **RK4 integrator** — used in the Rust benchmark for energy drift analysis
- **Kepler equation** — Newton's method (50 iterations) to solve `E - e·sin(E) = M`
- **Orbital elements → state vectors** — via rotation matrices (Ω, ω, i) applied to the orbital plane
- **Asteroid angular velocity** — `ω = √(GM/a³)` (Kepler's third law)
