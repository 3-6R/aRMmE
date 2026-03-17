// ============================================================
//   6-AXIS ROBOTIC Stranger — Arduino Uno
//   7 Servos total (Axis 6 uses 2 servos working together)
//
//   TWO INPUT MODES — comment out the one you find worthless:
//     MODE 1: Keyboard fingering via terminal
//     MODE 2: Rawdawging the Arduino pins
//
//   Author note: Read the comments above each section to
//   understand what the fuck is even going on.
// ============================================================

#include <Servo.h>   // Include the built-in Arduino Servo Bullshit

// ============================================================
//   CHOOSE YOUR WEAPON
//   Comment out (#//) the mode you find worthless
// ============================================================
     #define KEYBOARD_MODE   // <-- Keyboard fucking via Serial terminal
//   #define GPIO_MODE    // <-- Physical buttons stimulating the pins


// ============================================================
//   SERVO PIN ASSIGNMENTS
//   These are the digital PWM pins on the Arduino Uno.
//   Change these numbers if you wire your servos differently.
// ============================================================
#define PIN_AXIS1   2   // Axis 1 — Base rotation (left/right)
#define PIN_AXIS2   3   // Axis 2 — Shoulder (up/down)
#define PIN_AXIS3   4   // Axis 3 — Elbow (up/down)
#define PIN_AXIS4   5   // Axis 4 — Wrist pitch (up/down)
#define PIN_AXIS5   6   // Axis 5 — Wrist roll (twist)
#define PIN_AXIS6A  9   // Axis 6 — Gripper servo A (open/close)
#define PIN_AXIS6B  10  // Axis 6 — Gripper servo B (mirrors A)


// ============================================================
//   SERVO ANGLE LIMITS
//   These prevent the arm from moving beyond safe positions.
//   Adjust MIN/MAX values to match your physical hardware.
// ============================================================
#define SERVO_MIN    10   // Minimum angle (degrees) for all servos
#define SERVO_MAX   170   // Maximum angle (degrees) for all servos
#define SERVO_HOME   90   // Starting/home angle for all servos
#define STEP_SIZE     5   // How many degrees to move per key press


// ============================================================
//   GPIO BUTTON PIN ASSIGNMENTS (only used in GPIO_MODE)
//   Each axis needs two buttons: one for + direction, one for -
//   Wire buttons between these pins and GND. The INPUT_PULLUP
//   setting means the pin reads HIGH when open, LOW when pressed.
// ============================================================
#ifdef GPIO_MODE
  #define BTN_AXIS1_POS  A0   // Axis 1 move positive (rotate right)
  #define BTN_AXIS1_NEG  A1   // Axis 1 move negative (rotate left)
  #define BTN_AXIS2_POS  A2   // Axis 2 move positive (shoulder up)
  #define BTN_AXIS2_NEG  A3   // Axis 2 move negative (shoulder down)
  #define BTN_AXIS3_POS  A4   // Axis 3 move positive (elbow up)
  #define BTN_AXIS3_NEG  A5   // Axis 3 move negative (elbow down)
  #define BTN_AXIS4_POS  11   // Axis 4 move positive (wrist up)
  #define BTN_AXIS4_NEG  12   // Axis 4 move negative (wrist down)
  #define BTN_AXIS5_POS  13   // Axis 5 move positive (wrist roll CW)
  #define BTN_AXIS5_NEG   7   // Axis 5 move negative (wrist roll CCW)
  #define BTN_AXIS6_POS   8   // Axis 6 move positive (gripper open)
  #define BTN_AXIS6_NEG  14   // Axis 6 move negative (gripper close)
                              // Note: pin 14 = A0 as digital, adjust if needed
#endif


// ============================================================
//   SERVO OBJECTS
//   One Servo object per physical servo motor
// ============================================================
Servo axis1;    // Base rotation servo
Servo axis2;    // Shoulder servo
Servo axis3;    // Elbow servo
Servo axis4;    // Wrist pitch servo
Servo axis5;    // Wrist roll servo
Servo axis6a;   // Gripper servo A
Servo axis6b;   // Gripper servo B (moves opposite to A for claw action)


// ============================================================
//   CURRENT ANGLE TRACKING
//   These variables store the current angle of each axis
//   so we know where it is before we move it.
// ============================================================
int angle1 = SERVO_HOME;   // Current angle of Axis 1
int angle2 = SERVO_HOME;   // Current angle of Axis 2
int angle3 = SERVO_HOME;   // Current angle of Axis 3
int angle4 = SERVO_HOME;   // Current angle of Axis 4
int angle5 = SERVO_HOME;   // Current angle of Axis 5
int angle6 = SERVO_HOME;   // Current angle of Axis 6 (both gripper servos)


