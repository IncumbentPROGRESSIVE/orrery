#include "renderer.h"
#include "orbit_integrator.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <filesystem>

static constexpr double G_CONST = 6.674e-11;
static constexpr double M_SUN = 1.989e30;
static constexpr double PI = 3.14159265358979323846;
static constexpr double DEG = PI / 180.0;
static constexpr double AU = 1.496e11;

struct OrbitalElements {
    double a, e, i, omega, Omega, M0, mass;
};

static double kepler(double M, double e) {
    double E = M;
    for (int iter = 0; iter < 50; ++iter) {
        double dE = (E - e * std::sin(E) - M) / (1.0 - e * std::cos(E));
        E -= dE;
        if (std::abs(dE) < 1e-12) break;
    }
    return E;
}

static std::pair<Vec3f, Vec3f> elements_to_cartesian(const OrbitalElements& oe) {
    double E = kepler(oe.M0, oe.e);
    double cosE = std::cos(E), sinE = std::sin(E);
    double nu = std::atan2(std::sqrt(1.0 - oe.e * oe.e) * sinE, cosE - oe.e);
    double r = oe.a * (1.0 - oe.e * cosE);

    double x_orb = r * std::cos(nu), y_orb = r * std::sin(nu);
    double mu = G_CONST * M_SUN;
    double p = oe.a * (1.0 - oe.e * oe.e);
    double h = std::sqrt(mu * p);
    double vx_orb = -mu / h * std::sin(nu);
    double vy_orb =  mu / h * (oe.e + std::cos(nu));

    double cO = std::cos(oe.Omega), sO = std::sin(oe.Omega);
    double cw = std::cos(oe.omega), sw = std::sin(oe.omega);
    double ci = std::cos(oe.i),     si = std::sin(oe.i);
    double Px = cO*cw - sO*sw*ci, Qx = -cO*sw - sO*cw*ci;
    double Py = sO*cw + cO*sw*ci, Qy = -sO*sw + cO*cw*ci;
    double Pz = sw*si,             Qz = cw*si;

    Vec3f pos{ x_orb*Px + y_orb*Qx, x_orb*Py + y_orb*Qy, x_orb*Pz + y_orb*Qz };
    Vec3f vel{ vx_orb*Px + vy_orb*Qx, vx_orb*Py + vy_orb*Qy, vx_orb*Pz + vy_orb*Qz };
    return {pos, vel};
}

// Generate asteroid belt particles (static positions scattered between Mars and Jupiter)
static std::vector<Particle> generate_asteroid_belt(uint32_t seed, int count) {
    std::vector<Particle> particles;
    uint32_t state = seed;
    auto rng = [&]() -> uint32_t { state = state * 1664525u + 1013904223u; return state; };

    for (int i = 0; i < count; ++i) {
        // Semi-major axis between 2.1 and 3.3 AU
        double a = (2.1 + (rng() % 10000) / 10000.0 * 1.2) * AU;
        double e = (rng() % 1000) / 10000.0; // 0 to 0.1
        double angle = (rng() % 36000) / 100.0 * DEG;
        double r = a * (1.0 - e * e) / (1.0 + e * std::cos(angle));
        double x = r * std::cos(angle);
        double y = r * std::sin(angle);
        double z = ((rng() % 2000) - 1000) / 1000.0 * a * 0.05; // slight z scatter

        uint8_t br = 40 + (rng() % 50);
        particles.push_back({{x, y, z}, {br, br, static_cast<uint8_t>(br - 10)}});
    }
    return particles;
}

