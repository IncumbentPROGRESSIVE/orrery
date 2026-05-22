#include "renderer.h"
#include "orbit_integrator.h"
#include <SDL.h>
#include <iostream>
#include <vector>
#include <cmath>

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

struct Asteroid {
    double a, e, angle;
    Color color;
};

static std::vector<Asteroid> generate_asteroid_belt(uint32_t seed, int count) {
    std::vector<Asteroid> asteroids;
    uint32_t state = seed;
    auto rng = [&]() -> uint32_t { state = state * 1664525u + 1013904223u; return state; };
    for (int i = 0; i < count; ++i) {
        double a = (2.1 + (rng() % 10000) / 10000.0 * 1.2) * AU;
        double e = (rng() % 1000) / 10000.0;
        double angle = (rng() % 36000) / 100.0 * DEG;
        uint8_t br = 40 + (rng() % 50);
        asteroids.push_back({a, e, angle, {br, br, static_cast<uint8_t>(br - 10)}});
    }
    return asteroids;
}

static void advance_asteroids(std::vector<Asteroid>& asteroids, double dt) {
    // Kepler's third law: angular velocity = sqrt(G*M_sun / a^3)
    for (auto& ast : asteroids) {
        double omega = std::sqrt(G_CONST * M_SUN / (ast.a * ast.a * ast.a));
        ast.angle += omega * dt;
    }
}

static std::vector<Particle> asteroids_to_particles(const std::vector<Asteroid>& asteroids) {
    std::vector<Particle> particles;
    particles.reserve(asteroids.size());
    for (const auto& ast : asteroids) {
        double r = ast.a * (1.0 - ast.e * ast.e) / (1.0 + ast.e * std::cos(ast.angle));
        particles.push_back({{r * std::cos(ast.angle), r * std::sin(ast.angle), 0}, ast.color});
    }
    return particles;
}

