#include "renderer.h"
#include "orbit_integrator.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "Orrery Renderer (C++ backend)\n";
    std::cout << "=============================\n";

    // Sun + 8 planets: position (m), velocity (m/s), mass (kg)
    std::vector<BodyState> bodies = {
        {{0, 0, 0},            {0, 0, 0},           1.989e30},  // Sun
        {{5.791e10, 0, 0},     {0, 4.787e4, 0},     3.301e23},  // Mercury
        {{1.082e11, 0, 0},     {0, 3.502e4, 0},     4.867e24},  // Venus
        {{1.496e11, 0, 0},     {0, 2.978e4, 0},     5.972e24},  // Earth
        {{2.279e11, 0, 0},     {0, 2.407e4, 0},     6.417e23},  // Mars
        {{7.785e11, 0, 0},     {0, 1.307e4, 0},     1.898e27},  // Jupiter
        {{1.4335e12, 0, 0},    {0, 9.690e3, 0},     5.683e26},  // Saturn
        {{2.8725e12, 0, 0},    {0, 6.810e3, 0},     8.681e25},  // Uranus
        {{4.4951e12, 0, 0},    {0, 5.430e3, 0},     1.024e26},  // Neptune
    };

    const char* names[] = {
        "Sun", "Mercury", "Venus", "Earth", "Mars",
        "Jupiter", "Saturn", "Uranus", "Neptune"
    };
    constexpr int N = 9;

    std::vector<std::vector<Vec3f>> trails(N);
    OrbitIntegrator integrator;
    double dt = 3600.0;
    int total_steps = 8760 * 5; // 5 years for outer planet arcs
    int record_interval = 48;

    std::cout << "Simulating " << total_steps << " steps (5 years)...\n";
    for (int step = 0; step < total_steps; ++step) {
        integrator.step_leapfrog(bodies, dt);
        if (step % record_interval == 0) {
            for (int i = 0; i < N; ++i) {
                trails[i].push_back(bodies[i].position);
            }
        }
    }

    for (int i = 0; i < N; ++i) {
        auto& p = bodies[i].position;
        std::cout << names[i] << ": ("
                  << p.x << ", " << p.y << ", " << p.z << ")\n";
    }

    // max_radius = Neptune's orbit (~4.5e12 m)
    Renderer renderer(1920, 1080, 5.0e12);

    std::vector<RenderBody> render_bodies = {
        {"Sun",     bodies[0].position, 20, {255, 230, 80},  100, {200, 140, 30}, trails[0]},
        {"Mercury", bodies[1].position,  3, {180, 180, 180},   8, {100, 100, 100}, trails[1]},
        {"Venus",   bodies[2].position,  5, {230, 200, 140},  12, {140, 120, 70},  trails[2]},
        {"Earth",   bodies[3].position,  6, {70, 140, 240},   14, {30, 70, 140},   trails[3]},
        {"Mars",    bodies[4].position,  4, {220, 100, 50},   10, {130, 55, 25},   trails[4]},
        {"Jupiter", bodies[5].position, 12, {210, 180, 110},  28, {110, 90, 45},   trails[5]},
        {"Saturn",  bodies[6].position, 10, {220, 200, 150},  26, {130, 110, 60},  trails[6]},
        {"Uranus",  bodies[7].position,  7, {150, 220, 230},  18, {70, 120, 130},  trails[7]},
        {"Neptune", bodies[8].position,  7, {60, 100, 220},   18, {30, 50, 130},   trails[8]},
    };

    renderer.render_frame(render_bodies);
    renderer.write_ppm("orrery_output.ppm");
    std::cout << "Frame written to orrery_output.ppm (1920x1080)\n";

    return 0;
}
