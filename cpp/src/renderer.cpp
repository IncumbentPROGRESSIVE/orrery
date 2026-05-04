#include "renderer.h"
#include <fstream>
#include <algorithm>
#include <cstring>

static constexpr double PI = 3.14159265358979323846;

static const uint8_t FONT_5X7[][7] = {
    ['A'] = {0x0E,0x11,0x11,0x1F,0x11,0x11,0x11},
    ['B'] = {0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E},
    ['C'] = {0x0E,0x11,0x10,0x10,0x10,0x11,0x0E},
    ['D'] = {0x1E,0x11,0x11,0x11,0x11,0x11,0x1E},
    ['E'] = {0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F},
    ['F'] = {0x1F,0x10,0x10,0x1E,0x10,0x10,0x10},
    ['G'] = {0x0E,0x11,0x10,0x17,0x11,0x11,0x0E},
    ['H'] = {0x11,0x11,0x11,0x1F,0x11,0x11,0x11},
    ['I'] = {0x0E,0x04,0x04,0x04,0x04,0x04,0x0E},
    ['J'] = {0x07,0x02,0x02,0x02,0x02,0x12,0x0C},
    ['K'] = {0x11,0x12,0x14,0x18,0x14,0x12,0x11},
    ['L'] = {0x10,0x10,0x10,0x10,0x10,0x10,0x1F},
    ['M'] = {0x11,0x1B,0x15,0x15,0x11,0x11,0x11},
    ['N'] = {0x11,0x19,0x15,0x13,0x11,0x11,0x11},
    ['O'] = {0x0E,0x11,0x11,0x11,0x11,0x11,0x0E},
    ['P'] = {0x1E,0x11,0x11,0x1E,0x10,0x10,0x10},
    ['Q'] = {0x0E,0x11,0x11,0x11,0x15,0x12,0x0D},
    ['R'] = {0x1E,0x11,0x11,0x1E,0x14,0x12,0x11},
    ['S'] = {0x0E,0x11,0x10,0x0E,0x01,0x11,0x0E},
    ['T'] = {0x1F,0x04,0x04,0x04,0x04,0x04,0x04},
    ['U'] = {0x11,0x11,0x11,0x11,0x11,0x11,0x0E},
    ['V'] = {0x11,0x11,0x11,0x11,0x0A,0x0A,0x04},
    ['W'] = {0x11,0x11,0x11,0x15,0x15,0x1B,0x11},
    ['X'] = {0x11,0x11,0x0A,0x04,0x0A,0x11,0x11},
    ['Y'] = {0x11,0x11,0x0A,0x04,0x04,0x04,0x04},
    ['Z'] = {0x1F,0x01,0x02,0x04,0x08,0x10,0x1F},
    ['a'] = {0x00,0x00,0x0E,0x01,0x0F,0x11,0x0F},
    ['b'] = {0x10,0x10,0x1E,0x11,0x11,0x11,0x1E},
    ['c'] = {0x00,0x00,0x0E,0x11,0x10,0x11,0x0E},
    ['d'] = {0x01,0x01,0x0F,0x11,0x11,0x11,0x0F},
    ['e'] = {0x00,0x00,0x0E,0x11,0x1F,0x10,0x0E},
    ['f'] = {0x06,0x08,0x1E,0x08,0x08,0x08,0x08},
    ['g'] = {0x00,0x00,0x0F,0x11,0x0F,0x01,0x0E},
    ['h'] = {0x10,0x10,0x1E,0x11,0x11,0x11,0x11},
    ['i'] = {0x04,0x00,0x0C,0x04,0x04,0x04,0x0E},
    ['j'] = {0x02,0x00,0x06,0x02,0x02,0x12,0x0C},
    ['k'] = {0x10,0x10,0x12,0x14,0x18,0x14,0x12},
    ['l'] = {0x0C,0x04,0x04,0x04,0x04,0x04,0x0E},
    ['m'] = {0x00,0x00,0x1A,0x15,0x15,0x11,0x11},
    ['n'] = {0x00,0x00,0x1E,0x11,0x11,0x11,0x11},
    ['o'] = {0x00,0x00,0x0E,0x11,0x11,0x11,0x0E},
    ['p'] = {0x00,0x00,0x1E,0x11,0x1E,0x10,0x10},
    ['q'] = {0x00,0x00,0x0F,0x11,0x0F,0x01,0x01},
    ['r'] = {0x00,0x00,0x16,0x19,0x10,0x10,0x10},
    ['s'] = {0x00,0x00,0x0F,0x10,0x0E,0x01,0x1E},
    ['t'] = {0x08,0x08,0x1E,0x08,0x08,0x09,0x06},
    ['u'] = {0x00,0x00,0x11,0x11,0x11,0x13,0x0D},
    ['v'] = {0x00,0x00,0x11,0x11,0x11,0x0A,0x04},
    ['w'] = {0x00,0x00,0x11,0x11,0x15,0x15,0x0A},
    ['x'] = {0x00,0x00,0x11,0x0A,0x04,0x0A,0x11},
    ['y'] = {0x00,0x00,0x11,0x11,0x0F,0x01,0x0E},
    ['z'] = {0x00,0x00,0x1F,0x02,0x04,0x08,0x1F},
    [' '] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    ['.'] = {0x00,0x00,0x00,0x00,0x00,0x00,0x04},
    [','] = {0x00,0x00,0x00,0x00,0x00,0x04,0x08},
    [':'] = {0x00,0x00,0x04,0x00,0x00,0x04,0x00},
    ['-'] = {0x00,0x00,0x00,0x1F,0x00,0x00,0x00},
    ['('] = {0x02,0x04,0x08,0x08,0x08,0x04,0x02},
    [')'] = {0x08,0x04,0x02,0x02,0x02,0x04,0x08},
    ['0'] = {0x0E,0x11,0x13,0x15,0x19,0x11,0x0E},
    ['1'] = {0x04,0x0C,0x04,0x04,0x04,0x04,0x0E},
    ['2'] = {0x0E,0x11,0x01,0x06,0x08,0x10,0x1F},
    ['3'] = {0x0E,0x11,0x01,0x06,0x01,0x11,0x0E},
    ['4'] = {0x02,0x06,0x0A,0x12,0x1F,0x02,0x02},
    ['5'] = {0x1F,0x10,0x1E,0x01,0x01,0x11,0x0E},
    ['6'] = {0x06,0x08,0x10,0x1E,0x11,0x11,0x0E},
    ['7'] = {0x1F,0x01,0x02,0x04,0x08,0x08,0x08},
    ['8'] = {0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E},
    ['9'] = {0x0E,0x11,0x11,0x0F,0x01,0x02,0x0C},
};

