package orrery.display

import orrery.model.*
import kotlin.math.*

data class ViewPort(
    val width: Int = 120,
    val height: Int = 40,
    val scale: Double = 1e-9,
    val centerX: Double = 0.0,
    val centerY: Double = 0.0
) {
    fun worldToScreen(pos: Vec3): Pair<Int, Int> {
        val sx = ((pos.x - centerX) * scale + width / 2).toInt()
        val sy = ((pos.y - centerY) * scale + height / 2).toInt()
        return sx to sy
    }

    fun isVisible(sx: Int, sy: Int) = sx in 0 until width && sy in 0 until height
}

class AsciiRenderer(private val viewport: ViewPort = ViewPort()) {
    private val symbolMap = mapOf(
        BodyType.STAR to '*',
        BodyType.PLANET to 'O',
        BodyType.MOON to 'o',
        BodyType.ASTEROID to '.',
        BodyType.COMET to '~'
    )

    fun render(bodies: List<CelestialBody>): String {
        val grid = Array(viewport.height) { CharArray(viewport.width) { ' ' } }

        for (body in bodies) {
            val (sx, sy) = viewport.worldToScreen(body.position)
            if (viewport.isVisible(sx, sy)) {
                grid[sy][sx] = symbolMap[body.type] ?: '?'
            }
        }

        return grid.joinToString("\n") { String(it) }
    }

    fun renderWithLabels(bodies: List<CelestialBody>): String {
        val frame = render(bodies)
        val legend = bodies.joinToString("\n") { body ->
            val symbol = symbolMap[body.type] ?: '?'
            "  $symbol ${body.name} (${formatDistance(body.position.magnitude())})"
        }
        return "$frame\n\nLegend:\n$legend"
    }

    private fun formatDistance(meters: Double): String = when {
        meters >= 1.496e11 -> String.format("%.2f AU", meters / 1.496e11)
        meters >= 1e6 -> String.format("%.2f Mm", meters / 1e6)
        else -> String.format("%.0f m", meters)
    }
}

class OrbitTracer(private val trailLength: Int = 200) {
    private val trails = mutableMapOf<String, MutableList<Vec3>>()

    fun record(bodies: List<CelestialBody>) {
        for (body in bodies) {
            val trail = trails.getOrPut(body.name) { mutableListOf() }
            trail.add(body.position)
            if (trail.size > trailLength) trail.removeAt(0)
        }
    }

    fun getTrail(name: String): List<Vec3> = trails[name] ?: emptyList()

    fun clear() = trails.clear()
}

class SimulationStats {
    private var stepCount = 0L
    private var totalEnergy = 0.0
    private var initialEnergy: Double? = null

    fun update(bodies: List<CelestialBody>) {
        stepCount++
        totalEnergy = computeTotalEnergy(bodies)
        if (initialEnergy == null) initialEnergy = totalEnergy
    }

    fun energyDrift(): Double {
        val init = initialEnergy ?: return 0.0
        return if (init != 0.0) abs((totalEnergy - init) / init) else 0.0
    }

    fun summary() = "Steps: $stepCount | Energy: %.6e | Drift: %.2e".format(totalEnergy, energyDrift())

    private fun computeTotalEnergy(bodies: List<CelestialBody>): Double {
        var kinetic = bodies.sumOf { it.kineticEnergy() }
        var potential = 0.0
        for (i in bodies.indices) {
            for (j in i + 1 until bodies.size) {
                val r = bodies[i].position.distanceTo(bodies[j].position)
                if (r > 0.0) potential -= CelestialBody.G * bodies[i].mass * bodies[j].mass / r
            }
        }
        return kinetic + potential
    }
}