// ============================================================
//   moveServo() — SAFE MOVEMENT HELPER FUNCTION
//   This function moves a servo to a new angle, but first
//   checks that the angle is within the safe MIN/MAX range.
//   Pass it: the Servo object, the current angle variable
//   (by reference so it updates automatically), and the change.
// ============================================================
void moveServo(Servo &srv, int &currentAngle, int delta) {
  int newAngle = currentAngle + delta;   // Calculate the new target angle

  // Clamp the angle so it never goes below MIN or above MAX
  newAngle = constrain(newAngle, SERVO_MIN, SERVO_MAX);

  currentAngle = newAngle;    // Update the stored angle
  srv.write(currentAngle);    // Send the angle command to the servo
}


// ============================================================
//   moveGripper() — SPECIAL FUNCTION FOR THE DUAL GRIPPER
//   The gripper uses two servos. One opens as the other closes.
//   Servo A moves normally; Servo B mirrors it (moves opposite).
//   This creates a claw-like pinching motion.
// ============================================================
void moveGripper(int delta) {
  int newAngle = angle6 + delta;
  newAngle = constrain(newAngle, SERVO_MIN, SERVO_MAX);

  angle6 = newAngle;

  axis6a.write(angle6);              // Servo A moves to the angle
  axis6b.write(180 - angle6);        // Servo B mirrors it (opposite direction)
}


// ============================================================
//   printStatus() — SHOW CURRENT ANGLES IN SERIAL MONITOR
//   Prints a quick summary of all axis positions.
//   Helpful for knowing where your arm currently is.
// ============================================================
void printStatus() {
  Serial.println(F("--- Current Arm Angles ---"));
  Serial.print(F("Axis 1 (Base):        ")); Serial.println(angle1);
  Serial.print(F("Axis 2 (Shoulder):    ")); Serial.println(angle2);
  Serial.print(F("Axis 3 (Elbow):       ")); Serial.println(angle3);
  Serial.print(F("Axis 4 (Wrist Pitch): ")); Serial.println(angle4);
  Serial.print(F("Axis 5 (Wrist Roll):  ")); Serial.println(angle5);
  Serial.print(F("Axis 6 (Gripper):     ")); Serial.println(angle6);
  Serial.println(F("--------------------------"));
}


// ============================================================
//   printKeyboardHelp() — SHOW CONTROL KEYS IN SERIAL MONITOR
//   Printed once at startup so you remember which key does what.
// ============================================================
void printKeyboardHelp() {
  Serial.println(F("=============================="));
  Serial.println(F("  6-AXIS ARM — KEYBOARD MODE  "));
  Serial.println(F("=============================="));
  Serial.println(F("  Q/A  = Axis 1 Base +/-"));
  Serial.println(F("  W/S  = Axis 2 Shoulder +/-"));
  Serial.println(F("  E/D  = Axis 3 Elbow +/-"));
  Serial.println(F("  R/F  = Axis 4 Wrist Pitch +/-"));
  Serial.println(F("  T/G  = Axis 5 Wrist Roll +/-"));
  Serial.println(F("  Y/H  = Axis 6 Gripper Open/Close"));
  Serial.println(F("  0    = Return ALL axes to HOME (90 deg)"));
  Serial.println(F("  ?    = Show this help menu"));
  Serial.println(F("  *    = Print current angles"));
  Serial.println(F("=============================="));
}


// ============================================================
//   homeAllAxes() — MOVE EVERY SERVO TO THE HOME POSITION
//   Useful to reset the arm to a known neutral position.
// ============================================================
void homeAllAxes() {
  angle1 = SERVO_HOME; axis1.write(angle1);
  angle2 = SERVO_HOME; axis2.write(angle2);
  angle3 = SERVO_HOME; axis3.write(angle3);
  angle4 = SERVO_HOME; axis4.write(angle4);
  angle5 = SERVO_HOME; axis5.write(angle5);
  angle6 = SERVO_HOME;
  axis6a.write(angle6);
  axis6b.write(180 - angle6);   // Mirror the gripper servo B
  Serial.println(F("All axes returned to HOME (90 degrees)."));
}


