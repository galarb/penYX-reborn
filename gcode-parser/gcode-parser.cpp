#include "gcode-parser.h"
// in the motors direction, LOW is left HIGH is right

gcodeparser::gcodeparser(Stream &serial) : serialPort(serial), charCount(0) {}

void gcodeparser::setup(int ServoPin, int XDir, int XStep, int YDir, int YStep,
                        int XLIMIT, int YLIMIT, float ScalingFactor){
    
    pen.attach(ServoPin);
    
    _XDir = XDir;
    _XStep = XStep;

    _YDir = YDir;
    _YStep = YStep;

    _XLIMIT = XLIMIT;
    _YLIMIT = YLIMIT;

    _ScalingFactor = ScalingFactor;

    gcodeparser::initPins();

    gcodeparser::ToHome(); // move to home at start to reset and start count x and y positions in absolute mode!

    Serial.println("ok");
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

        if (c == '\n' || c == '\r') { // NEW LINE
            if (charCount == 0) continue; // skip empty lines (from \r\n)

            lineBuffer[charCount] = '\0'; // terminate the string
            Serial.print("Line: ");
            Serial.println(lineBuffer);

            handleLine(lineBuffer);

            Serial.println("ok"); // only after command executed

            charCount = 0; // reset buffer
            lineCount++;
        } else {
            if (charCount < MAX_LINE_LENGTH - 1) {
                lineBuffer[charCount++] = c;
            }
        }
    }
}

void gcodeparser::handleLine(const char *line){
    char after_line[4] = {0};
    after_line[0] = line[1];
    after_line[1] = line[2];
    after_line[2] = line[3];
    after_line[3] = '\0';

    char letter = line[0];

    if (letter == 'M') gcodeparser::handleM(line);
    if (letter == 'G') gcodeparser::handleG(line);
    if (letter == 'H' || (line[0] == '$' && line[1] == 'H')) gcodeparser::ToHome();
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

            if (positions.x == 0 && positions.y == 0) return;

            xDetectedList[detectedListIndex] = positions.x;
            yDetectedList[detectedListIndex] = positions.y;
            detectedListIndex++;

            gcodeparser::G_MoveSpeed(MOTOR_PULSE_OFF_FAST, positions);
            break;

        case G_MOVE_SLOW:
            positions = gcodeparser::parseXY(line);
            if (positions.x == 0 && positions.y == 0) return;
            xDetectedList[detectedListIndex] = positions.x;
            yDetectedList[detectedListIndex] = positions.y;
            detectedListIndex++;
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
    if (modes.pm == ABSOLUTE) return;

    ChangeDir(positions.x, positions.y);

    Serial.print("Moving in X: ");
    Serial.println(positions.x);
    Serial.print("Moving in Y: ");
    Serial.println(positions.y);
    gcodeparser::MoveXY(abs(positions.x), abs(positions.y), MOTOR_PULSE_ON, speed);
    //if (positions.x != 0) moveMM(_XStep, MOTOR_PULSE_ON, speed, abs(positions.x));
    //if (positions.y != 0) moveMM(_YStep, MOTOR_PULSE_ON, speed, abs(positions.y));
}

