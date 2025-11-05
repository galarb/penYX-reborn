#include "gcode-parser.h"
// in the motors direction, LOW is left HIGH is right



gcodeparser::gcodeparser(Stream &serial) : serialPort(serial), charCount(0) {}

void gcodeparser::setup(int ServoPin, int XDir, int XStep, int YDir, int YStep,
                        int XLIMIT, int YLIMIT, int ScalingFactor){
    
    modes.pm = RELATIVE; // sets the default mode to be relative

    pen.attach(ServoPin);
    
    pen.write(0);

    _XDir = XDir;
    _XStep = XStep;

    _YDir = YDir;
    _YStep = YStep;

    _XLIMIT = XLIMIT;
    _YLIMIT = YLIMIT;

    _ScalingFactor = ScalingFactor;

    gcodeparser::initPins();

    gcodeparser::ToHome(); // move to home at start to reset and start count x and y positions in absolute mode!

    _AX = 0;
    _AY = 0;
}

void gcodeparser::initPins(){
    pinMode(_XDir, OUTPUT);
    pinMode(_XStep, OUTPUT);
    pinMode(_YDir, OUTPUT);
    pinMode(_YStep, OUTPUT);
    pinMode(_XLIMIT, INPUT_PULLUP);
    pinMode(_YLIMIT, INPUT_PULLUP);

    Serial.println("INIT PINS FINISHED!");
}

void gcodeparser::process() {
    while (serialPort.available()) {
        char c = serialPort.read();

        if (c == '\n' || c == '\r') { // MAKE A NEW LINE
            lineBuffer[charCount] = '\0';
            if (lineBuffer[0] == 'M' && lineBuffer[1] == '3') StartCode = true;
            if (StartCode){
                handleLine(lineBuffer);
            }
            charCount = 0;
            lineCount++;
        } else {
            if (charCount < MAX_LINE_LENGTH - 1) { // ADD THE CHAR TO THE LINE BUFFER
                lineBuffer[charCount++] = c;
            }
        }
    }
} // PROCESS THE GC FILE SENT IN THE SERIAL AND CALLS handleLine FOR EACH LINE!

void gcodeparser::handleLine(const char *line){
    char after_line[4] = {0};
    after_line[0] = line[1];
    after_line[1] = line[2];
    after_line[2] = line[3];
    after_line[3] = '\0';

    char letter = line[0];

    Serial.println(after_line);
    Serial.println("LETTER");
    Serial.println(letter);

    if (letter == 'M') gcodeparser::handleM(line);
    if (letter == 'G') gcodeparser::handleG(line);
    if (strcmp(after_line, "$H") == 0) gcodeparser::ToHome();
    if (strcmp(after_line, "$J=") == 0) gcodeparser::MoveForXYPara(line); // TESTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
}

void gcodeparser::handleG(const char *line) {
    char after_line[3];
    after_line[0] = line[1];
    after_line[1] = line[2];
    after_line[2] = '\0';


    switch (atoi(after_line)) {
        XY positions;

        case G_MOVE_FAST:
            positions = gcodeparser::parseXY(line);
            gcodeparser::G_MoveSpeed(MOTOR_PULSE_OFF_FAST, positions);
            break;

        case G_MOVE_SLOW:
            positions = gcodeparser::parseXY(line);
            gcodeparser::G_MoveSpeed(MOTOR_PULSE_OFF_SLOW, positions);
            break;
        
        case G_MOVE_ABSOLUTE:
            modes.pm = ABSOLUTE;
            Serial.println("ABSOLUTE");
            break;
        case G_MOVE_RELATIVE:
            modes.pm = RELATIVE;
            Serial.println("RELATIVE");
            break;
    }
}

void gcodeparser::MoveForXYPara(const char *line){
    const char* px = strchr(line, 'X');
    const char* py = strchr(line, 'Y');

    if (!px || !py) return;

    XY positions;

    positions.x = atof(px + 1);
    positions.y = atof(py + 1);

    gcodeparser::G_MoveSpeed(MOTOR_PULSE_OFF_FAST, positions);
}

void gcodeparser::G_MoveSpeed(int speed, XY positions){
    if (modes.pm == RELATIVE) {
        ChangeDir(positions.x, positions.y);
        if (positions.x != 0) moveMM(_XStep, MOTOR_PULSE_ON, speed, abs(positions.x));
        if (positions.y != 0) moveMM(_YStep, MOTOR_PULSE_ON, speed, abs(positions.y));
        _AX += positions.x;
        _AY += positions.y;
    }
    else if (modes.pm == ABSOLUTE) {
        float Xmm = positions.x - _AX;
        float Ymm = positions.y - _AY;

        ChangeDir(Xmm, Ymm);
        if (Xmm != 0) moveMM(_XStep, MOTOR_PULSE_ON, speed, abs(Xmm));
        if (Ymm != 0) moveMM(_YStep, MOTOR_PULSE_ON, speed, abs(Ymm));
        _AX = positions.x;
        _AY = positions.y;
    }
}


void gcodeparser::ChangeDir(float x, float y){
    if (x < 0) {
        digitalWrite(_XDir, HIGH);
        _XDirState = HIGH;
    }
    else
    {
        digitalWrite(_XDir, LOW);
        _XDirState = LOW;
    }
    if (y < 0)
    {
        digitalWrite(_YDir, LOW);
        _YDirState = LOW;
    }
    else
    {
        digitalWrite(_YDir, HIGH);
        _YDirState = HIGH;
    }
}