Renderer::Renderer(int width, int height, double max_radius)
    : width_(width), height_(height), max_radius_(max_radius),
      pixel_radius_(std::min(width, height) * 0.45),
      framebuffer_(width * height * 3, 0) {}

bool Renderer::in_bounds(int x, int y) const {
    return x >= 0 && x < width_ && y >= 0 && y < height_;
}

void Renderer::set_pixel(int x, int y, const Color& c) {
    if (!in_bounds(x, y)) return;
    int idx = (y * width_ + x) * 3;
    framebuffer_[idx] = c.r; framebuffer_[idx+1] = c.g; framebuffer_[idx+2] = c.b;
}

void Renderer::set_pixel_blend(int x, int y, const Color& c, double alpha) {
    if (!in_bounds(x, y) || alpha <= 0.0) return;
    alpha = std::min(1.0, alpha);
    int idx = (y * width_ + x) * 3;
    framebuffer_[idx]   = static_cast<uint8_t>(framebuffer_[idx]   + (c.r - framebuffer_[idx])   * alpha);
    framebuffer_[idx+1] = static_cast<uint8_t>(framebuffer_[idx+1] + (c.g - framebuffer_[idx+1]) * alpha);
    framebuffer_[idx+2] = static_cast<uint8_t>(framebuffer_[idx+2] + (c.b - framebuffer_[idx+2]) * alpha);
}

void Renderer::set_pixel_additive(int x, int y, const Color& c) {
    if (!in_bounds(x, y)) return;
    int idx = (y * width_ + x) * 3;
    framebuffer_[idx]   = std::min(255, framebuffer_[idx]   + c.r);
    framebuffer_[idx+1] = std::min(255, framebuffer_[idx+1] + c.g);
    framebuffer_[idx+2] = std::min(255, framebuffer_[idx+2] + c.b);
}

