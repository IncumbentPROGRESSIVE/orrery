plugins {
    kotlin("jvm") version "1.9.0"
    application
}

repositories {
    mavenCentral()
}

application {
    mainClass.set("orrery.MainKt")
}