void gcodeparser::pulsePin(int pin, int timeOn, int timeOff, int count, bool Limits = true){
    for (int i = 0; i < count; i++){
        digitalWrite(pin, HIGH);
        delayMicroseconds(timeOn);
        digitalWrite(pin, LOW);
        delayMicroseconds(timeOff);

        if (Limits){
            XY l = gcodeparser::Limits();

            if (l.x != 0 || l.y != 0) return;
        }
    }
}

void gcodeparser::ToHome(){
    digitalWrite(_XDir, HIGH);
    digitalWrite(_YDir, LOW);
    for (int i = 0; i < 100000; i++){
        digitalWrite(_XStep, HIGH);
        delayMicroseconds(MOTOR_PULSE_ON);
        digitalWrite(_XStep, LOW);
        delayMicroseconds(MOTOR_PULSE_OFF_FAST);

        XY l = gcodeparser::Limits();
        if (l.x != 0) break;
    }

    Serial.println("reached right X pos, switching to Y!");

    for (int i = 0; i < 100000; i++){
        digitalWrite(_YStep, HIGH);
        delayMicroseconds(MOTOR_PULSE_ON);
        digitalWrite(_YStep, LOW);
        delayMicroseconds(MOTOR_PULSE_OFF_FAST);

        XY l = gcodeparser::Limits();
        if (l.y != 0) break;
    }
    digitalWrite(_XDir, LOW);
    digitalWrite(_YDir, HIGH);

    gcodeparser::moveMM(_XStep, MOTOR_PULSE_ON, MOTOR_PULSE_OFF_SLOW, 10, false);
    gcodeparser::moveMM(_YStep, MOTOR_PULSE_ON, MOTOR_PULSE_OFF_SLOW, 10, false);
    Serial.println("reached home point!");
}

/*
x < 0 = LIMIT SWITCH END
x > 0 = LIMIT SWITCH START

y < 0 = LIMIT SWITCH END
y > 0 = LIMIT SWITCH START
*/
XY gcodeparser::Limits(){
    XY xy = {0, 0};
    int SwitchX = digitalRead(_XLIMIT);
    int SwitchY = digitalRead(_YLIMIT);

    if (SwitchX == LOW){
        if (_XDirState == HIGH) xy.x = -1;
        if (_XDirState == LOW) xy.x = 1;
    }

    if (SwitchY == LOW){
        if (_YDirState == HIGH) xy.y = 1;
        if (_YDirState == LOW) xy.y = -1;
    }

    return xy;
}

void gcodeparser::moveMM(int pin, int timeOn, int timeOff, int mm, bool Limits = true){
    gcodeparser::pulsePin(pin, timeOn, timeOff, mm * _ScalingFactor, Limits);
}

XY gcodeparser::parseXY(const char *line) {
    XY result = {0.0f, 0.0f};
    const char *ptr = line;

    while (*ptr) {
        if (*ptr == 'X' || *ptr == 'x') {
            ptr++;
            result.x = atof(ptr);
        } 
        else if (*ptr == 'Y' || *ptr == 'y') {
            ptr++;
            result.y = atof(ptr);
        } 
        else {
            ptr++; // skip anything else
        }
    }

    return result;
}

void gcodeparser::handleM(const char *line){
    int m3value = M3_MOVE_VALUE;
    int m4value = M4_MOVE_VALUE;
    int m5value = M5_MOVE_VALUE;

    switch (line[1]){ // the number after M example: M3, M4, M5
        case '3':
            pen.write(m3value);
            break;
        case '4':
            pen.write(m4value);
            break;
        case '5':
            pen.write(m5value);
            break;
        case '6':
            digitalWrite(_YDir, LOW);
            moveMM(_YStep, MOTOR_PULSE_ON, MOTOR_PULSE_OFF_FAST, 10);
            break;
    }
}

int gcodeparser::fixLine(const char *line, char words[MAX_WORDS][MAX_WORD_LENGTH]) {
    int wordIndex = -1;
    int charIndex = 0;
    bool inComment = false;
    bool groupMode = false; // true after G or M, until next G/M or end

    for (int i = 0; line[i] != '\0'; i++) {
        char c = line[i];

        // Stop at comment
        if (c == ';') break;

        // Skip whitespace
        if (c == ' ' || c == '\t') continue;

        if (isalpha(c)) {
            // Start a new word if it's G/M or S/F standalone
            if (c == 'G' || c == 'M') {
                wordIndex++;
                charIndex = 0;
                groupMode = true; // G/M group
            } else if (c == 'S' || c == 'F') {
                wordIndex++;
                charIndex = 0;
                groupMode = false; // standalone
            } else if (!groupMode) {
                // ignore other letters as new word
                wordIndex++;
                charIndex = 0;
            }
        }

        // Append character
        if (wordIndex >= 0 && wordIndex < MAX_WORDS && charIndex < MAX_WORD_LENGTH - 1) {
            words[wordIndex][charIndex++] = c;
            words[wordIndex][charIndex] = '\0';
        }

        // End group mode if next character is G/M/S/F
        if (groupMode && (line[i + 1] == 'G' || line[i + 1] == 'M' || line[i + 1] == 'S' || line[i + 1] == 'F')) {
            groupMode = false;
        }
    }

    return wordIndex + 1;
}

// 1000 pulse at 100 100 is 49mm
