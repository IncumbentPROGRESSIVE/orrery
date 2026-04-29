use crate::vector::Vec3;

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub enum BodyType {
    Star,
    Planet,
    Moon,
    Asteroid,
    Comet,
}

#[derive(Debug, Clone)]
pub struct CelestialBody {
    pub name: String,
    pub body_type: BodyType,
    pub mass: f64,
    pub radius: f64,
    pub position: Vec3,
    pub velocity: Vec3,
    pub acceleration: Vec3,
    pub parent_id: Option<usize>,
}

impl CelestialBody {
    pub fn new(name: &str, body_type: BodyType, mass: f64, radius: f64) -> Self {
        Self {
            name: name.to_string(),
            body_type,
            mass,
            radius,
            position: Vec3::ZERO,
            velocity: Vec3::ZERO,
            acceleration: Vec3::ZERO,
            parent_id: None,
        }
    }

    pub fn with_state(mut self, position: Vec3, velocity: Vec3) -> Self {
        self.position = position;
        self.velocity = velocity;
        self
    }

    pub fn with_parent(mut self, parent_id: usize) -> Self {
        self.parent_id = Some(parent_id);
        self
    }

    pub fn kinetic_energy(&self) -> f64 {
        0.5 * self.mass * self.velocity.magnitude().powi(2)
    }

    pub fn momentum(&self) -> Vec3 {
        self.velocity.scale(self.mass)
    }

    pub fn angular_momentum(&self, origin: &Vec3) -> Vec3 {
        let r = self.position - *origin;
        r.cross(&self.momentum())
    }

    pub fn surface_gravity(&self) -> f64 {
        const G: f64 = 6.674e-11;
        if self.radius == 0.0 {
            return 0.0;
        }
        G * self.mass / (self.radius * self.radius)
    }

    pub fn escape_velocity(&self) -> f64 {
        const G: f64 = 6.674e-11;
        if self.radius == 0.0 {
            return 0.0;
        }
        (2.0 * G * self.mass / self.radius).sqrt()
    }

    pub fn hill_sphere_radius(&self, parent_mass: f64, semi_major_axis: f64) -> f64 {
        semi_major_axis * (self.mass / (3.0 * parent_mass)).powf(1.0 / 3.0)
    }
}

pub fn create_solar_system() -> Vec<CelestialBody> {
    let sun = CelestialBody::new("Sun", BodyType::Star, 1.989e30, 6.957e8);

    let earth = CelestialBody::new("Earth", BodyType::Planet, 5.972e24, 6.371e6)
        .with_state(Vec3::new(1.496e11, 0.0, 0.0), Vec3::new(0.0, 2.978e4, 0.0))
        .with_parent(0);

    let mars = CelestialBody::new("Mars", BodyType::Planet, 6.417e23, 3.389e6)
        .with_state(Vec3::new(2.279e11, 0.0, 0.0), Vec3::new(0.0, 2.407e4, 0.0))
        .with_parent(0);

    let jupiter = CelestialBody::new("Jupiter", BodyType::Planet, 1.898e27, 6.991e7)
        .with_state(Vec3::new(7.785e11, 0.0, 0.0), Vec3::new(0.0, 1.307e4, 0.0))
        .with_parent(0);

    let moon = CelestialBody::new("Moon", BodyType::Moon, 7.342e22, 1.737e6)
        .with_state(
            Vec3::new(1.496e11 + 3.844e8, 0.0, 0.0),
            Vec3::new(0.0, 2.978e4 + 1.022e3, 0.0),
        )
        .with_parent(1);

    vec![sun, earth, mars, jupiter, moon]
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_kinetic_energy() {
        let body = CelestialBody::new("test", BodyType::Asteroid, 100.0, 1.0)
            .with_state(Vec3::ZERO, Vec3::new(10.0, 0.0, 0.0));
        assert!((body.kinetic_energy() - 5000.0).abs() < 1e-10);
    }

    #[test]
    fn test_surface_gravity_zero_radius() {
        let body = CelestialBody::new("point", BodyType::Asteroid, 1e10, 0.0);
        assert_eq!(body.surface_gravity(), 0.0);
    }
}
