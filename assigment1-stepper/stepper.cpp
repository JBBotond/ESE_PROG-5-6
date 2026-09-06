#include "stepper.h"
#include <Arduino.h>

// === SYSTEM SETUP FUNCTIONS ===

void performSystemSetup(void) {
  initializeHardware();
  initializeMotionParameters();
}

void initializeHardware(void) {
  initializePins();
  enableDriverOutput();
}

void initializePins(void) {
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(EN_PIN, OUTPUT);
  
  digitalWrite(STEP_PIN, LOW);  // STEP low
  digitalWrite(DIR_PIN, LOW);   // DIR low (forward)
}

void enableDriverOutput(void) {
  digitalWrite(EN_PIN, HIGH);   // EN high (enable driver)
}

void initializeMotionParameters(void) {
  currentPosition = 0;
  targetPosition = STEPS_PER_REV;
  currentStepInterval = TARGET_STEP_INTERVAL * 10;
  updateDirection();
}

// === MAIN MOTION CONTROL ===

void executeMotionControl(void) {
  static unsigned long lastStepTime = 0;
  unsigned long currentTime = getCurrentTime();
  
  if (isTimeForNextStep(currentTime, &lastStepTime)) {
    if (hasReachedTarget()) {
      handleTargetReached();
    } else {
      executeStepSequence(currentTime, &lastStepTime);
    }
  }
}

boolean isTimeForNextStep(unsigned long currentTime, unsigned long *lastStepTime) {
  return hasTimeElapsed(currentTime, *lastStepTime, currentStepInterval);
}

void executeStepSequence(unsigned long currentTime, unsigned long *lastStepTime) {
  makeStep();
  *lastStepTime = currentTime;
  updatePosition();
  handleAcceleration();
}

void handleTargetReached(void) {
  debugPosition();
  reverseDirection();
  resetSpeedForDirectionChange();
  updateDirection();
}

// === POSITION MANAGEMENT ===

void updatePosition(void) {
  if (isMovingForward()) {
    currentPosition++;
  } else {
    currentPosition--;
  }
}

boolean hasReachedTarget(void) {
  return (currentPosition == targetPosition);
}

void reverseDirection(void) {
  targetPosition = calculateNewTarget();
}

long calculateNewTarget(void) {
  return -currentPosition; // ping-pong motion
}

boolean isMovingForward(void) {
  return (targetPosition > currentPosition);
}

// === STEP GENERATION ===

void makeStep(void) {
  generateStepPulse();
}

void generateStepPulse(void) {
  createHighPulse();
  createLowPulse();
}

void createHighPulse(void) {
  digitalWrite(STEP_PIN, HIGH);
  delayMicroseconds(5); // Minimum pulse width for TB6600
}

void createLowPulse(void) {
  digitalWrite(STEP_PIN, LOW);
  delayMicroseconds(5); // Minimum low time
}

// === DIRECTION CONTROL ===

void updateDirection(void) {
  if (isMovingForward()) {
    setDirectionForward();
  } else {
    setDirectionReverse();
  }
  waitForDirectionSetup();
}

void setDirectionForward(void) {
  digitalWrite(DIR_PIN, LOW);  // DIR low = forward
}

void setDirectionReverse(void) {
  digitalWrite(DIR_PIN, HIGH); // DIR high = reverse
}

void waitForDirectionSetup(void) {
  delayMicroseconds(20);       // Direction setup time for TB6600
}

// === SPEED/ACCELERATION CONTROL ===

void handleAcceleration(void) {
  if (canAccelerate()) {
    accelerateMotor();
  }
}

boolean canAccelerate(void) {
  return (currentStepInterval > TARGET_STEP_INTERVAL);
}

void accelerateMotor(void) {
  currentStepInterval -= ACCEL_STEP_SIZE;
  if (currentStepInterval < TARGET_STEP_INTERVAL) {
    currentStepInterval = TARGET_STEP_INTERVAL;
  }
}

void resetSpeedForDirectionChange(void) {
  currentStepInterval = TARGET_STEP_INTERVAL * DECEL_MULTIPLIER;
}

// === TIMING UTILITIES ===

unsigned long getCurrentTime(void) {
  return micros();
}

boolean hasTimeElapsed(unsigned long currentTime, unsigned long previousTime, unsigned long interval) {
  return (currentTime - previousTime) >= interval;
}

// === UTILITY FUNCTIONS ===

void printMotionStatus(void) {
  Serial.print("Position: ");
  Serial.print(currentPosition);
  Serial.print(" Target: ");
  Serial.print(targetPosition);
  Serial.print(" Interval: ");
  Serial.println(currentStepInterval);
}

void debugPosition(void) {
  Serial.print("Direction changed. Position: ");
  Serial.print(currentPosition);
  Serial.print(" New target: ");
  Serial.println(targetPosition);
}
