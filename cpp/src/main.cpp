#include "renderer.h"
#include "orbit_integrator.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "Orrery Renderer (C++ backend)\n";
    std::cout << "=============================\n";

    std::vector<BodyState> bodies = {
        {{0, 0, 0}, {0, 0, 0}, 1.989e30},           // Sun
        {{1.496e11, 0, 0}, {0, 2.978e4, 0}, 5.972e24}, // Earth
        {{2.279e11, 0, 0}, {0, 2.407e4, 0}, 6.417e23}, // Mars
        {{7.785e11, 0, 0}, {0, 1.307e4, 0}, 1.898e27}, // Jupiter
    };

    const char* names[] = {"Sun", "Earth", "Mars", "Jupiter"};
    constexpr int N = 4;

    // Record orbit trails during simulation
    std::vector<std::vector<Vec3f>> trails(N);
    OrbitIntegrator integrator;
    double dt = 3600.0;
    int total_steps = 8760 * 2; // 2 years
    int record_interval = 24;   // record once per day

    std::cout << "Simulating " << total_steps << " steps...\n";
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

    // Artistic display parameters (not physical radii)
    Renderer renderer(1920, 1080, 5e-10);

    std::vector<RenderBody> render_bodies = {
        {"Sun",     bodies[0].position, 22, {255, 230, 80},  120, {200, 140, 30}, trails[0]},
        {"Earth",   bodies[1].position,  7, {70, 140, 240},   20, {30, 70, 140},  trails[1]},
        {"Mars",    bodies[2].position,  6, {220, 100, 50},   16, {130, 55, 25},  trails[2]},
        {"Jupiter", bodies[3].position, 12, {210, 180, 110},  30, {110, 90, 45},  trails[3]},
    };

    renderer.render_frame(render_bodies);
    renderer.write_ppm("orrery_output.ppm");
    std::cout << "Frame written to orrery_output.ppm (1920x1080)\n";

    return 0;
}
