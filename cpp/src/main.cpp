#include "renderer.h"
#include "orbit_integrator.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "Orrery Renderer (C++ backend)\n";
    std::cout << "=============================\n";

    std::vector<BodyState> bodies = {
        {{0, 0, 0}, {0, 0, 0}, 1.989e30},
        {{1.496e11, 0, 0}, {0, 2.978e4, 0}, 5.972e24},
        {{2.279e11, 0, 0}, {0, 2.407e4, 0}, 6.417e23},
        {{7.785e11, 0, 0}, {0, 1.307e4, 0}, 1.898e27},
    };

    OrbitIntegrator integrator;
    double dt = 3600.0;
    int steps = 8760;

    std::cout << "Simulating " << steps << " steps (dt=" << dt << "s)...\n";
    for (int i = 0; i < steps; ++i) {
        integrator.step_leapfrog(bodies, dt);
    }

    const char* names[] = {"Sun", "Earth", "Mars", "Jupiter"};
    for (size_t i = 0; i < bodies.size(); ++i) {
        auto& p = bodies[i].position;
        std::cout << names[i] << ": ("
                  << p.x << ", " << p.y << ", " << p.z << ")\n";
    }

    Renderer renderer(800, 600, 5e-10);
    std::vector<RenderBody> render_bodies;
    float colors[][3] = {{1,1,0}, {0,0.5,1}, {1,0.3,0}, {0.8,0.6,0.2}};
    double radii[] = {6.957e8, 6.371e6, 3.389e6, 6.991e7};
    for (size_t i = 0; i < bodies.size(); ++i) {
        render_bodies.push_back({names[i], bodies[i].position, radii[i],
            {colors[i][0], colors[i][1], colors[i][2]}});
    }

    renderer.render_frame(render_bodies);
    renderer.write_ppm("orrery_output.ppm");
    std::cout << "Frame written to orrery_output.ppm\n";

    return 0;
}
