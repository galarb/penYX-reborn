#include "gcode-parser.h"

gcodeparser parser(Serial);

#define DIRY 6
#define STEPY 3
#define DIRX 5
#define STEPX 2
#define LIMITX 9
#define LIMITY 10

void setup() {
    Serial.begin(115200);
    digitalWrite(DIRX, LOW);
    parser.setup(11, DIRX, STEPX, DIRY, STEPY, LIMITX, LIMITY, 20);
}

void loop() {
    parser.process();
    //digitalWrite(STEPX, HIGH);
    //delayMicroseconds(10);
    //digitalWrite(STEPX, LOW);
    //delayMicroseconds(1500);
}
/*
#define STEPX 2
#define STEPY 3

// steps per mm (CHANGE THIS)
#define STEPS_PER_MM 84.21

// step pulse timing (microseconds)
#define STEP_PULSE_US 5
#define STEP_DELAY_US 2000   // speed control

int happen = 0;

void setup() {
  pinMode(STEPX, OUTPUT);
  pinMode(STEPY, OUTPUT);
}

void loop() {
  // test move
    if (happen == 1) {return;}
    MoveXY(0, 100);  // 20mm X, 10mm Y
    happen = 1;
}

void MoveXY(int x_mm, int y_mm, unsigned long time_on, unsigned long time_off) {

  long xSteps = (long)x_mm * _ScalingFactor;
  long ySteps = (long)y_mm * _ScalingFactor;

  long dx = abs(xSteps);
  long dy = abs(ySteps);

  long error = 0;

  // Determine major/minor axis
  if (dx >= dy) {
    // X is major axis
    for (long i = 0; i < dx; i++) {

      // Step X
      digitalWrite(STEPX, HIGH);
      delayMicroseconds(time_on);
      digitalWrite(STEPX, LOW);

      error += dy;
      if (error >= dx) {
        // Step Y
        digitalWrite(STEPY, HIGH);
        delayMicroseconds(time_on);
        digitalWrite(STEPY, LOW);

        error -= dx;
      }

      delayMicroseconds(time_off);
    }
  } else {
    // Y is major axis
    for (long i = 0; i < dy; i++) {

      // Step Y
      digitalWrite(STEPY, HIGH);
      delayMicroseconds(time_on);
      digitalWrite(STEPY, LOW);

      error += dx;
      if (error >= dy) {
        // Step X
        digitalWrite(STEPX, HIGH);
        delayMicroseconds(time_on);
        digitalWrite(STEPX, LOW);

        error -= dy;
      }

      delayMicroseconds(time_off);
    }
  }
}
*/
