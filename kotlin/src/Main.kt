package orrery

import orrery.model.*
import orrery.display.*

fun main() {
    val bodies = SolarSystem.create().toMutableList()
    println("Orrery Visualization")
    println("====================")
    println("Bodies loaded: ${bodies.size}")

    val renderer = AsciiRenderer(ViewPort(scale = 5e-10))
    val tracer = OrbitTracer()
    val stats = SimulationStats()

    stats.update(bodies)
    println("\nInitial state:")
    println(renderer.renderWithLabels(bodies))

    println("\n${stats.summary()}")
    println("\nSimulation ready. Use the Rust engine for n-body integration.")
}