int main(int argc, char* argv[]) {
    constexpr int W = 1280, H = 720;

    // Init simulation
    OrbitalElements planet_elements[] = {
        {5.7909e10,  0.2056, 7.005*DEG,  29.124*DEG,  48.331*DEG, 174.796*DEG, 3.301e23},
        {1.0821e11,  0.0068, 3.394*DEG,  54.884*DEG,  76.680*DEG,  50.115*DEG, 4.867e24},
        {1.4960e11,  0.0167, 0.000*DEG, 114.208*DEG, -11.261*DEG, 357.517*DEG, 5.972e24},
        {2.2794e11,  0.0934, 1.850*DEG, 286.502*DEG,  49.558*DEG,  19.373*DEG, 6.417e23},
        {7.7857e11,  0.0489, 1.303*DEG, 273.867*DEG, 100.464*DEG,  20.020*DEG, 1.898e27},
        {1.4335e12,  0.0565, 2.485*DEG, 339.392*DEG, 113.665*DEG, 317.020*DEG, 5.683e26},
        {2.8725e12,  0.0457, 0.773*DEG,  96.998*DEG,  74.006*DEG, 142.238*DEG, 8.681e25},
        {4.4951e12,  0.0113, 1.770*DEG, 276.336*DEG, 131.784*DEG, 256.228*DEG, 1.024e26},
    };
    OrbitalElements halley = {2.682e12, 0.96714, 162.26*DEG, 111.33*DEG, 58.42*DEG, 38.38*DEG, 2.2e14};

    std::vector<BodyState> bodies;
    bodies.push_back({{0,0,0}, {0,0,0}, M_SUN});
    for (auto& oe : planet_elements) {
        auto [pos, vel] = elements_to_cartesian(oe);
        bodies.push_back({pos, vel, oe.mass});
    }
    { auto [pos, vel] = elements_to_cartesian(halley); bodies.push_back({pos, vel, halley.mass}); }

    constexpr int N = 10;

    // Orbit guides
    std::vector<OrbitGuide> orbit_guides;
    Color orbit_colors[] = {{60,60,60},{80,70,50},{40,60,100},{90,50,25},{80,65,40},{80,70,50},{50,80,85},{30,45,90}};
    for (int i = 0; i < 8; ++i) {
        auto& oe = planet_elements[i];
        orbit_guides.push_back({oe.a, oe.e, oe.i, oe.omega, oe.Omega, orbit_colors[i]});
    }
    orbit_guides.push_back({halley.a, halley.e, halley.i, halley.omega, halley.Omega, {40,60,40}});

    auto asteroids = generate_asteroid_belt(12345, 600);
    double asteroid_time_acc = 0.0;
    std::vector<std::vector<Vec3f>> trails(N);
    OrbitIntegrator integrator;

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

    Renderer renderer(W, H, 5.5e12);

    // SDL init
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Orrery",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, W, H, 0);
    SDL_Renderer* sdl_renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_Texture* texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_RGB24,
        SDL_TEXTUREACCESS_STREAMING, W, H);

    double dt = 7200.0;
    int steps_per_frame = 30;
    int trail_record = 0;
    double rotation_phase = 0.0;
    bool running = true;

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) running = false;
        }

        // Advance simulation
        for (int s = 0; s < steps_per_frame; ++s)
            integrator.step_leapfrog(bodies, dt);

        // Record trails every few frames
        trail_record++;
        if (trail_record % 2 == 0) {
            for (int i = 0; i < N; ++i) {
                trails[i].push_back(bodies[i].position);
                // Inner planets (Mercury-Mars): 300 points is fine
                // Outer planets (Jupiter+) and Halley: keep much longer trails
                size_t max_trail = (i >= 5) ? 3000 : 300;
                if (trails[i].size() > max_trail) trails[i].erase(trails[i].begin());
            }
        }

        // Advance asteroids
        asteroid_time_acc += dt * steps_per_frame;
        advance_asteroids(asteroids, dt * steps_per_frame);

        // Advance rotation phase
        rotation_phase += 0.02;

        // Build render bodies
        std::vector<RenderBody> rb = {
            {"Sun",     bodies[0].position, 16, {255,160,40},  90, {255,120,20}, trails[0], false, 0, 0, {}, false, 0, TextureType::SUN, rotation_phase},
            {"Mercury", bodies[1].position,  3, {180,180,180},  6, {80,80,80}, trails[1], false, 0, 0, {}, false, 0, TextureType::MERCURY, rotation_phase * 0.017},
            {"Venus",   bodies[2].position,  4, {230,200,140}, 10, {120,100,60}, trails[2], false, 0, 0, {}, false, 0, TextureType::VENUS, rotation_phase * 0.004},
            {"Earth",   bodies[3].position,  5, {70,140,240},  12, {30,60,120}, trails[3], false, 0, 0, {}, false, 0, TextureType::EARTH, rotation_phase},
            {"Mars",    bodies[4].position,  4, {220,100,50},   8, {110,50,20}, trails[4], false, 0, 0, {}, false, 0, TextureType::MARS, rotation_phase * 0.97},
            {"Jupiter", bodies[5].position, 10, {210,180,110}, 22, {100,80,40}, trails[5], false, 0, 0, {}, false, 0, TextureType::JUPITER, rotation_phase * 2.4},
            {"Saturn",  bodies[6].position,  8, {220,200,150}, 20, {110,95,50}, trails[6], false, 0, 0, {}, false, 0, TextureType::SATURN, rotation_phase * 2.3},
            {"Uranus",  bodies[7].position,  6, {150,220,230}, 14, {60,100,110}, trails[7], false, 0, 0, {}, false, 0, TextureType::URANUS, rotation_phase * 1.4},
            {"Neptune", bodies[8].position,  6, {60,100,220},  14, {25,45,100}, trails[8], false, 0, 0, {}, false, 0, TextureType::NEPTUNE, rotation_phase * 1.5},
            {"Halley",  bodies[9].position,  2, {180,220,180},  4, {60,90,60}, trails[9], false, 0, 0, {}, false, 0, TextureType::COMET, 0},
        };
        rb[6].has_ring = true; rb[6].ring_inner = 12; rb[6].ring_outer = 20; rb[6].ring_color = {200,180,130};
        rb[9].is_comet = true; rb[9].tail_length = 30;

        // Render
        auto asteroid_particles = asteroids_to_particles(asteroids);
        renderer.render_frame(rb, orbit_guides, asteroid_particles, legend);

        // Blit to SDL texture
        void* pixels; int pitch;
        SDL_LockTexture(texture, nullptr, &pixels, &pitch);
        const uint8_t* fb = renderer.framebuffer();
        for (int y = 0; y < H; ++y)
            memcpy(static_cast<uint8_t*>(pixels) + y * pitch, fb + y * W * 3, W * 3);
        SDL_UnlockTexture(texture);

        SDL_RenderCopy(sdl_renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(sdl_renderer);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(sdl_renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
