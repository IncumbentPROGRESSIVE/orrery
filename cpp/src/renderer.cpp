#include "renderer.h"
#include <fstream>
#include <algorithm>
#include <cstring>

Renderer::Renderer(int width, int height, double scale)
    : width_(width), height_(height), scale_(scale),
      camera_pos_{0, 0, 1e12}, camera_dir_{0, 0, -1},
      framebuffer_(width * height * 3, 0) {}

void Renderer::set_camera(const Vec3f& position, const Vec3f& target) {
    camera_pos_ = position;
    camera_dir_ = (target - position).normalized();
}

void Renderer::clear() {
    std::memset(framebuffer_.data(), 0, framebuffer_.size());
}

std::pair<int,int> Renderer::project(const Vec3f& world_pos) const {
    Vec3f rel = world_pos - camera_pos_;
    int sx = static_cast<int>(rel.x * scale_ + width_ / 2.0);
    int sy = static_cast<int>(rel.y * scale_ + height_ / 2.0);
    return {sx, sy};
}

void Renderer::draw_circle(int cx, int cy, int r, uint8_t red, uint8_t green, uint8_t blue) {
    for (int dy = -r; dy <= r; ++dy) {
        for (int dx = -r; dx <= r; ++dx) {
            if (dx*dx + dy*dy <= r*r) {
                int px = cx + dx, py = cy + dy;
                if (px >= 0 && px < width_ && py >= 0 && py < height_) {
                    int idx = (py * width_ + px) * 3;
                    framebuffer_[idx]     = red;
                    framebuffer_[idx + 1] = green;
                    framebuffer_[idx + 2] = blue;
                }
            }
        }
    }
}

void Renderer::render_frame(const std::vector<RenderBody>& bodies) {
    clear();
    for (const auto& body : bodies) {
        auto [sx, sy] = project(body.position);
        int pixel_radius = std::max(1, static_cast<int>(body.radius * scale_));
        auto r = static_cast<uint8_t>(body.color[0] * 255);
        auto g = static_cast<uint8_t>(body.color[1] * 255);
        auto b = static_cast<uint8_t>(body.color[2] * 255);
        draw_circle(sx, sy, pixel_radius, r, g, b);
    }
}

void Renderer::write_ppm(const std::string& filename) const {
    std::ofstream out(filename, std::ios::binary);
    out << "P6\n" << width_ << " " << height_ << "\n255\n";
    out.write(reinterpret_cast<const char*>(framebuffer_.data()), framebuffer_.size());
}
