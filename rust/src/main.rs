use orrery::Simulation;
use orrery::celestial_body::create_solar_system;
use orrery::simulation::Integrator;

fn main() {
    let bodies = create_solar_system();
    println!("Orrery Simulation Engine");
    println!("========================");
    println!("Bodies: {}", bodies.len());

    for body in &bodies {
        println!(
            "  {} - mass: {:.3e} kg, pos: ({:.3e}, {:.3e}, {:.3e})",
            body.name, body.mass, body.position.x, body.position.y, body.position.z
        );
    }

    let mut sim = Simulation::new(bodies, 3600.0)
        .with_integrator(Integrator::RungeKutta4);

    let initial_energy = sim.total_energy();
    println!("\nInitial total energy: {:.6e} J", initial_energy);

    let one_year = 365.25 * 86400.0;
    sim.run_for(one_year);

    let final_energy = sim.total_energy();
    let drift = ((final_energy - initial_energy) / initial_energy).abs();

    println!("Simulated time: {}", sim.time);
    println!("Steps taken: {}", sim.step_count());
    println!("Final total energy: {:.6e} J", final_energy);
    println!("Energy drift: {:.2e}", drift);

    println!("\nFinal positions:");
    for body in &sim.bodies {
        println!(
            "  {} -> ({:.3e}, {:.3e}, {:.3e})",
            body.name, body.position.x, body.position.y, body.position.z
        );
    }
}
