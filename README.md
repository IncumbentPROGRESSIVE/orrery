# Orrery

A multi-language celestial mechanics simulation engine.

## Architecture

- **Rust** (`rust/`) — Core simulation engine: n-body gravity, orbital mechanics, RK4/Verlet integrators
- **Kotlin** (`kotlin/`) — Visualization layer: ASCII rendering, orbit tracing, simulation stats
- **C++** (`cpp/`) — High-performance renderer: PPM frame output, leapfrog integrator

## Building

### Rust
```bash
cd rust && cargo build --release
```

### Kotlin
```bash
cd kotlin && ./gradlew build
```

### C++
```bash
cd cpp && cmake -B build && cmake --build build
```
