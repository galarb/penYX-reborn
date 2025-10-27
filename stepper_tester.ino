#define MOTOR_PULSE_ON 11
#define MOTOR_PULSE_OFF_FAST 400
#define MOTOR_PULSE_OFF_WORK 1200
#define MOTOR_PULSE_OFF_SLOW 1500

class MotorTester {
  public:
    int dirPin;
    int stepPin;

    // Constructor
    MotorTester(int _dirpin, int _steppin) {
      dirPin = _dirpin;
      stepPin = _steppin;
      pinMode(dirPin, OUTPUT);
      pinMode(stepPin, OUTPUT);
    }

    // Move method
    void move(int times, int speed) {
      for (int i = 0; i < times; i++) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(MOTOR_PULSE_ON);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(speed);
      }
    }
};

// Create global object
MotorTester penyxtest(7, 4);

void setup() {
  Serial.begin(115200);
  Serial.println("Finished setup");
}

void loop() {
  // Move stepper motor with slow pulse timing
  penyxtest.move(2000, MOTOR_PULSE_OFF_SLOW);
  delay(1000);

  // Reverse direction
  digitalWrite(penyxtest.dirPin, !digitalRead(penyxtest.dirPin));
  penyxtest.move(2000, MOTOR_PULSE_OFF_SLOW);
  delay(1000);
}
