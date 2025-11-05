#include "gcode-parser.h"

gcodeparser parser(Serial);

#define DIRY 5
#define STEPY 2
#define DIRX 6
#define STEPX 3
#define LIMITX 9
#define LIMITY 10

void setup() {
    Serial.begin(115200);
    parser.setup(11, DIRX, STEPX, DIRY, STEPY, LIMITX, LIMITY, 20);
}

void loop() {
    parser.process();
    
}