Color Renderer::get_pixel(int x, int y) const {
    if (!in_bounds(x, y)) return {};
    int idx = (y * width_ + x) * 3;
    return {framebuffer_[idx], framebuffer_[idx+1], framebuffer_[idx+2]};
}

void Renderer::clear() {
    for (int i = 0; i < width_ * height_; ++i) {
        framebuffer_[i*3] = 2; framebuffer_[i*3+1] = 2; framebuffer_[i*3+2] = 8;
    }
}

void Renderer::generate_starfield(uint32_t seed) {
    uint32_t state = seed;
    auto rng = [&]() -> uint32_t { state = state * 1664525u + 1013904223u; return state; };

    int num_stars = (width_ * height_) / 80;
    for (int i = 0; i < num_stars; ++i) {
        int x = rng() % width_, y = rng() % height_;
        uint8_t br = 30 + (rng() % 190);
        uint8_t r = br, g = br, b = br;
        uint32_t tint = rng() % 6;
        if (tint == 0) { r = std::min(255, r + 35); b = std::max(0, b - 20); }
        if (tint == 1) { b = std::min(255, b + 45); r = std::max(0, r - 15); }
        if (tint == 2) { r = std::min(255, r + 20); g = std::min(255, g + 15); }
        set_pixel(x, y, {r, g, b});
        if (br > 170 && (rng() % 4 == 0)) {
            uint8_t dim = br / 4;
            Color dc{dim, dim, dim};
            set_pixel_additive(x-1, y, dc); set_pixel_additive(x+1, y, dc);
            set_pixel_additive(x, y-1, dc); set_pixel_additive(x, y+1, dc);
        }
    }
}

Vec3f Renderer::sqrt_warp(const Vec3f& pos) const {
    double r = pos.magnitude();
    if (r == 0.0) return pos;
    double warped = std::sqrt(r / max_radius_) * pixel_radius_;
    return pos.normalized() * warped;
}

std::pair<int,int> Renderer::project(const Vec3f& pos) const {
    Vec3f w = sqrt_warp(pos);
    return { static_cast<int>(w.x + width_ / 2.0), static_cast<int>(-w.y + height_ / 2.0) };
}

// Anti-aliased filled circle with shading
void Renderer::draw_filled_circle_aa(int cx, int cy, double r, const Color& c) {
    int ir = static_cast<int>(r) + 2;
    for (int dy = -ir; dy <= ir; ++dy) {
        for (int dx = -ir; dx <= ir; ++dx) {
            double dist = std::sqrt(dx*dx + dy*dy);
            if (dist <= r + 1.0) {
                double coverage = std::max(0.0, std::min(1.0, r + 0.5 - dist));
                double shade = 1.0 - 0.35 * std::min(1.0, dist / std::max(1.0, r));
                Color shaded{
                    static_cast<uint8_t>(c.r * shade),
                    static_cast<uint8_t>(c.g * shade),
                    static_cast<uint8_t>(c.b * shade)
                };
                set_pixel_blend(cx + dx, cy + dy, shaded, coverage);
            }
        }
    }
}

void Renderer::draw_glow(int cx, int cy, int r, const Color& c) {
    for (int dy = -r; dy <= r; ++dy) {
        for (int dx = -r; dx <= r; ++dx) {
            double dist = std::sqrt(dx*dx + dy*dy);
            if (dist <= r && dist > 0) {
                double t = 1.0 - dist / r;
                double intensity = t * t * t; // cubic falloff for softer glow
                Color gc{
                    static_cast<uint8_t>(c.r * intensity),
                    static_cast<uint8_t>(c.g * intensity),
                    static_cast<uint8_t>(c.b * intensity)
                };
                set_pixel_additive(cx + dx, cy + dy, gc);
            }
        }
    }
}

// Wu's anti-aliased line
void Renderer::draw_line_aa(int x0, int y0, int x1, int y1, const Color& c, double alpha) {
    bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);
    if (steep) { std::swap(x0, y0); std::swap(x1, y1); }
    if (x0 > x1) { std::swap(x0, x1); std::swap(y0, y1); }

    double dx = x1 - x0, dy = y1 - y0;
    double gradient = dx == 0.0 ? 1.0 : dy / dx;
    double y = y0 + gradient;

    Color ac{ static_cast<uint8_t>(c.r * alpha), static_cast<uint8_t>(c.g * alpha), static_cast<uint8_t>(c.b * alpha) };

    for (int x = x0; x <= x1; ++x) {
        int iy = static_cast<int>(y);
        double frac = y - iy;
        if (steep) {
            set_pixel_blend(iy, x, ac, 1.0 - frac);
            set_pixel_blend(iy + 1, x, ac, frac);
        } else {
            set_pixel_blend(x, iy, ac, 1.0 - frac);
            set_pixel_blend(x, iy + 1, ac, frac);
        }
        y += gradient;
    }
}

