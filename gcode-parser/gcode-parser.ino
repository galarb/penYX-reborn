#include "gcode-parser.h"

gcodeparser parser(Serial);

void setup() {
    Serial.begin(115200);
    parser.setup(11, 5, 2, 6, 3, 20);
    Serial.println("PARSING! STARTED INIT");

    pinMode(2, OUTPUT);
    pinMode(5, OUTPUT);

    /*digitalWrite(5, HIGH);

    for (int i = 0; i < 1000; i++){
        digitalWrite(2, HIGH);
        delayMicroseconds(500);
        digitalWrite(2, LOW);
        delayMicroseconds(500);
        Serial.println("Pulse Sent!");
    }*/
}

void loop() {
    parser.process();
    
}
