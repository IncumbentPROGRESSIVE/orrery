#pragma once
#include <string>
#include <vector>
#include <cmath>
#include <cstdint>

struct Vec3f {
    double x = 0.0, y = 0.0, z = 0.0;
    Vec3f() = default;
    Vec3f(double x, double y, double z) : x(x), y(y), z(z) {}
    double magnitude() const { return std::sqrt(x*x + y*y + z*z); }
    Vec3f normalized() const {
        double m = magnitude();
        return m == 0.0 ? Vec3f{} : Vec3f{x/m, y/m, z/m};
    }
    Vec3f operator+(const Vec3f& o) const { return {x+o.x, y+o.y, z+o.z}; }
    Vec3f operator-(const Vec3f& o) const { return {x-o.x, y-o.y, z-o.z}; }
    Vec3f operator*(double s) const { return {x*s, y*s, z*s}; }
    double dot(const Vec3f& o) const { return x*o.x + y*o.y + z*o.z; }
};

struct Color {
    uint8_t r = 0, g = 0, b = 0;
    Color() = default;
    Color(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}
    Color blend(const Color& other, double t) const {
        auto cl = [](double v) { return static_cast<uint8_t>(std::max(0.0, std::min(255.0, v))); };
        return { cl(r + (other.r - r) * t), cl(g + (other.g - g) * t), cl(b + (other.b - b) * t) };
    }
};

struct OrbitGuide {
    double a, e, i, omega, Omega; // orbital elements for drawing the full ellipse
    Color color;
};

enum class TextureType {
    FLAT, SUN, MERCURY, VENUS, EARTH, MARS, JUPITER, SATURN, URANUS, NEPTUNE, COMET
};

struct RenderBody {
    std::string name;
    Vec3f position;
    int display_radius;
    Color color;
    int glow_radius;
    Color glow_color;
    std::vector<Vec3f> trail;
    bool has_ring = false;
    int ring_inner = 0, ring_outer = 0;
    Color ring_color;
    bool is_comet = false;
    int tail_length = 0;
    TextureType texture = TextureType::FLAT;
    double rotation_phase = 0.0;
};

struct Particle {
    Vec3f position;
    Color color;
};

struct LegendEntry {
    std::string name;
    Color color;
    std::string info;
};

class Renderer {
public:
    Renderer(int width, int height, double max_radius);
    void set_pan(double px, double py) { pan_x_ = px; pan_y_ = py; }
    void render_frame(
        const std::vector<RenderBody>& bodies,
        const std::vector<OrbitGuide>& orbits = {},
        const std::vector<Particle>& particles = {},
        const std::vector<LegendEntry>& legend = {}
    );
    void write_ppm(const std::string& filename) const;
    const uint8_t* framebuffer() const { return framebuffer_.data(); }
    int width() const { return width_; }
    int height() const { return height_; }

private:
    int width_, height_;
    double max_radius_, pixel_radius_;
    double pan_x_ = 0.0, pan_y_ = 0.0;
    std::vector<uint8_t> framebuffer_;

    void clear();
    void generate_starfield(uint32_t seed);
    void set_pixel(int x, int y, const Color& c);
    void set_pixel_blend(int x, int y, const Color& c, double alpha);
    void set_pixel_additive(int x, int y, const Color& c);
    Color get_pixel(int x, int y) const;
    std::pair<int,int> project(const Vec3f& pos) const;
    Vec3f sqrt_warp(const Vec3f& pos) const;
    bool in_bounds(int x, int y) const;

    void draw_filled_circle_aa(int cx, int cy, double r, const Color& c);
    void draw_textured_body(int cx, int cy, double r, const RenderBody& body);
    Color sample_texture(TextureType tex, double u, double v, double phase, const Color& base);
    void draw_glow(int cx, int cy, int r, const Color& c);
    void draw_trail(const std::vector<Vec3f>& trail, const Color& c);
    void draw_line_aa(int x0, int y0, int x1, int y1, const Color& c, double alpha);
    void draw_orbit_guide(const OrbitGuide& og);
    void draw_ring(int cx, int cy, int inner, int outer, const Color& c, double tilt);
    void draw_comet_tail(const Vec3f& pos, const Vec3f& sun_pos, int length, const Color& c);
    void draw_particle(const Particle& p);
    void draw_char(int x, int y, char ch, const Color& c);
    void draw_label(int x, int y, const std::string& text, const Color& c);
    void draw_legend(const std::vector<LegendEntry>& legend);
};