void Renderer::draw_trail(const std::vector<Vec3f>& trail, const Color& c) {
    if (trail.size() < 2) return;
    for (size_t i = 1; i < trail.size(); ++i) {
        auto [x0, y0] = project(trail[i-1]);
        auto [x1, y1] = project(trail[i]);
        double alpha = static_cast<double>(i) / trail.size();
        alpha = alpha * alpha * 0.8;
        draw_line_aa(x0, y0, x1, y1, c, alpha);
    }
}

void Renderer::draw_orbit_guide(const OrbitGuide& og) {
    // Sample points around the full orbit and draw as dotted line
    int samples = 720;
    double cO = std::cos(og.Omega), sO = std::sin(og.Omega);
    double cw = std::cos(og.omega), sw = std::sin(og.omega);
    double ci = std::cos(og.i),     si = std::sin(og.i);
    double Px = cO*cw - sO*sw*ci, Qx = -cO*sw - sO*cw*ci;
    double Py = sO*cw + cO*sw*ci, Qy = -sO*sw + cO*cw*ci;

    double p = og.a * (1.0 - og.e * og.e);

    for (int s = 0; s < samples; ++s) {
        double nu = 2.0 * PI * s / samples;
        double r = p / (1.0 + og.e * std::cos(nu));
        double xo = r * std::cos(nu), yo = r * std::sin(nu);
        Vec3f world{ xo * Px + yo * Qx, xo * Py + yo * Qy, 0 };
        auto [sx, sy] = project(world);
        // Dotted: draw every other pair of samples
        if ((s / 3) % 2 == 0) {
            set_pixel_blend(sx, sy, og.color, 0.4);
        }
    }
}

void Renderer::draw_ring(int cx, int cy, int inner, int outer, const Color& c, double tilt) {
    // Draw a tilted ring (ellipse) around a planet
    double cos_tilt = std::cos(tilt);
    for (int dx = -outer; dx <= outer; ++dx) {
        for (int dy_raw = -outer; dy_raw <= outer; ++dy_raw) {
            double dy = dy_raw / cos_tilt; // stretch for tilt
            double dist = std::sqrt(dx*dx + dy*dy);
            if (dist >= inner && dist <= outer) {
                double edge_aa = std::min(
                    std::min(1.0, dist - inner + 0.5),
                    std::min(1.0, outer + 0.5 - dist)
                );
                // Ring brightness varies with angle for texture
                double angle = std::atan2(dy, dx);
                double brightness = 0.6 + 0.4 * std::abs(std::sin(angle * 3.0));
                Color rc{
                    static_cast<uint8_t>(c.r * brightness),
                    static_cast<uint8_t>(c.g * brightness),
                    static_cast<uint8_t>(c.b * brightness)
                };
                set_pixel_blend(cx + dx, cy + dy_raw, rc, edge_aa * 0.7);
            }
        }
    }
}

void Renderer::draw_comet_tail(const Vec3f& pos, const Vec3f& sun_pos, int length, const Color& c) {
    // Tail points away from the Sun
    Vec3f away = (pos - sun_pos).normalized();
    auto [px, py] = project(pos);

    // Draw a fan of lines radiating away
    for (int i = 0; i < length; ++i) {
        double t = static_cast<double>(i) / length;
        double spread = t * 8.0; // tail widens
        double alpha = (1.0 - t) * 0.6;

        // Main tail direction in screen space
        int tx = px + static_cast<int>(away.x * i * 2.5);
        int ty = py + static_cast<int>(-away.y * i * 2.5);

        // Draw a few pixels wide, widening
        int w = static_cast<int>(spread) + 1;
        for (int dy = -w; dy <= w; ++dy) {
            for (int dx = -w; dx <= w; ++dx) {
                double d = std::sqrt(dx*dx + dy*dy);
                if (d <= w) {
                    double fade = alpha * (1.0 - d / (w + 1));
                    Color tc{
                        static_cast<uint8_t>(c.r * fade),
                        static_cast<uint8_t>(c.g * fade),
                        static_cast<uint8_t>(c.b * fade)
                    };
                    set_pixel_additive(tx + dx, ty + dy, tc);
                }
            }
        }
    }
}

