// ========= USER SETTINGS ========= //
int X_DIR_PIN   = 5;
int X_STEP_PIN  = 2;

int Y_DIR_PIN   = 6;
int Y_STEP_PIN  = 3;

int Z_DIR_PIN   = 7;
int Z_STEP_PIN  = 4;
int time_on = 10;
int time_off = 1200;
//////////////////////////////////////

void setup() {
  pinMode(X_DIR_PIN, OUTPUT);
  pinMode(X_STEP_PIN, OUTPUT);

  pinMode(Y_DIR_PIN, OUTPUT);
  pinMode(Y_STEP_PIN, OUTPUT);

  pinMode(Z_DIR_PIN, OUTPUT);
  pinMode(Z_STEP_PIN, OUTPUT);

  Serial.begin(115200);
  Serial.println("Stepper XYZ test ready.");
}

// Move 3 axes at once so they finish together
void moveXYZ(long dx, long dy, long dz) {
  // Set directions
  digitalWrite(X_DIR_PIN, dx > 0);
  digitalWrite(Y_DIR_PIN, dy > 0);
  digitalWrite(Z_DIR_PIN, dz > 0);

  dx = abs(dx);
  dy = abs(dy);
  dz = abs(dz);

  long maxSteps = max(dx, max(dy, dz));

  // These accumulate fractional steps
  long xCount = 0;
  long yCount = 0;
  long zCount = 0;

  for (long i = 0; i < maxSteps; i++) {
    xCount += dx;
    yCount += dy;
    zCount += dz;

    if (xCount >= maxSteps) {
      digitalWrite(X_STEP_PIN, HIGH);
      delayMicroseconds(time_on);
      digitalWrite(X_STEP_PIN, LOW);
      xCount -= maxSteps;
    }

    if (yCount >= maxSteps) {
      digitalWrite(Y_STEP_PIN, HIGH);
      delayMicroseconds(time_on);
      digitalWrite(Y_STEP_PIN, LOW);
      yCount -= maxSteps;
    }

    if (zCount >= maxSteps) {
      digitalWrite(Z_STEP_PIN, HIGH);
      delayMicroseconds(time_on);
      digitalWrite(Z_STEP_PIN, LOW);
      zCount -= maxSteps;
    }

    delayMicroseconds(time_off);
  }
}

void loop() {
  // Example test movement: X100, Y50, Z20 steps
  moveXYZ(1000, 1000, 1000);
  delay(500);

  // Move back
  moveXYZ(-1000, -1000, -1000);
  delay(1000);
}
