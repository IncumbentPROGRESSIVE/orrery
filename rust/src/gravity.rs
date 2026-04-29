use crate::celestial_body::CelestialBody;
use crate::vector::Vec3;

const G: f64 = 6.674e-11;

pub struct GravityField {
    pub softening: f64,
}

impl GravityField {
    pub fn new() -> Self {
        Self { softening: 1e4 }
    }

    pub fn with_softening(softening: f64) -> Self {
        Self { softening }
    }

    pub fn compute_force(&self, a: &CelestialBody, b: &CelestialBody) -> Vec3 {
        let r = b.position - a.position;
        let dist_sq = r.dot(&r) + self.softening * self.softening;
        let force_mag = G * a.mass * b.mass / dist_sq;
        r.normalized().scale(force_mag)
    }

    pub fn compute_acceleration(&self, body: &CelestialBody, others: &[CelestialBody]) -> Vec3 {
        let mut acc = Vec3::ZERO;
        for other in others {
            if std::ptr::eq(body, other) {
                continue;
            }
            let r = other.position - body.position;
            let dist_sq = r.dot(&r) + self.softening * self.softening;
            let a_mag = G * other.mass / dist_sq;
            acc = acc + r.normalized().scale(a_mag);
        }
        acc
    }

    pub fn gravitational_potential(&self, position: &Vec3, bodies: &[CelestialBody]) -> f64 {
        let mut potential = 0.0;
        for body in bodies {
            let r = position.distance_to(&body.position);
            let r_soft = (r * r + self.softening * self.softening).sqrt();
            potential -= G * body.mass / r_soft;
        }
        potential
    }

    pub fn total_energy(bodies: &[CelestialBody]) -> f64 {
        let mut kinetic = 0.0;
        let mut potential = 0.0;

        for body in bodies {
            kinetic += body.kinetic_energy();
        }

        for i in 0..bodies.len() {
            for j in (i + 1)..bodies.len() {
                let r = bodies[i].position.distance_to(&bodies[j].position);
                if r > 0.0 {
                    potential -= G * bodies[i].mass * bodies[j].mass / r;
                }
            }
        }

        kinetic + potential
    }

    pub fn barycenter(bodies: &[CelestialBody]) -> Vec3 {
        let mut total_mass = 0.0;
        let mut weighted_pos = Vec3::ZERO;
        for body in bodies {
            total_mass += body.mass;
            weighted_pos = weighted_pos + body.position.scale(body.mass);
        }
        if total_mass == 0.0 {
            return Vec3::ZERO;
        }
        weighted_pos.scale(1.0 / total_mass)
    }
}

impl Default for GravityField {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::celestial_body::BodyType;

    #[test]
    fn test_barycenter_single_body() {
        let body = CelestialBody::new("a", BodyType::Star, 1.0, 1.0)
            .with_state(Vec3::new(10.0, 0.0, 0.0), Vec3::ZERO);
        let bc = GravityField::barycenter(&[body]);
        assert!((bc.x - 10.0).abs() < 1e-10);
    }

    #[test]
    fn test_force_symmetry() {
        let gf = GravityField::new();
        let a = CelestialBody::new("a", BodyType::Star, 1e20, 1.0)
            .with_state(Vec3::ZERO, Vec3::ZERO);
        let b = CelestialBody::new("b", BodyType::Planet, 1e20, 1.0)
            .with_state(Vec3::new(1e8, 0.0, 0.0), Vec3::ZERO);
        let f_ab = gf.compute_force(&a, &b);
        let f_ba = gf.compute_force(&b, &a);
        assert!((f_ab.x + f_ba.x).abs() < 1e-5);
    }
}
