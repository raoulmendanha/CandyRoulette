/*
  Halloween Candy Roulette
  ---------------------------------------
  Author: Raoul Mendanha
  Date: October 2026

  Description:
  This Halloween-themed Arduino project turns candy selection into a game of chance!
  Trick-or-treaters press a button to spin a motorized "Wheel of Fate" powered by a 
  360-degree continuous rotation servo. The pointer spins chaotically with retro arcade
  sound effects and lands squarely on one of four candies (Snickers, Skittles, Twix, 
  or Life Savers Gummies). Players get 3 spins total tracked by indicator LEDs before 
  the game locks in their haul and triggers a victory fanfare.

  GitHub Repository: https://github.com/raoulmendanha/CandyRoulette
*/

#include <Servo.h>

// --- Pin Definitions ---
const int BUTTON_PIN = 2;
const int SERVO_PIN  = 9;
const int BUZZER_PIN = 8;

// LEDs: Spin 1, 2, and 3 Indicators
const int LED_PIN_1 = 4;
const int LED_PIN_2 = 5;
const int LED_PIN_3 = 6;

// --- Global Objects ---
Servo wheelPointer;

// --- Calibration & Servo Constants ---
// For continuous rotation servos: 90 is stop, 180 is full CW speed
const int SERVO_STOP = 90;
const int SPIN_SPEED = 180;

// CALIBRATION: Milliseconds required to rotate exactly 90 degrees (1 quadrant).
// - Overshooting past the target? Decrease this number (e.g., 140).
// - Undershooting before the target? Increase this number (e.g., 165).
const int MS_PER_90_DEG = 150;

// --- Global Variables ---
int spinCount  = 0;
int currentPos = 0; // 0 = 12 o'clock, 1 = 3 o'clock, 2 = 6 o'clock, 3 = 9 o'clock

// ===================================================================
// SETUP
// ===================================================================
void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN_1, OUTPUT);
  pinMode(LED_PIN_2, OUTPUT);
  pinMode(LED_PIN_3, OUTPUT);

  stopServo();

  randomSeed(analogRead(A0)); // Seed randomness
  startupFanfare();
  resetGame();
}

// ===================================================================
// LOOP
// ===================================================================
void loop() {
  // Check for button press (active LOW with internal pullup)
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(50); // Debounce
    if (digitalRead(BUTTON_PIN) == LOW) {
      executeSpin();

      // Debounce release: wait until button is released
      while (digitalRead(BUTTON_PIN) == LOW) {
        delay(10);
      }
    }
  }
}

// ===================================================================
// RESET FUNCTION
// ===================================================================
void resetGame() {
  spinCount = 0;
  stopServo();
  noTone(BUZZER_PIN);

  digitalWrite(LED_PIN_1, LOW);
  digitalWrite(LED_PIN_2, LOW);
  digitalWrite(LED_PIN_3, LOW);

  delay(400);
}

// ===================================================================
// SPIN SEQUENCE
// ===================================================================
void executeSpin() {
  spinCount++;

  // Light up the corresponding turn LED
  if (spinCount == 1) digitalWrite(LED_PIN_1, HIGH);
  if (spinCount == 2) digitalWrite(LED_PIN_2, HIGH);
  if (spinCount == 3) digitalWrite(LED_PIN_3, HIGH);

  // 1. Pick a random target quadrant (0 to 3)
  int targetPos = random(0, 4);

  // 2. Compute 90-degree steps required
  int forwardSteps = (targetPos - currentPos + 4) % 4;
  if (forwardSteps == 0) {
    forwardSteps = 4; // Ensure at least 1 full spin if landing on same item
  }

  // 3. Add 2 to 4 full revolutions for dramatic arcade suspense
  int extraFullRotations = random(2, 5);
  int totalSteps = forwardSteps + (extraFullRotations * 4);

  // 4. Calculate total run duration based on calibration constant
  unsigned long runTime = (unsigned long)totalSteps * MS_PER_90_DEG;

  // 5. Spin and play clicking wheel sound effects
  wheelPointer.attach(SERVO_PIN);
  wheelPointer.write(SPIN_SPEED);

  unsigned long startTime = millis();
  int tickDelay = 35;

  while (millis() - startTime < runTime) {
    tone(BUZZER_PIN, random(900, 1500), 12);
    delay(tickDelay);
    noTone(BUZZER_PIN);

    // Decelerate clicks near the end
    if (runTime - (millis() - startTime) < 500) {
      tickDelay = 65;
    }
  }

  // 6. Hard stop and update logical position
  stopServo();
  currentPos = targetPos;
  delay(200);

  // Candy reward chime
  awardCandyChime();

  // 7. Check if 3 turns are complete
  if (spinCount >= 3) {
    delay(800);
    grandFinaleFanfare();
    resetGame();
  } else {
    delay(800);
  }
}

// ===================================================================
// AUDIO & HELPER ROUTINES
// ===================================================================
void stopServo() {
  if (wheelPointer.attached()) {
    wheelPointer.write(SERVO_STOP);
  }
  wheelPointer.detach(); // Cuts PWM signal to completely prevent idle creep
}

void awardCandyChime() {
  int notes[] = {523, 659, 784, 1046}; // C5, E5, G5, C6
  for (int i = 0; i < 4; i++) {
    tone(BUZZER_PIN, notes[i], 70);
    delay(85);
  }
  noTone(BUZZER_PIN);
}

void grandFinaleFanfare() {
  // Spooky rising slide
  for (int freq = 220; freq < 1200; freq += 45) {
    tone(BUZZER_PIN, freq, 20);
    delay(15);
  }

  // LED strobe and celebratory beeps
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_PIN_1, HIGH);
    digitalWrite(LED_PIN_2, HIGH);
    digitalWrite(LED_PIN_3, HIGH);
    tone(BUZZER_PIN, 880, 90);
    delay(100);

    digitalWrite(LED_PIN_1, LOW);
    digitalWrite(LED_PIN_2, LOW);
    digitalWrite(LED_PIN_3, LOW);
    delay(70);
  }
  noTone(BUZZER_PIN);
}

void startupFanfare() {
  tone(BUZZER_PIN, 440, 80);
  delay(100);
  tone(BUZZER_PIN, 880, 120);
  delay(150);
  noTone(BUZZER_PIN);
}