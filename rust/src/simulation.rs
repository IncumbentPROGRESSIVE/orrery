use crate::celestial_body::CelestialBody;
use crate::gravity::GravityField;
use crate::time::{SimTime, TimeStep};
use crate::vector::Vec3;

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum Integrator {
    Euler,
    Verlet,
    RungeKutta4,
}

pub struct Simulation {
    pub bodies: Vec<CelestialBody>,
    pub time: SimTime,
    pub timestep: TimeStep,
    pub integrator: Integrator,
    gravity: GravityField,
    step_count: u64,
}

impl Simulation {
    pub fn new(bodies: Vec<CelestialBody>, dt: f64) -> Self {
        Self {
            bodies,
            time: SimTime::zero(),
            timestep: TimeStep::fixed(dt),
            integrator: Integrator::Verlet,
            gravity: GravityField::new(),
            step_count: 0,
        }
    }

    pub fn with_integrator(mut self, integrator: Integrator) -> Self {
        self.integrator = integrator;
        self
    }

    pub fn step(&mut self) {
        let dt = self.timestep.dt;
        match self.integrator {
            Integrator::Euler => self.step_euler(dt),
            Integrator::Verlet => self.step_verlet(dt),
            Integrator::RungeKutta4 => self.step_rk4(dt),
        }
        self.time.advance(dt);
        self.step_count += 1;
    }

    pub fn run_for(&mut self, duration: f64) {
        let mut elapsed = 0.0;
        while elapsed < duration {
            let dt = self.timestep.dt.min(duration - elapsed);
            self.timestep.dt = dt;
            self.step();
            elapsed += dt;
        }
    }

    pub fn step_count(&self) -> u64 {
        self.step_count
    }

    pub fn total_energy(&self) -> f64 {
        GravityField::total_energy(&self.bodies)
    }

    pub fn barycenter(&self) -> Vec3 {
        GravityField::barycenter(&self.bodies)
    }

    fn step_euler(&mut self, dt: f64) {
        let accelerations: Vec<Vec3> = self
            .bodies
            .iter()
            .map(|b| self.gravity.compute_acceleration(b, &self.bodies))
            .collect();

        for (body, acc) in self.bodies.iter_mut().zip(accelerations.iter()) {
            body.position = body.position + body.velocity.scale(dt);
            body.velocity = body.velocity + acc.scale(dt);
            body.acceleration = *acc;
        }
    }

    fn step_verlet(&mut self, dt: f64) {
        let old_accelerations: Vec<Vec3> = self
            .bodies
            .iter()
            .map(|b| self.gravity.compute_acceleration(b, &self.bodies))
            .collect();

        for (body, acc) in self.bodies.iter_mut().zip(old_accelerations.iter()) {
            body.position = body.position
                + body.velocity.scale(dt)
                + acc.scale(0.5 * dt * dt);
        }

        let new_accelerations: Vec<Vec3> = self
            .bodies
            .iter()
            .map(|b| self.gravity.compute_acceleration(b, &self.bodies))
            .collect();

        for (i, body) in self.bodies.iter_mut().enumerate() {
            body.velocity = body.velocity
                + (old_accelerations[i] + new_accelerations[i]).scale(0.5 * dt);
            body.acceleration = new_accelerations[i];
        }
    }

    fn step_rk4(&mut self, dt: f64) {
        let n = self.bodies.len();
        let positions: Vec<Vec3> = self.bodies.iter().map(|b| b.position).collect();
        let velocities: Vec<Vec3> = self.bodies.iter().map(|b| b.velocity).collect();

        let k1v: Vec<Vec3> = self
            .bodies
            .iter()
            .map(|b| self.gravity.compute_acceleration(b, &self.bodies))
            .collect();
        let k1x: Vec<Vec3> = velocities.clone();

        let mut temp_bodies = self.bodies.clone();
        for i in 0..n {
            temp_bodies[i].position = positions[i] + k1x[i].scale(dt * 0.5);
            temp_bodies[i].velocity = velocities[i] + k1v[i].scale(dt * 0.5);
        }
        let k2v: Vec<Vec3> = temp_bodies
            .iter()
            .map(|b| self.gravity.compute_acceleration(b, &temp_bodies))
            .collect();
        let k2x: Vec<Vec3> = temp_bodies.iter().map(|b| b.velocity).collect();

        for i in 0..n {
            temp_bodies[i].position = positions[i] + k2x[i].scale(dt * 0.5);
            temp_bodies[i].velocity = velocities[i] + k2v[i].scale(dt * 0.5);
        }
        let k3v: Vec<Vec3> = temp_bodies
            .iter()
            .map(|b| self.gravity.compute_acceleration(b, &temp_bodies))
            .collect();
        let k3x: Vec<Vec3> = temp_bodies.iter().map(|b| b.velocity).collect();

        for i in 0..n {
            temp_bodies[i].position = positions[i] + k3x[i].scale(dt);
            temp_bodies[i].velocity = velocities[i] + k3v[i].scale(dt);
        }
        let k4v: Vec<Vec3> = temp_bodies
            .iter()
            .map(|b| self.gravity.compute_acceleration(b, &temp_bodies))
            .collect();
        let k4x: Vec<Vec3> = temp_bodies.iter().map(|b| b.velocity).collect();

        for i in 0..n {
            self.bodies[i].position = positions[i]
                + (k1x[i] + k2x[i].scale(2.0) + k3x[i].scale(2.0) + k4x[i]).scale(dt / 6.0);
            self.bodies[i].velocity = velocities[i]
                + (k1v[i] + k2v[i].scale(2.0) + k3v[i].scale(2.0) + k4v[i]).scale(dt / 6.0);
            self.bodies[i].acceleration = k1v[i];
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::celestial_body::create_solar_system;

    #[test]
    fn test_simulation_step_advances_time() {
        let bodies = create_solar_system();
        let mut sim = Simulation::new(bodies, 3600.0);
        sim.step();
        assert!(sim.time.as_seconds() > 0.0);
        assert_eq!(sim.step_count(), 1);
    }

    #[test]
    fn test_energy_conservation_verlet() {
        let bodies = create_solar_system();
        let mut sim = Simulation::new(bodies, 3600.0)
            .with_integrator(Integrator::Verlet);
        let e0 = sim.total_energy();
        sim.run_for(86400.0);
        let e1 = sim.total_energy();
        let drift = ((e1 - e0) / e0).abs();
        assert!(drift < 1e-4, "Energy drift too large: {}", drift);
    }
}
