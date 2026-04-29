#pragma once
#include <string>
#include <vector>
#include <cmath>

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

struct RenderBody {
    std::string name;
    Vec3f position;
    double radius;
    float color[3];
};

class Renderer {
public:
    Renderer(int width, int height, double scale);
    void set_camera(const Vec3f& position, const Vec3f& target);
    void render_frame(const std::vector<RenderBody>& bodies);
    void write_ppm(const std::string& filename) const;

private:
    int width_, height_;
    double scale_;
    Vec3f camera_pos_, camera_dir_;
    std::vector<uint8_t> framebuffer_;

    void clear();
    void draw_circle(int cx, int cy, int r, uint8_t red, uint8_t green, uint8_t blue);
    std::pair<int,int> project(const Vec3f& world_pos) const;
};