void Renderer::draw_particle(const Particle& p) {
    auto [sx, sy] = project(p.position);
    set_pixel_blend(sx, sy, p.color, 0.7);
}

void Renderer::draw_char(int x, int y, char ch, const Color& c) {
    auto idx = static_cast<unsigned char>(ch);
    if (idx >= 128) return;
    for (int row = 0; row < 7; ++row) {
        uint8_t bits = FONT_5X7[idx][row];
        for (int col = 0; col < 5; ++col) {
            if (bits & (0x10 >> col))
                set_pixel(x + col, y + row, c);
        }
    }
}

void Renderer::draw_label(int x, int y, const std::string& text, const Color& c) {
    for (size_t i = 0; i < text.size(); ++i)
        draw_char(x + static_cast<int>(i) * 6, y, text[i], c);
}

void Renderer::draw_legend(const std::vector<LegendEntry>& legend) {
    int x = 14, y = 14;
    draw_label(x, y, "ORRERY", {180, 180, 200});
    y += 14;
    // Separator line
    for (int i = 0; i < 100; ++i)
        set_pixel_blend(x + i, y, {80, 80, 100}, 0.6);
    y += 6;
    for (const auto& entry : legend) {
        // Color swatch
        for (int dy = 0; dy < 5; ++dy)
            for (int dx = 0; dx < 5; ++dx)
                set_pixel(x + dx, y + dy + 1, entry.color);
        draw_label(x + 8, y, entry.name, {170, 170, 180});
        draw_label(x + 8 + static_cast<int>(entry.name.size()) * 6 + 4, y, entry.info, {120, 120, 140});
        y += 11;
    }
}

void Renderer::render_frame(
    const std::vector<RenderBody>& bodies,
    const std::vector<OrbitGuide>& orbits,
    const std::vector<Particle>& particles,
    const std::vector<LegendEntry>& legend
) {
    clear();
    generate_starfield(42);

    // Layer 1: orbit guide ellipses
    for (const auto& og : orbits)
        draw_orbit_guide(og);

    // Layer 2: asteroid belt particles
    for (const auto& p : particles)
        draw_particle(p);

    // Layer 3: trails
    for (const auto& body : bodies)
        draw_trail(body.trail, body.color);

    // Layer 4: comet tails (behind body)
    Vec3f sun_pos = bodies.empty() ? Vec3f{} : bodies[0].position;
    for (const auto& body : bodies) {
        if (body.is_comet && body.tail_length > 0)
            draw_comet_tail(body.position, sun_pos, body.tail_length, body.color);
    }

    // Layer 5: glows
    for (const auto& body : bodies) {
        auto [sx, sy] = project(body.position);
        if (body.glow_radius > 0)
            draw_glow(sx, sy, body.glow_radius, body.glow_color);
    }

    // Layer 6: rings (behind body for top half, but we draw full for simplicity)
    for (const auto& body : bodies) {
        if (body.has_ring) {
            auto [sx, sy] = project(body.position);
            draw_ring(sx, sy, body.ring_inner, body.ring_outer, body.ring_color, 1.2);
        }
    }

    // Layer 7: bodies
    for (const auto& body : bodies) {
        auto [sx, sy] = project(body.position);
        draw_filled_circle_aa(sx, sy, body.display_radius, body.color);
    }

    // Layer 8: labels
    for (const auto& body : bodies) {
        auto [sx, sy] = project(body.position);
        int offset = body.display_radius + 4;
        if (body.has_ring) offset = body.ring_outer + 4;
        draw_label(sx + offset, sy - 3, body.name, {190, 190, 200});
    }

    // Layer 9: legend
    if (!legend.empty())
        draw_legend(legend);
}

void Renderer::write_ppm(const std::string& filename) const {
    std::ofstream out(filename, std::ios::binary);
    out << "P6\n" << width_ << " " << height_ << "\n255\n";
    out.write(reinterpret_cast<const char*>(framebuffer_.data()), framebuffer_.size());
}
