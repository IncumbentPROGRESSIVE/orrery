#include "renderer.h"
#include <fstream>
#include <algorithm>
#include <cstring>

// Tiny 5x7 bitmap font for labels (subset: A-Z, 0-9, space, period, paren)
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
    framebuffer_[idx]     = c.r;
    framebuffer_[idx + 1] = c.g;
    framebuffer_[idx + 2] = c.b;
}

void Renderer::set_pixel_additive(int x, int y, const Color& c) {
    if (!in_bounds(x, y)) return;
    int idx = (y * width_ + x) * 3;
    framebuffer_[idx]     = std::min(255, framebuffer_[idx] + c.r);
    framebuffer_[idx + 1] = std::min(255, framebuffer_[idx + 1] + c.g);
    framebuffer_[idx + 2] = std::min(255, framebuffer_[idx + 2] + c.b);
}

Color Renderer::get_pixel(int x, int y) const {
    if (!in_bounds(x, y)) return {};
    int idx = (y * width_ + x) * 3;
    return {framebuffer_[idx], framebuffer_[idx+1], framebuffer_[idx+2]};
}

void Renderer::clear() {
    // Deep space background — not pure black, slight blue tint
    for (int i = 0; i < width_ * height_; ++i) {
        framebuffer_[i * 3]     = 2;
        framebuffer_[i * 3 + 1] = 2;
        framebuffer_[i * 3 + 2] = 8;
    }
}

void Renderer::generate_starfield(uint32_t seed) {
    // Simple LCG PRNG for deterministic starfield
    uint32_t state = seed;
    auto rng = [&]() -> uint32_t {
        state = state * 1664525u + 1013904223u;
        return state;
    };

    int num_stars = (width_ * height_) / 90;
    for (int i = 0; i < num_stars; ++i) {
        int x = rng() % width_;
        int y = rng() % height_;
        uint8_t brightness = 40 + (rng() % 180);
        // Slight color variation
        uint8_t r = brightness, g = brightness, b = brightness;
        uint32_t tint = rng() % 5;
        if (tint == 0) { r = std::min(255, r + 30); b = std::max(0, b - 20); } // warm
        if (tint == 1) { b = std::min(255, b + 40); r = std::max(0, r - 15); } // cool
        set_pixel(x, y, {r, g, b});
        // Occasional brighter star with a tiny cross
        if (brightness > 180 && (rng() % 3 == 0)) {
            uint8_t dim = brightness / 3;
            Color dc{dim, dim, dim};
            set_pixel_additive(x-1, y, dc);
            set_pixel_additive(x+1, y, dc);
            set_pixel_additive(x, y-1, dc);
            set_pixel_additive(x, y+1, dc);
        }
    }
}

Vec3f Renderer::sqrt_warp(const Vec3f& pos) const {
    double r = pos.magnitude();
    if (r == 0.0) return pos;
    // sqrt mapping: maps [0, max_radius_] -> [0, pixel_radius_]
    double normalized = r / max_radius_;
    double warped = std::sqrt(normalized) * pixel_radius_;
    Vec3f dir = pos.normalized();
    return dir * warped;
}

std::pair<int,int> Renderer::project(const Vec3f& pos) const {
    Vec3f warped = sqrt_warp(pos);
    int sx = static_cast<int>(warped.x + width_ / 2.0);
    int sy = static_cast<int>(-warped.y + height_ / 2.0);
    return {sx, sy};
}

void Renderer::draw_filled_circle(int cx, int cy, int r, const Color& c) {
    for (int dy = -r; dy <= r; ++dy) {
        for (int dx = -r; dx <= r; ++dx) {
            double dist = std::sqrt(dx*dx + dy*dy);
            if (dist <= r) {
                // Slight shading: brighter at center
                double shade = 1.0 - 0.3 * (dist / r);
                Color shaded{
                    static_cast<uint8_t>(c.r * shade),
                    static_cast<uint8_t>(c.g * shade),
                    static_cast<uint8_t>(c.b * shade)
                };
                set_pixel(cx + dx, cy + dy, shaded);
            }
        }
    }
}

void Renderer::draw_glow(int cx, int cy, int r, const Color& c) {
    for (int dy = -r; dy <= r; ++dy) {
        for (int dx = -r; dx <= r; ++dx) {
            double dist = std::sqrt(dx*dx + dy*dy);
            if (dist <= r && dist > 0) {
                double intensity = 1.0 - (dist / r);
                intensity = intensity * intensity; // quadratic falloff
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

void Renderer::draw_line(int x0, int y0, int x1, int y1, const Color& c, double alpha) {
    // Bresenham's line
    int dx = std::abs(x1 - x0), dy = std::abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1, sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;
    Color ac{
        static_cast<uint8_t>(c.r * alpha),
        static_cast<uint8_t>(c.g * alpha),
        static_cast<uint8_t>(c.b * alpha)
    };
    while (true) {
        set_pixel_additive(x0, y0, ac);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx)  { err += dx; y0 += sy; }
    }
}

void Renderer::draw_trail(const std::vector<Vec3f>& trail, const Color& c) {
    if (trail.size() < 2) return;
    for (size_t i = 1; i < trail.size(); ++i) {
        auto [x0, y0] = project(trail[i-1]);
        auto [x1, y1] = project(trail[i]);
        // Fade trail: older = dimmer
        double alpha = static_cast<double>(i) / trail.size();
        alpha = alpha * alpha * 0.7; // keep it subtle
        draw_line(x0, y0, x1, y1, c, alpha);
    }
}

void Renderer::draw_char(int x, int y, char ch, const Color& c) {
    auto idx = static_cast<unsigned char>(ch);
    if (idx >= 128) return;
    for (int row = 0; row < 7; ++row) {
        uint8_t bits = FONT_5X7[idx][row];
        for (int col = 0; col < 5; ++col) {
            if (bits & (0x10 >> col)) {
                set_pixel(x + col, y + row, c);
            }
        }
    }
}

void Renderer::draw_label(int x, int y, const std::string& text, const Color& c) {
    for (size_t i = 0; i < text.size(); ++i) {
        draw_char(x + static_cast<int>(i) * 6, y, text[i], c);
    }
}

void Renderer::render_frame(const std::vector<RenderBody>& bodies) {
    clear();
    generate_starfield(42);

    // Draw trails first (behind everything)
    for (const auto& body : bodies) {
        draw_trail(body.trail, body.color);
    }

    // Draw glows, then bodies, then labels
    for (const auto& body : bodies) {
        auto [sx, sy] = project(body.position);
        if (body.glow_radius > 0) {
            draw_glow(sx, sy, body.glow_radius, body.glow_color);
        }
    }
    for (const auto& body : bodies) {
        auto [sx, sy] = project(body.position);
        draw_filled_circle(sx, sy, body.display_radius, body.color);
    }
    for (const auto& body : bodies) {
        auto [sx, sy] = project(body.position);
        draw_label(sx + body.display_radius + 4, sy - 3, body.name, {200, 200, 200});
    }
}

void Renderer::write_ppm(const std::string& filename) const {
    std::ofstream out(filename, std::ios::binary);
    out << "P6\n" << width_ << " " << height_ << "\n255\n";
    out.write(reinterpret_cast<const char*>(framebuffer_.data()), framebuffer_.size());
}
