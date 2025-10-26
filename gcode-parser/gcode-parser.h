#ifndef GCODEPARSER_H
#define GCODEPARSER_H

#include <Arduino.h>
#include <Servo.h>

#define MAX_LINE_LENGTH 200
#define MAX_WORDS 10
#define MAX_WORD_LENGTH 50

struct XY{
    float x;
    float y;
};

class gcodeparser {
public:

    gcodeparser(Stream &serial);
    void setup(int ServoPin, int XDir, int XStep, int YDir, int YStep, int ScalingFactor);
    void process();
private:
    bool StartCode;
    Servo pen;

    Stream &serialPort;
    char lineBuffer[MAX_LINE_LENGTH];
    int charCount;
    int lineCount;

    int SpindleSpeed;
    int FeedRate;

    int _XDir, _XStep;
    int _YDir, _YStep;

    int _ScalingFactor;

    void handleLine(const char *line);
    void handleM(const char *line);
    void handleG(const char *line);
    XY parseXY(const char *line);
    void pulsePin(int pin, int timeOn, int timeOff, int count);
    int fixLine(const char *line, char words[MAX_WORDS][MAX_WORD_LENGTH]);
};

#endif