// ============================================================
//   setup() — RUNS ONCE AT STARTUP
//   Attaches servos to pins, sets GPIO button pins if needed,
//   and moves everything to the home position.
// ============================================================
void setup() {

  // Start Serial communication at 9600 baud
  // Open Serial Monitor at 9600 baud to see output
  Serial.begin(9600);

  // --- Attach each Servo object to its digital pin ---
  axis1.attach(PIN_AXIS1);
  axis2.attach(PIN_AXIS2);
  axis3.attach(PIN_AXIS3);
  axis4.attach(PIN_AXIS4);
  axis5.attach(PIN_AXIS5);
  axis6a.attach(PIN_AXIS6A);
  axis6b.attach(PIN_AXIS6B);

  // --- Move all servos to the home position on startup ---
  homeAllAxes();
  delay(500);   // Short pause to let servos reach home before accepting input

  // --- GPIO MODE SETUP (only runs if GPIO_MODE is defined) ---
#ifdef GPIO_MODE
  // Set all button pins as inputs with internal pull-up resistors.
  // This means: pin reads HIGH when button is NOT pressed,
  //             pin reads LOW when button IS pressed (connected to GND).
  pinMode(BTN_AXIS1_POS, INPUT_PULLUP);
  pinMode(BTN_AXIS1_NEG, INPUT_PULLUP);
  pinMode(BTN_AXIS2_POS, INPUT_PULLUP);
  pinMode(BTN_AXIS2_NEG, INPUT_PULLUP);
  pinMode(BTN_AXIS3_POS, INPUT_PULLUP);
  pinMode(BTN_AXIS3_NEG, INPUT_PULLUP);
  pinMode(BTN_AXIS4_POS, INPUT_PULLUP);
  pinMode(BTN_AXIS4_NEG, INPUT_PULLUP);
  pinMode(BTN_AXIS5_POS, INPUT_PULLUP);
  pinMode(BTN_AXIS5_NEG, INPUT_PULLUP);
  pinMode(BTN_AXIS6_POS, INPUT_PULLUP);
  pinMode(BTN_AXIS6_NEG, INPUT_PULLUP);

  Serial.println(F("=== GPIO BUTTON MODE ACTIVE ==="));
  Serial.println(F("Press buttons to move each axis."));
  Serial.println(F("Hold button to continuously move."));
#endif

  // --- KEYBOARD MODE STARTUP MESSAGE ---
#ifdef KEYBOARD_MODE
  printKeyboardHelp();
#endif
}