int main() {
    std::cout << "Orrery Renderer (C++ backend)\n";
    std::cout << "=============================\n";

    // J2000 orbital elements
    OrbitalElements planet_elements[] = {
        {5.7909e10,  0.2056, 7.005*DEG,  29.124*DEG,  48.331*DEG, 174.796*DEG, 3.301e23}, // Mercury
        {1.0821e11,  0.0068, 3.394*DEG,  54.884*DEG,  76.680*DEG,  50.115*DEG, 4.867e24}, // Venus
        {1.4960e11,  0.0167, 0.000*DEG, 114.208*DEG, -11.261*DEG, 357.517*DEG, 5.972e24}, // Earth
        {2.2794e11,  0.0934, 1.850*DEG, 286.502*DEG,  49.558*DEG,  19.373*DEG, 6.417e23}, // Mars
        {7.7857e11,  0.0489, 1.303*DEG, 273.867*DEG, 100.464*DEG,  20.020*DEG, 1.898e27}, // Jupiter
        {1.4335e12,  0.0565, 2.485*DEG, 339.392*DEG, 113.665*DEG, 317.020*DEG, 5.683e26}, // Saturn
        {2.8725e12,  0.0457, 0.773*DEG,  96.998*DEG,  74.006*DEG, 142.238*DEG, 8.681e25}, // Uranus
        {4.4951e12,  0.0113, 1.770*DEG, 276.336*DEG, 131.784*DEG, 256.228*DEG, 1.024e26}, // Neptune
    };

    // Halley's comet
    OrbitalElements halley = {
        2.682e12, 0.96714, 162.26*DEG, 111.33*DEG, 58.42*DEG, 38.38*DEG, 2.2e14
    };

    const char* planet_names[] = { "Mercury", "Venus", "Earth", "Mars", "Jupiter", "Saturn", "Uranus", "Neptune" };

    // Build body list: Sun + 8 planets + Halley
    std::vector<BodyState> bodies;
    bodies.push_back({{0,0,0}, {0,0,0}, M_SUN});
    for (auto& oe : planet_elements) {
        auto [pos, vel] = elements_to_cartesian(oe);
        bodies.push_back({pos, vel, oe.mass});
    }
    {
        auto [pos, vel] = elements_to_cartesian(halley);
        bodies.push_back({pos, vel, halley.mass});
    }

    constexpr int N_BODIES = 10; // Sun + 8 planets + Halley
    const char* all_names[] = { "Sun", "Mercury", "Venus", "Earth", "Mars", "Jupiter", "Saturn", "Uranus", "Neptune", "Halley" };

    // Orbit guides for all planets + Halley
    std::vector<OrbitGuide> orbit_guides;
    Color orbit_colors[] = {
        {80,80,80}, {100,90,60}, {50,80,130}, {110,60,30},
        {100,85,50}, {100,90,65}, {60,100,105}, {35,55,110}
    };
    for (int i = 0; i < 8; ++i) {
        auto& oe = planet_elements[i];
        orbit_guides.push_back({oe.a, oe.e, oe.i, oe.omega, oe.Omega, orbit_colors[i]});
    }
    orbit_guides.push_back({halley.a, halley.e, halley.i, halley.omega, halley.Omega, {60, 80, 60}});

    // Asteroid belt
    auto asteroids = generate_asteroid_belt(12345, 800);

    // Simulation: record trails + output animation frames
    std::vector<std::vector<Vec3f>> trails(N_BODIES);
    OrbitIntegrator integrator;
    double dt = 3600.0;
    int total_steps = 8760 * 8; // 8 years (Halley needs time to show arc)
    int record_interval = 72;
    int frame_interval = 8760 / 4; // ~4 frames per year = 32 frames total
    int frame_count = 0;

    std::filesystem::create_directories("frames");

    Renderer renderer(1920, 1080, 5.5e12);

    // Legend
    std::vector<LegendEntry> legend = {
        {"Sun",     {255,230,80},  ""},
        {"Mercury", {180,180,180}, "0.39 AU"},
        {"Venus",   {230,200,140}, "0.72 AU"},
        {"Earth",   {70,140,240},  "1.00 AU"},
        {"Mars",    {220,100,50},  "1.52 AU"},
        {"Jupiter", {210,180,110}, "5.20 AU"},
        {"Saturn",  {220,200,150}, "9.58 AU"},
        {"Uranus",  {150,220,230}, "19.2 AU"},
        {"Neptune", {60,100,220},  "30.1 AU"},
        {"Halley",  {180,220,180}, "e=0.967"},
    };

    auto build_render_bodies = [&]() -> std::vector<RenderBody> {
        std::vector<RenderBody> rb = {
            {"Sun",     bodies[0].position, 20, {255,230,80},  100, {200,140,30}, trails[0]},
            {"Mercury", bodies[1].position,  3, {180,180,180},   8, {100,100,100}, trails[1]},
            {"Venus",   bodies[2].position,  5, {230,200,140},  12, {140,120,70},  trails[2]},
            {"Earth",   bodies[3].position,  6, {70,140,240},   14, {30,70,140},   trails[3]},
            {"Mars",    bodies[4].position,  4, {220,100,50},   10, {130,55,25},   trails[4]},
            {"Jupiter", bodies[5].position, 12, {210,180,110},  28, {110,90,45},   trails[5]},
            {"Saturn",  bodies[6].position, 10, {220,200,150},  26, {130,110,60},  trails[6]},
            {"Uranus",  bodies[7].position,  7, {150,220,230},  18, {70,120,130},  trails[7]},
            {"Neptune", bodies[8].position,  7, {60,100,220},   18, {30,50,130},   trails[8]},
            {"Halley",  bodies[9].position,  3, {180,220,180},   6, {80,120,80},   trails[9]},
        };
        // Saturn rings
        rb[6].has_ring = true;
        rb[6].ring_inner = 14;
        rb[6].ring_outer = 24;
        rb[6].ring_color = {200, 180, 130};
        // Halley comet tail
        rb[9].is_comet = true;
        rb[9].tail_length = 40;
        return rb;
    };

    std::cout << "Simulating " << total_steps << " steps (8 years)...\n";
    for (int step = 0; step < total_steps; ++step) {
        integrator.step_leapfrog(bodies, dt);

        if (step % record_interval == 0) {
            for (int i = 0; i < N_BODIES; ++i)
                trails[i].push_back(bodies[i].position);
        }

        if (step > 0 && step % frame_interval == 0) {
            auto rb = build_render_bodies();
            renderer.render_frame(rb, orbit_guides, asteroids, legend);
            std::ostringstream fname;
            fname << "frames/frame_" << std::setw(4) << std::setfill('0') << frame_count << ".ppm";
            renderer.write_ppm(fname.str());
            frame_count++;
            std::cout << "  Frame " << frame_count << " written (step " << step << ")\n";
        }
    }

    // Final frame
    auto rb = build_render_bodies();
    renderer.render_frame(rb, orbit_guides, asteroids, legend);
    renderer.write_ppm("orrery_output.ppm");
    std::cout << "\nFinal frame: orrery_output.ppm\n";
    std::cout << "Animation frames: frames/frame_0000.ppm - frame_"
              << std::setw(4) << std::setfill('0') << frame_count - 1 << ".ppm\n";

    for (int i = 0; i < N_BODIES; ++i) {
        auto& p = bodies[i].position;
        double au = p.magnitude() / AU;
        std::cout << all_names[i] << ": " << au << " AU\n";
    }

    return 0;
}
