#include "gcode-parser.h"

gcodeparser::gcodeparser(Stream &serial) : serialPort(serial), charCount(0) {}

void gcodeparser::setup(int ServoPin, int XDir, int XStep, int YDir, int YStep, int ScalingFactor){
    pen.attach(ServoPin);
    
    _XDir = XDir;
    _XStep = XStep;

    _YDir = YDir;
    _YDir = YDir;
    _ScalingFactor = ScalingFactor;
}

void gcodeparser::process() {
    while (serialPort.available()) {
        char c = serialPort.read();

        if (c == '\n' || c == '\r') { // MAKE A NEW LINE
            Serial.println("ok");
            if (lineBuffer[0] == 'M' && lineBuffer[1] == '3') StartCode = true;
            if (StartCode){
                handleLine(lineBuffer);
            }

            lineBuffer[charCount] = '\0';
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
    char letter = line[0];
    int v = 0;
    switch (letter){
        case 'M':
            // code for M
            handleM(line);
            break;

        case 'G':
            // code for G
            handleG(line);
            break;

        case 'F':
            v = atoi(line + 1);
            Serial.print("FeedRate: ");
            Serial.println(v);
            break;
        case 'S':
            v = atoi(line + 1);
            Serial.print("SpindleSpeed: ");
            Serial.println(v);
            break;
    }
}

void gcodeparser::handleG(const char *line){
    switch (line[1]){
        case '1':
            XY positions = parseXY(line);
            if (positions.x < 0) digitalWrite(_XDir, HIGH);
            else digitalWrite(_XDir, LOW);
            if (positions.y < 0) digitalWrite(_YDir, HIGH);
            else digitalWrite(_XDir, LOW);

            pulsePin(_XStep, 80, 80, 1);

            break;
    }
}

void gcodeparser::pulsePin(int pin, int timeOn, int timeOff, int count){
    for (int i = 0; i < count; i++){
        digitalWrite(pin, HIGH);
        delayMicroseconds(timeOn);
        digitalWrite(pin, LOW);
        delayMicroseconds(timeOff);
    }
}

XY gcodeparser::parseXY(const char *line) {
    XY result = {0.0f, 0.0f};
    const char *ptr = line;

    while (*ptr) {
        if (*ptr == 'X' || *ptr == 'x') {
            ptr++;
            Serial.println("Found X!");
            result.x = atof(ptr);
        } 
        else if (*ptr == 'Y' || *ptr == 'y') {
            ptr++;
            Serial.println("Found Y!");
            result.y = atof(ptr);
        } 
        else {
            ptr++; // skip anything else
        }
    }

    return result;
}

void gcodeparser::handleM(const char *line){
    int m3value = 65;
    int m4value = 0;
    int m5value = 40;

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
            Serial.print("$$$$$Pulsing 1000 times! Pin: ");
            Serial.print(_XStep);
            Serial.print(", and dir: ");
            Serial.println(_XDir);

            digitalWrite(_XDir, LOW);
            pulsePin(_XStep, 80, 100, 1000);
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