// ============================================================
//   loop() — RUNS CONTINUOUSLY FOREVER
//   This is the main program loop. It checks for input
//   (keyboard OR buttons) and moves servos accordingly.
// ============================================================
void loop() {

// ===========================================================
//   ██████╗  KEYBOARD MODE SECTION
//   Only compiles and runs if KEYBOARD_MODE is defined above.
// ===========================================================
#ifdef KEYBOARD_MODE

  // Check if a key has been typed and sent in the Serial Monitor
  if (Serial.available() > 0) {

    char key = Serial.read();   // Read the incoming character

    // Convert uppercase to lowercase so Q and q both work
    key = tolower(key);

    // React to each key press:
    switch (key) {

      // --- AXIS 1: Base Rotation ---
      case 'q': moveServo(axis1, angle1, +STEP_SIZE);
                Serial.print(F("Axis 1 +  Angle: ")); Serial.println(angle1); break;
      case 'a': moveServo(axis1, angle1, -STEP_SIZE);
                Serial.print(F("Axis 1 -  Angle: ")); Serial.println(angle1); break;

      // --- AXIS 2: Shoulder ---
      case 'w': moveServo(axis2, angle2, +STEP_SIZE);
                Serial.print(F("Axis 2 +  Angle: ")); Serial.println(angle2); break;
      case 's': moveServo(axis2, angle2, -STEP_SIZE);
                Serial.print(F("Axis 2 -  Angle: ")); Serial.println(angle2); break;

      // --- AXIS 3: Elbow ---
      case 'e': moveServo(axis3, angle3, +STEP_SIZE);
                Serial.print(F("Axis 3 +  Angle: ")); Serial.println(angle3); break;
      case 'd': moveServo(axis3, angle3, -STEP_SIZE);
                Serial.print(F("Axis 3 -  Angle: ")); Serial.println(angle3); break;

      // --- AXIS 4: Wrist Pitch ---
      case 'r': moveServo(axis4, angle4, +STEP_SIZE);
                Serial.print(F("Axis 4 +  Angle: ")); Serial.println(angle4); break;
      case 'f': moveServo(axis4, angle4, -STEP_SIZE);
                Serial.print(F("Axis 4 -  Angle: ")); Serial.println(angle4); break;

      // --- AXIS 5: Wrist Roll ---
      case 't': moveServo(axis5, angle5, +STEP_SIZE);
                Serial.print(F("Axis 5 +  Angle: ")); Serial.println(angle5); break;
      case 'g': moveServo(axis5, angle5, -STEP_SIZE);
                Serial.print(F("Axis 5 -  Angle: ")); Serial.println(angle5); break;

      // --- AXIS 6: Gripper (dual servo) ---
      case 'y': moveGripper(+STEP_SIZE);
                Serial.print(F("Gripper OPEN  Angle: ")); Serial.println(angle6); break;
      case 'h': moveGripper(-STEP_SIZE);
                Serial.print(F("Gripper CLOSE Angle: ")); Serial.println(angle6); break;

      // --- UTILITY KEYS ---
      case '0': homeAllAxes(); break;           // Return all to home position
      case '?': printKeyboardHelp(); break;     // Show key guide
      case '*': printStatus(); break;           // Show all current angles

      // Ignore newline and carriage return characters (they come with Serial input)
      case '\n': break;
      case '\r': break;

      // Any unrecognised key — give a hint
      default:
        Serial.print(F("Unknown key: '"));
        Serial.print(key);
        Serial.println(F("' — type ? for help."));
        break;
    }
  }

#endif  // END KEYBOARD_MODE


// ===========================================================
//   ██████╗  GPIO BUTTON MODE SECTION
//   Only compiles and runs if GPIO_MODE is defined above.
//   Buttons must be wired between the defined pin and GND.
//   Using INPUT_PULLUP means LOW = pressed, HIGH = released.
// ===========================================================
#ifdef GPIO_MODE

  // --- AXIS 1: Base Rotation buttons ---
  if (digitalRead(BTN_AXIS1_POS) == LOW) {   // Button pressed (LOW = pressed with pullup)
    moveServo(axis1, angle1, +STEP_SIZE);
    delay(100);   // 100ms delay prevents moving too fast while held
  }
  if (digitalRead(BTN_AXIS1_NEG) == LOW) {
    moveServo(axis1, angle1, -STEP_SIZE);
    delay(100);
  }

  // --- AXIS 2: Shoulder buttons ---
  if (digitalRead(BTN_AXIS2_POS) == LOW) {
    moveServo(axis2, angle2, +STEP_SIZE);
    delay(100);
  }
  if (digitalRead(BTN_AXIS2_NEG) == LOW) {
    moveServo(axis2, angle2, -STEP_SIZE);
    delay(100);
  }

  // --- AXIS 3: Elbow buttons ---
  if (digitalRead(BTN_AXIS3_POS) == LOW) {
    moveServo(axis3, angle3, +STEP_SIZE);
    delay(100);
  }
  if (digitalRead(BTN_AXIS3_NEG) == LOW) {
    moveServo(axis3, angle3, -STEP_SIZE);
    delay(100);
  }

  // --- AXIS 4: Wrist Pitch buttons ---
  if (digitalRead(BTN_AXIS4_POS) == LOW) {
    moveServo(axis4, angle4, +STEP_SIZE);
    delay(100);
  }
  if (digitalRead(BTN_AXIS4_NEG) == LOW) {
    moveServo(axis4, angle4, -STEP_SIZE);
    delay(100);
  }

  // --- AXIS 5: Wrist Roll buttons ---
  if (digitalRead(BTN_AXIS5_POS) == LOW) {
    moveServo(axis5, angle5, +STEP_SIZE);
    delay(100);
  }
  if (digitalRead(BTN_AXIS5_NEG) == LOW) {
    moveServo(axis5, angle5, -STEP_SIZE);
    delay(100);
  }

  // --- AXIS 6: Gripper buttons (dual servo) ---
  if (digitalRead(BTN_AXIS6_POS) == LOW) {
    moveGripper(+STEP_SIZE);    // Open gripper
    delay(100);
  }
  if (digitalRead(BTN_AXIS6_NEG) == LOW) {
    moveGripper(-STEP_SIZE);    // Close gripper
    delay(100);
  }

#endif  // END GPIO_MODE

}   // END loop()

// ============================================================
//   END OF FILE
//
//   QUICK SETUP CHECKLIST:
//   1. Wire servos to the pins defined in PIN_AXIS1 through
//      PIN_AXIS6B at the top of this file.
//   2. Power servos from an EXTERNAL 5V supply (not the Uno's
//      5V pin — 7 servos will overdraw it and damage the board).
//      Share the GND between the power supply and the Uno.
//   3. For GPIO mode, wire each button from the defined pin
//      to GND. No resistors needed (INPUT_PULLUP handles it).
//   4. To switch modes: comment out one #define and uncomment
//      the other at the top of the file, then re-upload.
//   5. Open Serial Monitor at 9600 baud to use keyboard mode.
// ============================================================
