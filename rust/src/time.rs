use std::fmt;

#[derive(Debug, Clone, Copy)]
pub struct SimTime {
    seconds: f64,
}

impl SimTime {
    pub fn zero() -> Self {
        Self { seconds: 0.0 }
    }

    pub fn from_seconds(s: f64) -> Self {
        Self { seconds: s }
    }

    pub fn from_days(d: f64) -> Self {
        Self { seconds: d * 86400.0 }
    }

    pub fn from_years(y: f64) -> Self {
        Self { seconds: y * 365.25 * 86400.0 }
    }

    pub fn as_seconds(&self) -> f64 {
        self.seconds
    }

    pub fn as_days(&self) -> f64 {
        self.seconds / 86400.0
    }

    pub fn as_years(&self) -> f64 {
        self.seconds / (365.25 * 86400.0)
    }

    pub fn advance(&mut self, dt: f64) {
        self.seconds += dt;
    }
}

impl fmt::Display for SimTime {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let years = self.as_years();
        if years.abs() >= 1.0 {
            write!(f, "{:.2} years", years)
        } else {
            let days = self.as_days();
            if days.abs() >= 1.0 {
                write!(f, "{:.2} days", days)
            } else {
                write!(f, "{:.2} seconds", self.seconds)
            }
        }
    }
}

#[derive(Debug, Clone, Copy)]
pub struct TimeStep {
    pub dt: f64,
    pub min_dt: f64,
    pub max_dt: f64,
    pub tolerance: f64,
}

impl TimeStep {
    pub fn fixed(dt: f64) -> Self {
        Self {
            dt,
            min_dt: dt,
            max_dt: dt,
            tolerance: 1e-6,
        }
    }

    pub fn adaptive(initial_dt: f64, min_dt: f64, max_dt: f64, tolerance: f64) -> Self {
        Self { dt: initial_dt, min_dt, max_dt, tolerance }
    }

    pub fn adjust(&mut self, error: f64) {
        if error > self.tolerance {
            self.dt = (self.dt * 0.5).max(self.min_dt);
        } else if error < self.tolerance * 0.1 {
            self.dt = (self.dt * 2.0).min(self.max_dt);
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_time_conversions() {
        let t = SimTime::from_years(1.0);
        assert!((t.as_days() - 365.25).abs() < 1e-10);
    }

    #[test]
    fn test_adaptive_timestep_shrink() {
        let mut ts = TimeStep::adaptive(1.0, 0.01, 100.0, 1e-6);
        ts.adjust(1.0);
        assert!(ts.dt < 1.0);
    }
}
