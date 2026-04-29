#pragma once
#include "renderer.h"
#include <vector>
#include <functional>

struct BodyState {
    Vec3f position;
    Vec3f velocity;
    double mass;
};

class OrbitIntegrator {
public:
    static constexpr double G = 6.674e-11;

    explicit OrbitIntegrator(double softening = 1e4) : softening_(softening) {}

    Vec3f compute_acceleration(const BodyState& body, const std::vector<BodyState>& all) const {
        Vec3f acc{};
        for (const auto& other : all) {
            if (&body == &other) continue;
            Vec3f r = other.position - body.position;
            double dist_sq = r.dot(r) + softening_ * softening_;
            double dist = std::sqrt(dist_sq);
            double a_mag = G * other.mass / dist_sq;
            acc = acc + r.normalized() * a_mag;
        }
        return acc;
    }

    void step_leapfrog(std::vector<BodyState>& bodies, double dt) const {
        for (auto& b : bodies) {
            Vec3f acc = compute_acceleration(b, bodies);
            b.velocity = b.velocity + acc * (dt * 0.5);
        }
        for (auto& b : bodies) {
            b.position = b.position + b.velocity * dt;
        }
        for (auto& b : bodies) {
            Vec3f acc = compute_acceleration(b, bodies);
            b.velocity = b.velocity + acc * (dt * 0.5);
        }
    }

    std::vector<Vec3f> compute_trajectory(
        BodyState body,
        const std::vector<BodyState>& others,
        double dt, int steps
    ) const {
        std::vector<Vec3f> path;
        path.reserve(steps);
        auto all = others;
        all.push_back(body);
        size_t idx = all.size() - 1;
        for (int i = 0; i < steps; ++i) {
            step_single(all, idx, dt);
            path.push_back(all[idx].position);
        }
        return path;
    }

private:
    double softening_;

    void step_single(std::vector<BodyState>& bodies, size_t idx, double dt) const {
        Vec3f acc = compute_acceleration(bodies[idx], bodies);
        bodies[idx].velocity = bodies[idx].velocity + acc * dt;
        bodies[idx].position = bodies[idx].position + bodies[idx].velocity * dt;
    }
};
