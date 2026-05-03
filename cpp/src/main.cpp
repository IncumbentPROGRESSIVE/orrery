#include "renderer.h"
#include "orbit_integrator.h"
#include <iostream>
#include <vector>
#include <cmath>

static constexpr double G = 6.674e-11;
static constexpr double M_SUN = 1.989e30;
static constexpr double PI = 3.14159265358979323846;
static constexpr double DEG = PI / 180.0;

struct OrbitalElements {
    double a;       // semi-major axis (m)
    double e;       // eccentricity
    double i;       // inclination (rad)
    double omega;   // argument of perihelion (rad)
    double Omega;   // longitude of ascending node (rad)
    double M0;      // mean anomaly at epoch (rad)
    double mass;    // body mass (kg)
};

// Solve Kepler's equation M = E - e*sin(E) via Newton iteration
static double kepler(double M, double e) {
    double E = M;
    for (int iter = 0; iter < 50; ++iter) {
        double dE = (E - e * std::sin(E) - M) / (1.0 - e * std::cos(E));
        E -= dE;
        if (std::abs(dE) < 1e-12) break;
    }
    return E;
}

// Convert orbital elements to Cartesian state (position, velocity)
static std::pair<Vec3f, Vec3f> elements_to_cartesian(const OrbitalElements& oe) {
    double E = kepler(oe.M0, oe.e);
    double cosE = std::cos(E), sinE = std::sin(E);

    // True anomaly
    double nu = std::atan2(std::sqrt(1.0 - oe.e * oe.e) * sinE, cosE - oe.e);

    // Distance
    double r = oe.a * (1.0 - oe.e * cosE);

    // Position and velocity in orbital plane
    double x_orb = r * std::cos(nu);
    double y_orb = r * std::sin(nu);

    double mu = G * M_SUN;
    double p = oe.a * (1.0 - oe.e * oe.e);
    double h = std::sqrt(mu * p);
    double vx_orb = -mu / h * std::sin(nu);
    double vy_orb =  mu / h * (oe.e + std::cos(nu));

    // Rotation matrix components
    double cO = std::cos(oe.Omega), sO = std::sin(oe.Omega);
    double cw = std::cos(oe.omega), sw = std::sin(oe.omega);
    double ci = std::cos(oe.i),     si = std::sin(oe.i);

    double Px = cO*cw - sO*sw*ci,  Qx = -cO*sw - sO*cw*ci;
    double Py = sO*cw + cO*sw*ci,  Qy = -sO*sw + cO*cw*ci;
    double Pz = sw*si,              Qz = cw*si;

    Vec3f pos{
        x_orb * Px + y_orb * Qx,
        x_orb * Py + y_orb * Qy,
        x_orb * Pz + y_orb * Qz
    };
    Vec3f vel{
        vx_orb * Px + vy_orb * Qx,
        vx_orb * Py + vy_orb * Qy,
        vx_orb * Pz + vy_orb * Qz
    };
    return {pos, vel};
}

int main() {
    std::cout << "Orrery Renderer (C++ backend)\n";
    std::cout << "=============================\n";

    // J2000 orbital elements (realistic eccentricities, inclinations, phases)
    //                          a (m)          e       i          omega      Omega      M0         mass (kg)
    OrbitalElements elements[] = {
        {5.7909e10,  0.2056, 7.005*DEG,  29.124*DEG,  48.331*DEG, 174.796*DEG, 3.301e23}, // Mercury
        {1.0821e11,  0.0068, 3.394*DEG,  54.884*DEG,  76.680*DEG,  50.115*DEG, 4.867e24}, // Venus
        {1.4960e11,  0.0167, 0.000*DEG, 114.208*DEG, -11.261*DEG, 357.517*DEG, 5.972e24}, // Earth
        {2.2794e11,  0.0934, 1.850*DEG, 286.502*DEG,  49.558*DEG,  19.373*DEG, 6.417e23}, // Mars
        {7.7857e11,  0.0489, 1.303*DEG, 273.867*DEG, 100.464*DEG,  20.020*DEG, 1.898e27}, // Jupiter
        {1.4335e12,  0.0565, 2.485*DEG, 339.392*DEG, 113.665*DEG, 317.020*DEG, 5.683e26}, // Saturn
        {2.8725e12,  0.0457, 0.773*DEG,  96.998*DEG,  74.006*DEG, 142.238*DEG, 8.681e25}, // Uranus
        {4.4951e12,  0.0113, 1.770*DEG, 276.336*DEG, 131.784*DEG, 256.228*DEG, 1.024e26}, // Neptune
    };

    const char* names[] = {
        "Sun", "Mercury", "Venus", "Earth", "Mars",
        "Jupiter", "Saturn", "Uranus", "Neptune"
    };
    constexpr int N = 9;

    std::vector<BodyState> bodies;
    bodies.push_back({{0, 0, 0}, {0, 0, 0}, M_SUN}); // Sun at origin

    for (auto& oe : elements) {
        auto [pos, vel] = elements_to_cartesian(oe);
        bodies.push_back({pos, vel, oe.mass});
    }

    std::vector<std::vector<Vec3f>> trails(N);
    OrbitIntegrator integrator;
    double dt = 3600.0;
    int total_steps = 8760 * 5;
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
