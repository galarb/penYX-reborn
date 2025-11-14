#ifndef GCODEPARSER_H
#define GCODEPARSER_H

#include <Arduino.h>
#include <Servo.h>

#define MAX_LINE_LENGTH 200
#define MAX_WORDS 10
#define MAX_WORD_LENGTH 50

#define MOTOR_PULSE_ON 11
#define MOTOR_PULSE_OFF_FAST 400
#define MOTOR_PULSE_OFF_WORK 1200
#define MOTOR_PULSE_OFF_SLOW 1500

// PEN
#define M3_MOVE_VALUE 75
#define M4_MOVE_VALUE 0
#define M5_MOVE_VALUE 40

// G Commands!
#define G_MOVE_FAST 1
#define G_MOVE_SLOW 0
#define G_MOVE_RELATIVE 91
#define G_MOVE_ABSOLUTE 90


struct XY{
    float x;
    float y;
};

enum PositionModes{
    RELATIVE,
    ABSOLUTE
};

struct MODES
{
    PositionModes pm;
};

class gcodeparser {
public:

    gcodeparser(Stream &serial);
    void setup(int ServoPin, int XDir,
        int XStep, int YDir, int YStep,
        int XLIMIT, int YLIMIT, int ScalingFactor);
    void process();
private:
    bool StartCode;
    Servo pen;

    MODES modes;
    float _AX; // absolute X in mm
    float _AY; // absolute Y in mm

    Stream &serialPort;
    char lineBuffer[MAX_LINE_LENGTH];
    int charCount;
    int lineCount;

    int SpindleSpeed;
    int FeedRate;

    int _XDir, _XStep;
    int _YDir, _YStep;

    int _XLIMIT, _YLIMIT;

    int _ScalingFactor;

    int _XDirState, _YDirState;

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
    void pulsePin(int pin, int timeOn, int timeOff, int count, bool Limits = true);
    void moveMM(int pin, int timeOn, int timeOff, int mm, bool Limits = true);
    int fixLine(const char *line, char words[MAX_WORDS][MAX_WORD_LENGTH]);
    void moveDig(float Xmm, float Ymm, float MIN_INTERVAL);
    void escapeLimits(XY limits);
};

#endif
