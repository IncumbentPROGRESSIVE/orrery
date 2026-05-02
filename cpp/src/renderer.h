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
        return {
            static_cast<uint8_t>(r + (other.r - r) * t),
            static_cast<uint8_t>(g + (other.g - g) * t),
            static_cast<uint8_t>(b + (other.b - b) * t)
        };
    }
    Color add_clamped(const Color& other) const {
        return {
            static_cast<uint8_t>(std::min(255, r + other.r)),
            static_cast<uint8_t>(std::min(255, g + other.g)),
            static_cast<uint8_t>(std::min(255, b + other.b))
        };
    }
};

struct RenderBody {
    std::string name;
    Vec3f position;
    int display_radius;
    Color color;
    int glow_radius;
    Color glow_color;
    std::vector<Vec3f> trail;
};

class Renderer {
public:
    Renderer(int width, int height, double max_radius);
    void render_frame(const std::vector<RenderBody>& bodies);
    void write_ppm(const std::string& filename) const;

private:
    int width_, height_;
    double max_radius_;
    double pixel_radius_;
    std::vector<uint8_t> framebuffer_;

    void clear();
    void generate_starfield(uint32_t seed);
    void set_pixel(int x, int y, const Color& c);
    void set_pixel_additive(int x, int y, const Color& c);
    Color get_pixel(int x, int y) const;
    std::pair<int,int> project(const Vec3f& pos) const;
    Vec3f sqrt_warp(const Vec3f& pos) const;
    bool in_bounds(int x, int y) const;

    void draw_filled_circle(int cx, int cy, int r, const Color& c);
    void draw_glow(int cx, int cy, int r, const Color& c);
    void draw_trail(const std::vector<Vec3f>& trail, const Color& c);
    void draw_line(int x0, int y0, int x1, int y1, const Color& c, double alpha);
    void draw_char(int x, int y, char ch, const Color& c);
    void draw_label(int x, int y, const std::string& text, const Color& c);
};
