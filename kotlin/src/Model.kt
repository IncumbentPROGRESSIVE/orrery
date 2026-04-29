package orrery.model

import kotlin.math.*

data class Vec3(val x: Double = 0.0, val y: Double = 0.0, val z: Double = 0.0) {
    fun magnitude() = sqrt(x * x + y * y + z * z)
    fun normalized(): Vec3 {
        val m = magnitude()
        return if (m == 0.0) ZERO else Vec3(x / m, y / m, z / m)
    }
    fun dot(other: Vec3) = x * other.x + y * other.y + z * other.z
    fun cross(other: Vec3) = Vec3(
        y * other.z - z * other.y,
        z * other.x - x * other.z,
        x * other.y - y * other.x
    )
    operator fun plus(other: Vec3) = Vec3(x + other.x, y + other.y, z + other.z)
    operator fun minus(other: Vec3) = Vec3(x - other.x, y - other.y, z - other.z)
    operator fun times(s: Double) = Vec3(x * s, y * s, z * s)
    fun distanceTo(other: Vec3) = (this - other).magnitude()

    companion object {
        val ZERO = Vec3(0.0, 0.0, 0.0)
    }
}

enum class BodyType { STAR, PLANET, MOON, ASTEROID, COMET }

data class CelestialBody(
    val name: String,
    val type: BodyType,
    val mass: Double,
    val radius: Double,
    val position: Vec3 = Vec3.ZERO,
    val velocity: Vec3 = Vec3.ZERO,
    val parentIndex: Int? = null
) {
    fun kineticEnergy() = 0.5 * mass * velocity.magnitude().pow(2)
    fun momentum() = velocity * mass
    fun surfaceGravity(): Double {
        if (radius == 0.0) return 0.0
        return G * mass / (radius * radius)
    }
    fun escapeVelocity(): Double {
        if (radius == 0.0) return 0.0
        return sqrt(2.0 * G * mass / radius)
    }

    companion object {
        const val G = 6.674e-11
    }
}

data class OrbitalElements(
    val semiMajorAxis: Double,
    val eccentricity: Double,
    val inclination: Double = 0.0,
    val longitudeAscending: Double = 0.0,
    val argumentPeriapsis: Double = 0.0,
    val trueAnomaly: Double = 0.0
) {
    fun periapsis() = semiMajorAxis * (1.0 - eccentricity)
    fun apoapsis() = semiMajorAxis * (1.0 + eccentricity)
    fun period(centralMass: Double): Double {
        return 2.0 * PI * sqrt(semiMajorAxis.pow(3) / (CelestialBody.G * centralMass))
    }
}

object SolarSystem {
    fun create(): List<CelestialBody> = listOf(
        CelestialBody("Sun", BodyType.STAR, 1.989e30, 6.957e8),
        CelestialBody("Earth", BodyType.PLANET, 5.972e24, 6.371e6,
            Vec3(1.496e11, 0.0, 0.0), Vec3(0.0, 2.978e4, 0.0), 0),
        CelestialBody("Mars", BodyType.PLANET, 6.417e23, 3.389e6,
            Vec3(2.279e11, 0.0, 0.0), Vec3(0.0, 2.407e4, 0.0), 0),
        CelestialBody("Jupiter", BodyType.PLANET, 1.898e27, 6.991e7,
            Vec3(7.785e11, 0.0, 0.0), Vec3(0.0, 1.307e4, 0.0), 0),
    )
}
