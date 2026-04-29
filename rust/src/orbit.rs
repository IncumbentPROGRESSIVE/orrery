use crate::vector::Vec3;
use std::f64::consts::PI;

#[derive(Debug, Clone, Copy)]
pub struct OrbitalElements {
    pub semi_major_axis: f64,
    pub eccentricity: f64,
    pub inclination: f64,
    pub longitude_ascending: f64,
    pub argument_periapsis: f64,
    pub true_anomaly: f64,
}

impl OrbitalElements {
    pub fn circular(radius: f64, inclination: f64) -> Self {
        Self {
            semi_major_axis: radius,
            eccentricity: 0.0,
            inclination,
            longitude_ascending: 0.0,
            argument_periapsis: 0.0,
            true_anomaly: 0.0,
        }
    }

    pub fn period(&self, central_mass: f64) -> f64 {
        const G: f64 = 6.674e-11;
        2.0 * PI * (self.semi_major_axis.powi(3) / (G * central_mass)).sqrt()
    }

    pub fn periapsis(&self) -> f64 {
        self.semi_major_axis * (1.0 - self.eccentricity)
    }

    pub fn apoapsis(&self) -> f64 {
        self.semi_major_axis * (1.0 + self.eccentricity)
    }

    pub fn radius_at_anomaly(&self, true_anomaly: f64) -> f64 {
        let p = self.semi_major_axis * (1.0 - self.eccentricity * self.eccentricity);
        p / (1.0 + self.eccentricity * true_anomaly.cos())
    }

    pub fn mean_anomaly_from_eccentric(&self, eccentric_anomaly: f64) -> f64 {
        eccentric_anomaly - self.eccentricity * eccentric_anomaly.sin()
    }

    pub fn eccentric_anomaly_from_mean(&self, mean_anomaly: f64) -> f64 {
        let mut e_anom = mean_anomaly;
        for _ in 0..50 {
            let delta = e_anom - self.eccentricity * e_anom.sin() - mean_anomaly;
            let derivative = 1.0 - self.eccentricity * e_anom.cos();
            e_anom -= delta / derivative;
            if delta.abs() < 1e-12 {
                break;
            }
        }
        e_anom
    }

    pub fn velocity_at_radius(&self, r: f64, central_mass: f64) -> f64 {
        const G: f64 = 6.674e-11;
        (G * central_mass * (2.0 / r - 1.0 / self.semi_major_axis)).sqrt()
    }

    pub fn to_cartesian(&self, central_mass: f64) -> (Vec3, Vec3) {
        let r = self.radius_at_anomaly(self.true_anomaly);
        let v = self.velocity_at_radius(r, central_mass);

        let cos_ta = self.true_anomaly.cos();
        let sin_ta = self.true_anomaly.sin();
        let cos_i = self.inclination.cos();
        let sin_i = self.inclination.sin();
        let cos_lan = self.longitude_ascending.cos();
        let sin_lan = self.longitude_ascending.sin();
        let cos_ap = self.argument_periapsis.cos();
        let sin_ap = self.argument_periapsis.sin();

        let x_orb = r * cos_ta;
        let y_orb = r * sin_ta;

        let px = cos_lan * cos_ap - sin_lan * sin_ap * cos_i;
        let py = sin_lan * cos_ap + cos_lan * sin_ap * cos_i;
        let pz = sin_ap * sin_i;

        let qx = -cos_lan * sin_ap - sin_lan * cos_ap * cos_i;
        let qy = -sin_lan * sin_ap + cos_lan * cos_ap * cos_i;
        let qz = cos_ap * sin_i;

        let position = Vec3::new(
            x_orb * px + y_orb * qx,
            x_orb * py + y_orb * qy,
            x_orb * pz + y_orb * qz,
        );

        let vx_orb = -sin_ta;
        let vy_orb = self.eccentricity + cos_ta;
        let v_scale = v / (1.0 + self.eccentricity * cos_ta);

        let velocity = Vec3::new(
            v_scale * (vx_orb * px + vy_orb * qx),
            v_scale * (vx_orb * py + vy_orb * qy),
            v_scale * (vx_orb * pz + vy_orb * qz),
        );

        (position, velocity)
    }

    pub fn specific_orbital_energy(&self, central_mass: f64) -> f64 {
        const G: f64 = 6.674e-11;
        -G * central_mass / (2.0 * self.semi_major_axis)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_circular_orbit_periapsis_equals_apoapsis() {
        let orbit = OrbitalElements::circular(1e8, 0.0);
        assert!((orbit.periapsis() - orbit.apoapsis()).abs() < 1e-5);
    }

    #[test]
    fn test_kepler_equation_convergence() {
        let orbit = OrbitalElements {
            semi_major_axis: 1e11,
            eccentricity: 0.5,
            inclination: 0.0,
            longitude_ascending: 0.0,
            argument_periapsis: 0.0,
            true_anomaly: 0.0,
        };
        let m = 1.0;
        let e = orbit.eccentric_anomaly_from_mean(m);
        let m_back = orbit.mean_anomaly_from_eccentric(e);
        assert!((m - m_back).abs() < 1e-10);
    }
}
