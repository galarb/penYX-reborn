#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <string.h> // REQUIRED for strtok and strstr

LiquidCrystal_I2C lcd(0x27, 16, 2);

String inputLine = "";
unsigned long errorDisplayTime = 0; // Keep error tracking

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("penyx monitor");
  lcd.setCursor(0,1);
  lcd.print("started, waiting...");
  
  // Set to standard GRBL speed
  Serial.begin(115200);
  Serial.println("monitor started at 115200");
}

void loop() {
  // Read incoming GRBL serial data line by line
  while (Serial.available()) {
    char c = Serial.read();

    if (c == '\n') {
      processLine(inputLine);
      inputLine = "";
    } else if (c != '\r') {
      inputLine += c;
    }
  }
}


#include <string.h> // Ensure this is included at the top of your sketch

// ... (setup and loop functions remain the same) ...

void processLine(String line) {
  // Debug print to Arduino Serial Monitor
  Serial.print("RAW: ");
  Serial.println(line);

  // 1. Check if it's a status report and contains MPos
  // We only proceed if we see MPos:
  if (line.indexOf("MPos:") == -1) {
    return; // Ignore lines without MPos
  }
  
  // 2. Convert Arduino String to C-style char array for robust parsing
  // Always ensure the array is large enough for the string + null terminator
  char charLine[line.length() + 1];
  line.toCharArray(charLine, sizeof(charLine));
  
  // --- Step 3: Extract MPos Coordinate String ---
  
  // Find "MPos:"
  char* mposPtr = strstr(charLine, "MPos:");
  if (mposPtr == NULL) return; // Failsafe

  // Pointer to the first coordinate (X), 5 chars past "MPos:"
  char* coordStartPtr = mposPtr + 5; 
  
  // Find the end marker specifically: "|FS:" (four characters)
  char* coordEndPtr = strstr(coordStartPtr, "|FS:");

  if (coordEndPtr == NULL) {
    // If |FS: isn't found, try the closing '>' as a fallback
    coordEndPtr = strchr(coordStartPtr, '>');
    if (coordEndPtr == NULL) {
      Serial.println("Parse ERROR: Could not find end marker for MPos data.");
      return;
    }
  }

  // Temporarily null-terminate the coordinate string: "X.XXX,Y.YYY,Z.ZZZ\0"
  *coordEndPtr = '\0'; 
  
  // --- Step 4: Tokenize and Convert ---

  float x = 0, y = 0, z = 0;
  int success_count = 0;
  
  // Use strtok to split the null-terminated coordinate string by comma (",")
  char* token = strtok(coordStartPtr, ",");
  
  // 1. Get X coordinate
  if (token != NULL) {
    x = atof(token);
    success_count++;
    
    // 2. Get Y coordinate
    token = strtok(NULL, ",");
    if (token != NULL) {
      y = atof(token);
      success_count++;
      
      // 3. Get Z coordinate
      token = strtok(NULL, ",");
      if (token != NULL) {
        z = atof(token);
        success_count++;
      }
    }
  }
  
  // --- Step 5: Display Results ---
  
  // (No need to restore the character, as the buffer is temporary)

  if (success_count == 3) {
    // SUCCESS! Display on LCD.
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("X:");
    lcd.print(x, 3);
    lcd.print(" Y:");
    lcd.print(y, 3);

    lcd.setCursor(0, 1);
    lcd.print("Z:");
    lcd.print(z, 3);
    errorDisplayTime = 0;
    
  } else {
    // FAILURE! Display error and log the specific failure.
    Serial.print("Parse ERROR: Failed to extract 3 coordinates. Only matched: ");
    Serial.println(success_count);
    
    lcd.clear();
    lcd.print("Parse error");
    lcd.setCursor(0, 1);
    lcd.print("Matched:");
    lcd.print(success_count);
    errorDisplayTime = millis();
  }
}
