
#ifndef GCODEPARSER_H
#define GCODEPARSER_H

#include <Arduino.h>
#include <Servo.h>

// --- constants ---
static constexpr int MAX_LINE_LENGTH = 200;
static constexpr int MAX_WORDS = 10;
static constexpr int MAX_WORD_LENGTH = 50;

// pulse timing (microseconds)
static constexpr unsigned long MOTOR_PULSE_ON = 10UL;
static constexpr unsigned long MOTOR_PULSE_OFF_FAST = 400UL;
static constexpr unsigned long MOTOR_PULSE_OFF_WORK = 1800UL;
static constexpr unsigned long MOTOR_PULSE_OFF_SLOW = 2000UL;

// PEN positions
static constexpr int M3_MOVE_VALUE = 0; // pen up
static constexpr int M4_MOVE_VALUE = 40; // pen down
static constexpr int M5_MOVE_VALUE = 30; //pen travel

// G command codes
static constexpr int G_MOVE_FAST = 1;
static constexpr int G_MOVE_SLOW = 0;
static constexpr int G_MOVE_RELATIVE = 91;
static constexpr int G_MOVE_ABSOLUTE = 90;

struct XY {
    float x;
    float y;
};

enum PositionModes {
    RELATIVE,
    ABSOLUTE
};

struct MODES {
    PositionModes pm;
};

class gcodeparser {
public:
    gcodeparser(Stream &serial);
    void setup(int ServoPin, int XDir, int XStep, int YDir, int YStep,
               int XLIMIT, int YLIMIT, float ScalingFactor); // ScalingFactor is float now
    void process();

    // optional runtime helpers
    void setScalingFactor(float stepsPerMM) { _ScalingFactor = stepsPerMM; }
    float getScalingFactor() const { return _ScalingFactor; }

private:
    Servo pen;

    MODES modes;

    Stream &serialPort;
    char lineBuffer[MAX_LINE_LENGTH];
    int charCount = 0;
    int lineCount = 0;

    float xDetectedList[80];
    float yDetectedList[80];
    int detectedListIndex = 0;

    int _XDir = -1, _XStep = -1;
    int _YDir = -1, _YStep = -1;

    int _XLIMIT = -1, _YLIMIT = -1;

    // steps-per-mm (float to avoid truncation bugs)
    float _ScalingFactor = 84.21f;

    int _XDirState = LOW, _YDirState = LOW;

    XY Limits();

    void initPins();

    void handleLine(const char *line);
    void handleM(const char *line);
    void handleG(const char *line);
    XY parseXY(const char *line);
    void ChangeDir(float x, float y);
    void G_MoveSpeed(int speed, XY positions);
    void MoveForXYPara(const char *line);
    void ToHome();

    void MoveXY(int x_mm, int y_mm, unsigned long time_on, unsigned long time_off);

    // note: count type is unsigned long to allow many pulses
    void pulsePin(int pin, unsigned long timeOn, unsigned long timeOff, unsigned long count, bool Limits = true);

    // mm is float now (not int) so fractional mm are preserved
    void moveMM(int pin, unsigned long timeOn, unsigned long timeOff, float mm, bool Limits = true);

    void moveDig(float Xmm, float Ymm, float MIN_INTERVAL);
    void escapeLimits(XY limits);
};

#endif // if you used include guards
