import java.util.Locale

plugins {
    id("com.android.application")
}

android {
    namespace = "com.NextMU"
    compileSdk = 34
    ndkVersion = "26.1.10909125"

    defaultConfig {
        applicationId = "com.NextMU"
        minSdk = 24
        targetSdk = 34
        versionCode = 1
        versionName = "0.0.1"
        externalNativeBuild {
            cmake {
                arguments(
                    "-DCMAKE_VERBOSE_MAKEFILE=1"
                )
            }
        }
    }

    externalNativeBuild {
        cmake {
            path("../../../CMakeLists.txt")
            version = "3.28.1"
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(getDefaultProguardFile("proguard-android-optimize.txt"), "proguard-rules.pro")
        }
        debug {
            isDebuggable = true
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_1_8
        targetCompatibility = JavaVersion.VERSION_1_8
    }
    buildFeatures {
        viewBinding = true
    }
    lint {
        abortOnError = false
    }

    applicationVariants.all {
        tasks.findByName("compile${name.replaceFirstChar { if (it.isLowerCase()) it.titlecase(Locale.getDefault()) else it.toString() }}}Kotlin")
            ?.dependsOn(tasks.findByName("externalNativeBuild${name.replaceFirstChar { if (it.isLowerCase()) it.titlecase(Locale.getDefault()) else it.toString() }}"))
    }

    if (!rootProject.hasProperty("EXCLUDE_NATIVE_LIBS")) {
        sourceSets {
            getByName("main") {
                jniLibs.srcDir("libs")
            }
        }
    }
}

dependencies {
    implementation(fileTree(mapOf("dir" to "libs", "include" to listOf("*.jar"))))
}