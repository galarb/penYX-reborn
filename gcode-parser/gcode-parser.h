
#ifndef GCODEPARSER_H
#define GCODEPARSER_H

#include <Arduino.h>
#include <Servo.h>

// --- constants ---
static constexpr int MAX_LINE_LENGTH = 200;
static constexpr int MAX_WORDS = 10;
static constexpr int MAX_WORD_LENGTH = 50;

// pulse timing (microseconds)
static constexpr unsigned long MOTOR_PULSE_ON = 11UL;
static constexpr unsigned long MOTOR_PULSE_OFF_FAST = 400UL;
static constexpr unsigned long MOTOR_PULSE_OFF_WORK = 1200UL;
static constexpr unsigned long MOTOR_PULSE_OFF_SLOW = 1500UL;

// PEN positions
static constexpr int M3_MOVE_VALUE = 90;
static constexpr int M4_MOVE_VALUE = 0;
static constexpr int M5_MOVE_VALUE = 40;

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
    bool StartCode = false;
    Servo pen;

    MODES modes;
    float _AX = 0.0f; // absolute X in mm
    float _AY = 0.0f; // absolute Y in mm

    Stream &serialPort;
    char lineBuffer[MAX_LINE_LENGTH];
    int charCount = 0;
    int lineCount = 0;

    int SpindleSpeed = 0;
    int FeedRate = 0;

    int _XDir = -1, _XStep = -1;
    int _YDir = -1, _YStep = -1;

    int _XLIMIT = -1, _YLIMIT = -1;

    // steps-per-mm (float to avoid truncation bugs)
    float _ScalingFactor = 1.0f;

    int _XDirState = LOW, _YDirState = LOW;

    bool canSendOk = true;

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

    // note: count type is unsigned long to allow many pulses
    void pulsePin(int pin, unsigned long timeOn, unsigned long timeOff, unsigned long count, bool Limits = true);

    // mm is float now (not int) so fractional mm are preserved
    void moveMM(int pin, unsigned long timeOn, unsigned long timeOff, float mm, bool Limits = true);

    int fixLine(const char *line, char words[MAX_WORDS][MAX_WORD_LENGTH]);
    void moveDig(float Xmm, float Ymm, float MIN_INTERVAL);
    void escapeLimits(XY limits);

    float _x_accum = 0.0f;
    float _y_accum = 0.0f;
};

#endif // if you used include guards
