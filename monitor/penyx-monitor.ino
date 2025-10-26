#include <LiquidCrystal_I2C.h>
#include <Wire.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

bool intro = true;
char lineBuffer[100]; // 100 is the max characters a message has!
int lineCount = 0;

void setup(){
  Serial.begin(115200);
  Serial.println("Starting monitor intro!");

  lcd.init();
  lcd.backlight();
  lcd.clear();

  lcd.setCursor(0, 0);
  writeTextSlow("XY Pen Monitor", 100); // 100 millis!
  lcd.setCursor(0, 1);
  writeTextSlow("Made By Gal.", 100);
  delay(1);
  intro = false;
}

void writeTextSlow(const char* text, int delayTime) {
  lcd.clear();
  lcd.setCursor(0, 0);
  for (int i = 0; text[i] != '\0'; i++) {
    lcd.print(text[i]);
    delay(delayTime);
  }
}

void loop(){
  if (!intro) getMessageFromSerial();

  delay(50);
}

void getMessageFromSerial(){
  if (Serial.available()){
    char c = Serial.read();
    if (c == '\n' || c == '\r'){
      processLine(lineBuffer);
      lineBuffer[0] = '\0';
      lineCount = 0;
    }else{
      lineBuffer[lineCount++] = c;
      lineBuffer[lineCount] = '\0'; // terminate operator to know when string ends!
    }
  }
}

void processLine(const char* line){
  if (!validLine(line)) return;

  const char* message = line + 2;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(message);
}

bool validLine(const char* line){
  if (line[0] == 'p' || line[0] == 'P') return true;
  return false;
}