void gcodeparser::moveDig(float Xmm, float Ymm, float MIN_INTERVAL){
    if (Xmm == 0) Xmm = 0.1;
    if (Ymm == 0) Ymm = 0.1;

    float dist = sqrt(Xmm * Xmm + Ymm * Ymm);
    if (dist == 0) return;

    float norm_x = Xmm / dist;
    float norm_y = Ymm / dist;

    int x_interval = max(1.0f, MIN_INTERVAL / abs(norm_x));
    int y_interval = max(1.0f, MIN_INTERVAL / abs(norm_y));

    unsigned long X_StepCount = 0;
    unsigned long Y_StepCount = 0;
    bool XState = LOW;
    bool YState = LOW;

    unsigned long XMicro = micros();
    unsigned long YMicro = micros();

    Serial.println(Xmm * _ScalingFactor);
    Serial.println(Ymm * _ScalingFactor);
    Serial.println(x_interval);
    Serial.println(y_interval);

    while (X_StepCount < Xmm * _ScalingFactor && Y_StepCount < Ymm * _ScalingFactor){
        unsigned long now = micros();

        XY limits = gcodeparser::Limits();

        if (limits.x != 0 || limits.y != 0){
            gcodeparser::ToHome();
            return;
        }

        if (X_StepCount < Xmm * _ScalingFactor && now - XMicro >= x_interval){
            XState = !XState;
            digitalWrite(abs(_XStep), XState);
            XMicro = now;
            X_StepCount++;
        }

        if (Y_StepCount < Ymm * _ScalingFactor && now - YMicro >= y_interval){
            YState = !YState;
            digitalWrite(abs(_YStep), YState);
            YMicro = now;
            Y_StepCount++;
        }
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

void gcodeparser::pulsePin(int pin, unsigned long timeOn, unsigned long timeOff, unsigned long count, bool Limits) {
    for (unsigned long i = 0; i < count; ++i) {
        digitalWrite(pin, HIGH);
        delayMicroseconds(timeOn);
        digitalWrite(pin, LOW);
        delayMicroseconds(timeOff);

        if (Limits) {
            XY l = gcodeparser::Limits();
            if (l.x != 0 || l.y != 0){
                Serial.println("HIT LIMIT SWITCH!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
                return;
            }
        }
    }
}

void gcodeparser::ToHome(){
    Serial.println("M3");
    pen.write(M3_MOVE_VALUE);

    digitalWrite(_XDir, HIGH);
    _XDirState = HIGH;
    digitalWrite(_YDir, LOW);
    _YDirState = LOW;
    for (int i = 0; i < 100000; i++){
        digitalWrite(_XStep, HIGH);
        delayMicroseconds(MOTOR_PULSE_ON);
        digitalWrite(_XStep, LOW);
        delayMicroseconds(MOTOR_PULSE_OFF_FAST);

        XY l = gcodeparser::Limits();
        if (l.x < 0) break;
    }

    Serial.println("reached right X pos, switching to Y!");

    for (int i = 0; i < 100000; i++){
        digitalWrite(_YStep, HIGH);
        delayMicroseconds(MOTOR_PULSE_ON);
        digitalWrite(_YStep, LOW);
        delayMicroseconds(MOTOR_PULSE_OFF_FAST);

        XY l = gcodeparser::Limits();
        if (l.y < 0) break;
    }
    digitalWrite(_XDir, LOW);
    _XDirState = LOW;
    digitalWrite(_YDir, HIGH);
    _YDirState = HIGH;

    gcodeparser::MoveXY(10, 0, MOTOR_PULSE_ON, MOTOR_PULSE_OFF_FAST);
    gcodeparser::MoveXY(0, 10, MOTOR_PULSE_ON, MOTOR_PULSE_OFF_FAST);
    Serial.println("reached home point!");
}

void gcodeparser::escapeLimits(XY limits){
    if (limits.x > 0){
        digitalWrite(_XDir, LOW);
        _XDirState = LOW;
        
        gcodeparser::MoveXY(10, 0, MOTOR_PULSE_ON, MOTOR_PULSE_OFF_FAST);
    }else if(limits.y < 0){
        digitalWrite(_XDir, HIGH);
        _XDirState = HIGH;
        gcodeparser::MoveXY(10, 0, MOTOR_PULSE_ON, MOTOR_PULSE_OFF_FAST);
    }

    if (limits.x > 0){
        digitalWrite(_YDir, LOW);
        _YDirState = LOW;
        gcodeparser::MoveXY(0, 10, MOTOR_PULSE_ON, MOTOR_PULSE_OFF_FAST);
    }else if(limits.x < 0){
        digitalWrite(_YDir, HIGH);
        _YDirState = HIGH;
        gcodeparser::MoveXY(0, 10, MOTOR_PULSE_ON, MOTOR_PULSE_OFF_FAST);
    }
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

void gcodeparser::moveMM(int pin, unsigned long timeOn, unsigned long timeOff, float mm, bool Limits)
{
    pulsePin(pin, timeOn, timeOff, (unsigned long)mm * _ScalingFactor, Limits);
}

void gcodeparser::MoveXY(int x_mm, int y_mm, unsigned long time_on, unsigned long time_off) {
  int STEPX = _XStep;
  int STEPY = _YStep;
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

XY gcodeparser::parseXY(const char *line) {
    XY result = {0.0f, 0.0f};
    const char *ptr = line;

    while (*ptr) {
        if (*ptr == 'X' || *ptr == 'x') {
            ptr++;
            result.x = atof(ptr);
            result.x = round(result.x * 1000.0f) / 1000.0f;
        } 
        else if (*ptr == 'Y' || *ptr == 'y') {
            ptr++;
            result.y = atof(ptr);
            result.y = round(result.y * 1000.0f) / 1000.0f;
        } 
        else {
            ptr++;
        }
    }

    Serial.print("X: ");
    Serial.println(result.x, 3);
    Serial.print("Y: ");
    Serial.println(result.y, 3);

    return result;
}

void gcodeparser::handleM(const char *line){
    int m3value = M3_MOVE_VALUE;
    int m4value = M4_MOVE_VALUE;
    int m5value = M5_MOVE_VALUE;
    Serial.println("M running!");
    switch (line[1]){ // the number after M example: M3, M4, M5
        case '3':
            Serial.println("M3");
            pen.write(m3value);
            break;
        case '4':
            pen.write(m4value);
            break;
        case '5':
            pen.write(m5value);
            break;
        case '6':
            // Print all detected positions
            for (int i = 0; i < detectedListIndex; i++){
                Serial.print("Detected: x: ");
                Serial.print(xDetectedList[i]);
                Serial.print(", y: ");
                Serial.println(yDetectedList[i]);
            }
            Serial.println("-----------------------------");
            
            // Clear lists
            for (int i = 0; i < detectedListIndex; i++){
                xDetectedList[i] = 0;
                yDetectedList[i] = 0;
            }

            detectedListIndex = 0;

            //gcodeparser::MoveXY(20, 60, MOTOR_PULSE_ON, MOTOR_PULSE_OFF_SLOW);
            break;

    }
}
