#include <Servo.h>

// ── Pin definitions ────────────────────────────────────────
const int PIN_SERVO_SHOULDER = 9;
const int PIN_SERVO_ELBOW = 10;
const int PIN_SERVO_GRIPPER = 11;
const int PIN_MOTOR_IN1 = 5;
const int PIN_MOTOR_IN2 = 6;
const int PIN_MOTOR_ENA = 3;
const int PIN_IR_SENSOR = 7;

// ── Servo positions (degrees) ──────────────────────────────
// Find these manually BEFORE running automation — see guide
const int HOME_SHOULDER = 90;
const int HOME_ELBOW = 90;

const int PICK_SHOULDER = 60; // tune this for your arm
const int PICK_ELBOW = 150;   // tune this for your arm

const int BIN_A_SHOULDER = 30;
const int BIN_A_ELBOW = 110;

const int GRIPPER_OPEN = 20;
const int GRIPPER_CLOSED = 80;

// ── Motor settings ─────────────────────────────────────────
const int BELT_SPEED = 130; // 0–255. Start low, increase if needed.

// ── Timing ────────────────────────────────────────────────
const int GRIP_DELAY_MS = 300;
const int MOVE_DELAY_MS = 600; // wait for servo to reach position

// ── State machine ─────────────────────────────────────────
enum State
{
    IDLE,
    CONVEYING,
    PICKING,
    GRIPPING,
    PLACING,
    RELEASING,
    HOMING
};

State currentState = IDLE;

// ── Servo objects ─────────────────────────────────────────
Servo shoulder;
Servo elbow;
Servo gripper;

// ── setup() ───────────────────────────────────────────────
void setup()
{
    shoulder.attach(PIN_SERVO_SHOULDER);
    elbow.attach(PIN_SERVO_ELBOW);
    gripper.attach(PIN_SERVO_GRIPPER);

    pinMode(PIN_MOTOR_IN1, OUTPUT);
    pinMode(PIN_MOTOR_IN2, OUTPUT);
    pinMode(PIN_MOTOR_ENA, OUTPUT);
    pinMode(PIN_IR_SENSOR, INPUT_PULLUP); // PULLUP avoids floating reads

    Serial.begin(9600);
    Serial.println("System ready.");

    // Move arm to home on startup
    shoulder.write(HOME_SHOULDER);
    elbow.write(HOME_ELBOW);
    gripper.write(GRIPPER_OPEN);
    delay(1000); // give servos time to reach home

    currentState = CONVEYING; // start the loop
}

// ── loop() ────────────────────────────────────────────────
// This is the heart of the state machine.
// It runs thousands of times per second, checking which
// state we're in and calling the right function.

void loop()
{
    switch (currentState)
    {
    case CONVEYING:
        doConveying();
        break;
    case PICKING:
        doPicking();
        break;
    case GRIPPING:
        doGripping();
        break;
    case PLACING:
        doPlacing();
        break;
    case RELEASING:
        doReleasing();
        break;
    case HOMING:
        doHoming();
        break;
    }
}

// ── State functions ────────────────────────────────────────

void doConveying()
{
    Serial.println("STATE: CONVEYING");
    beltForward();

    // Wait until IR sensor detects an object
    // IR sensor outputs LOW when object is detected (with PULLUP)
    while (digitalRead(PIN_IR_SENSOR) == HIGH)
    {
        // keep belt running, do nothing else
    }

    // Object detected — stop belt and move to next state
    beltStop();
    delay(100); // brief pause to let object settle
    currentState = PICKING;
}

void doPicking()
{
    Serial.println("STATE: PICKING");

    // Move arm to pick position above the object
    shoulder.write(PICK_SHOULDER);
    elbow.write(PICK_ELBOW);
    delay(MOVE_DELAY_MS); // wait for servos to arrive

    currentState = GRIPPING;
}

void doGripping()
{
    Serial.println("STATE: GRIPPING");

    gripper.write(GRIPPER_CLOSED);
    delay(GRIP_DELAY_MS); // give gripper time to close fully

    currentState = PLACING;
}

void doPlacing()
{
    Serial.println("STATE: PLACING");

    // Move to bin A (hardcoded for v1 — add sensor later for sorting)
    shoulder.write(BIN_A_SHOULDER);
    elbow.write(BIN_A_ELBOW);
    delay(MOVE_DELAY_MS);

    currentState = RELEASING;
}

void doReleasing()
{
    Serial.println("STATE: RELEASING");

    gripper.write(GRIPPER_OPEN);
    delay(GRIP_DELAY_MS);

    currentState = HOMING;
}

void doHoming()
{
    Serial.println("STATE: HOMING");

    shoulder.write(HOME_SHOULDER);
    elbow.write(HOME_ELBOW);
    delay(MOVE_DELAY_MS);

    currentState = CONVEYING; // loop restarts
}

// ── Helper functions ──────────────────────────────────────

void beltForward()
{
    digitalWrite(PIN_MOTOR_IN1, HIGH);
    digitalWrite(PIN_MOTOR_IN2, LOW);
    analogWrite(PIN_MOTOR_ENA, BELT_SPEED);
}

void beltStop()
{
    digitalWrite(PIN_MOTOR_IN1, LOW);
    digitalWrite(PIN_MOTOR_IN2, LOW);
    analogWrite(PIN_MOTOR_ENA, 0);
}