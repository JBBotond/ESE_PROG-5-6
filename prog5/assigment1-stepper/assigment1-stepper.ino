/*
 * Arduino Stepper Motor Control - Structured Programming
 * Uses Arduino libraries with structured function design
 * Compatible with 8-bit 16MHz Arduino boards
 * 
 * v1.0
 * Aug 2025
 * Embedded Programming (Prog 5/6)
 * johan.korten@han.nl
 * MIC2 Style
 */

#include "stepper.h"

// --- Pin definitions ---
const uint8_t STEP_PIN = 7;  // TB6600 PUL
const uint8_t DIR_PIN = 6;   // TB6600 DIR  
const uint8_t EN_PIN = 5;    // TB6600 ENA

// --- Motor/driver parameters ---
const int FULL_STEPS_PER_REV = 200;
const int MICROSTEPS = 16;
const long STEPS_PER_REV = (long)FULL_STEPS_PER_REV * MICROSTEPS;

// --- Speed control ---
const float TARGET_RPM = 120.0;
const float ACCEL_RPM_S = 300.0;

// Timing calculations
const float TARGET_SPS = (TARGET_RPM / 60.0f) * (float)STEPS_PER_REV;
const unsigned long TARGET_STEP_INTERVAL = 1000000UL / (unsigned long)TARGET_SPS; // microseconds
const unsigned long ACCEL_STEP_SIZE = 50; // microseconds
const uint8_t DECEL_MULTIPLIER = 5;

// --- Motion state variables ---
volatile long currentPosition = 0;
volatile long targetPosition = STEPS_PER_REV;
volatile unsigned long currentStepInterval = TARGET_STEP_INTERVAL * 10; // start slow

void setup() {
  Serial.begin(115200);
  performSystemSetup();
  Serial.println("Structured Arduino stepper control started");
}

void loop() {
  executeMotionControl();
}

