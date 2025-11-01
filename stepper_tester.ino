#define MOTOR_PULSE_ON 11
#define MOTOR_PULSE_OFF_FAST 400
#define MOTOR_PULSE_OFF_WORK 1200
#define MOTOR_PULSE_OFF_SLOW 1500

#define DIRY 5
#define STEPY 2
#define DIRX 6
#define STEPX 3

class MotorTester {
  public:
    int XdirPin;
    int XstepPin;
    int YdirPin;
    int YstepPin;

    // Constructor
    MotorTester(int _Xdirpin, int _Xsteppin, int _Ydirpin, int _Ysteppin) {
      XdirPin = _Xdirpin;
      XstepPin = _Xsteppin;
      YdirPin = _Ydirpin;
      YstepPin = _Ysteppin;

      pinMode(XdirPin, OUTPUT);
      pinMode(XstepPin, OUTPUT);
      pinMode(YdirPin, OUTPUT);
      pinMode(YstepPin, OUTPUT);
    }

    // Move one motor
    void moveMotor(int stepPin, int times, int speed) {
      for (int i = 0; i < times; i++) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(MOTOR_PULSE_ON);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(speed);
      }
    }

    // Move both motors together
    void moveBoth(int times, int speed) {
      for (int i = 0; i < times; i++) {
        digitalWrite(XstepPin, HIGH);
        digitalWrite(YstepPin, HIGH);
        delayMicroseconds(MOTOR_PULSE_ON);
        digitalWrite(XstepPin, LOW);
        digitalWrite(YstepPin, LOW);
        delayMicroseconds(speed);
      }
    }
};

// Create global object for both X and Y
MotorTester penyxtest(DIRX, STEPX, DIRY, STEPY);

void setup() {
  Serial.begin(115200);
  Serial.println("Finished setup");
}

void loop() {
  // Move both motors forward
  digitalWrite(penyxtest.XdirPin, HIGH);
  digitalWrite(penyxtest.YdirPin, HIGH);
  penyxtest.moveBoth(2000, MOTOR_PULSE_OFF_SLOW);

  delay(1000);

  // Move both motors backward
  digitalWrite(penyxtest.XdirPin, LOW);
  digitalWrite(penyxtest.YdirPin, LOW);
  penyxtest.moveBoth(2000, MOTOR_PULSE_OFF_SLOW);

  delay(1000);

  digitalWrite(penyxtest.XdirPin, HIGH);
  digitalWrite(penyxtest.YdirPin, HIGH);
  penyxtest.moveBoth(2000, MOTOR_PULSE_OFF_WORK);

  delay(1000);

  // Move both motors backward
  digitalWrite(penyxtest.XdirPin, LOW);
  digitalWrite(penyxtest.YdirPin, LOW);
  penyxtest.moveBoth(2000, MOTOR_PULSE_OFF_WORK);

  delay(1000);

  digitalWrite(penyxtest.XdirPin, HIGH);
  digitalWrite(penyxtest.YdirPin, HIGH);
  penyxtest.moveBoth(2000, MOTOR_PULSE_OFF_FAST);

  delay(1000);

  // Move both motors backward
  digitalWrite(penyxtest.XdirPin, LOW);
  digitalWrite(penyxtest.YdirPin, LOW);
  penyxtest.moveBoth(2000, MOTOR_PULSE_OFF_FAST);

  delay(1000);
}
