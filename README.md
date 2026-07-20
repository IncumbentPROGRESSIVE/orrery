# Orrery

A multi-language celestial mechanics simulation engine with a real-time interactive solar system renderer.

## Architecture

- **C++** (`cpp/`) — Primary renderer: SDL2 real-time display, leapfrog integrator, full solar system
- **Rust** (`rust/`) — Headless physics benchmark: n-body gravity, RK4/Verlet integrators, energy drift analysis
- **Kotlin** (`kotlin/`) — ASCII terminal preview: orbit tracing, simulation stats

## Features

### Solar System Bodies
- 8 planets with real orbital elements (Mercury → Neptune)
- 6 dwarf planets: Ceres, Pluto, Eris, Haumea, Makemake, Sedna (506 AU semi-major axis)
- Halley's Comet with accurate eccentricity (e = 0.967)

### Moons
- Earth: Moon
- Mars: Phobos, Deimos
- Jupiter: Io, Europa, Ganymede, Callisto
- Saturn: Enceladus, Rhea, Titan
- Uranus: Miranda, Ariel, Umbriel, Titania, Oberon
- Neptune: Triton (retrograde orbit)
- Moon labels appear at zoom ≥ 8×

### Small Bodies
- Asteroid belt (600 bodies, 2.1–3.3 AU) with Kirkwood gaps at 4:1, 3:1, 5:2, 7:3, 2:1 Jupiter resonances
- Jupiter Trojans: 80 at L4, 80 at L5 Lagrange points
- Kuiper belt: 400 icy bodies (30–50 AU, bluish tint)
- Spawnable comets with glowing tails (C key)

### Rendering
- Procedural planet textures with rotation
- Saturn's rings
- Comet tails
- Orbit guide lines (toggleable)
- Procedural nebula + starfield background (cached for performance)
- Orbital trails with adaptive recording interval

### Controls
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

### HUD
- Simulated time elapsed (days / years)
- Current time scale
- Body legend with orbital distances

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
```bash
cd rust && cargo build --release && cargo run --release
```

### Kotlin (ASCII preview)
```bash
cd kotlin && ./gradlew run
```

## Physics

- Leapfrog integrator (C++ renderer)
- RK4 and Verlet integrators (Rust benchmark)
- Kepler's third law for asteroid belt angular velocity: `ω = √(GM/a³)`
- Full Kepler equation solver (Newton's method, 50 iterations) for initial conditions
- Orbital elements → Cartesian state vectors via rotation matrices
