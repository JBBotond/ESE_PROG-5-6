#include <Arduino.h>

//external variables
extern const uint8_t STEP_PIN;
extern const uint8_t DIR_PIN;
extern const uint8_t EN_PIN;

extern const int FULL_STEPS_PER_REV;
extern const int MICROSTEPS;
extern const long STEPS_PER_REV;

extern const float TARGET_RPM;
extern const float ACCEL_RPM_S;

extern const float TARGET_SPS;
extern const unsigned long TARGET_STEP_INTERVAL;
extern const unsigned long ACCEL_STEP_SIZE;
extern const uint8_t DECEL_MULTIPLIER;

extern volatile long currentPosition;
extern volatile long targetPosition;
extern volatile unsigned long currentStepInterval;

// --- Function prototypes ---
// System setup
void performSystemSetup(void);
void initializeHardware(void);
void initializePins(void);
void enableDriverOutput(void);
void initializeMotionParameters(void);

// Main motion control
void executeMotionControl(void);
boolean isTimeForNextStep(unsigned long currentTime, unsigned long *lastStepTime);
void executeStepSequence(unsigned long currentTime, unsigned long *lastStepTime);
void handleTargetReached(void);

// Position management
void updatePosition(void);
boolean hasReachedTarget(void);
void reverseDirection(void);
long calculateNewTarget(void);
boolean isMovingForward(void);

// Step generation
void makeStep(void);
void generateStepPulse(void);
void createHighPulse(void);
void createLowPulse(void);

// Direction control
void updateDirection(void);
void setDirectionForward(void);
void setDirectionReverse(void);
void waitForDirectionSetup(void);

// Speed/acceleration control
void handleAcceleration(void);
void accelerateMotor(void);
boolean canAccelerate(void);
void resetSpeedForDirectionChange(void);

// Timing utilities
unsigned long getCurrentTime(void);
boolean hasTimeElapsed(unsigned long currentTime, unsigned long previousTime, unsigned long interval);

// Utility functions
void printMotionStatus(void);
void debugPosition(void);
