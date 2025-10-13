#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial grblSerial(3, 255);  // RX, only RX is connected

String buffer = "";

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("penyx monitor");
  lcd.setCursor(0,1);
  lcd.print("started, waiting...");

  grblSerial.begin(115200);
}

void loop() {
  while (grblSerial.available()) {
    char c = grblSerial.read();

    // accumulate characters
    if (c == '\n' || c == '\r') {  // end of line
      // clean buffer
      buffer.trim();

      // ✅ relaxed check: must contain "MPos:" and start with '<'
      if (buffer.startsWith("<") && buffer.indexOf("MPos:") > 0) {
        parseStatus(buffer);
      }

      // reset buffer
      buffer = "";
    } 
    else {
      buffer += c;
      // prevent overflow
      if (buffer.length() > 100) buffer = "";
    }
  }
}


void parseStatus(String msg) {
  // Example input: <Run|MPos:12.345,45.678,-1.000|FS:500,0>
  int mposIndex = msg.indexOf("MPos:");
  if (mposIndex == -1) return;

  int pipeIndex = msg.indexOf("|", mposIndex);
  String coords = msg.substring(mposIndex + 5, pipeIndex);
  coords.trim();

  float x = 0, y = 0, z = 0;
  sscanf(coords.c_str(), "%f,%f,%f", &x, &y, &z);

  // Get state (Run / Idle)
  int start = msg.indexOf("<");
  int bar = msg.indexOf("|");
  String state = msg.substring(start + 1, bar);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("X:");
  lcd.print(x, 2);
  lcd.print(" Y:");
  lcd.print(y, 2);

  lcd.setCursor(0, 1);
  lcd.print("Z:");
  lcd.print(z, 2);
  lcd.print(" ");
  lcd.print(state);
}
