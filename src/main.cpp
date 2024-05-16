#include <Arduino.h>  

#include <Adafruit_Sensor.h>   //// la gestion des capteurs. 

#include <BH1750.h> 

#include <Wire.h> 

#include <RTClib.h> 

#include <DHT.h> 

#include <SD.h> 

#include <SPI.h>  /////la communication série synchrone sur votre carte Arduino 

#define Nb_Capteurs 3  

 

const int buttonRPin = 2;   

const int buttonVPin = 3; 

const int ledRpin = 5; 

const int ledBPin = 6 ; 

const int ledVPin = 9;    // The pin where your LED is connected 

bool buttonPressed = false; 

bool buttonPressed2 = false; 

unsigned long buttonPressStartTime = 0;  //- 

bool inConfigurationMode = false; 

bool inMaintenanceMode = false; 

bool inEconomeMode = false; 

unsigned long sdWriteInterval = 600000; // Delay for SD card write in milliseconds (10 minutes) 

unsigned long lastSdWriteTime = 0; 

bool initialWrite = true ; 

bool initialWriteSD = true ; 

unsigned long lastLOGInterval = 0; 

unsigned long LOG_INTERVAL = 5000;  // Interval between sensor readings in milliseconds (10 minutes) 

unsigned int TIMEOUT = 30000;       // Timeout for non-responsive sensors in milliseconds (30 seconds) 

unsigned int FILE_MAX_SIZE = 4096;  // Maximum size of data file in bytes (4KB) 

// const int maxInputLength = 6; // Adjust this based on your needs 

// char userInput[maxInputLength + 1]; // +1 for null-terminator 

 

uint8_t rtcHour = 0; 

uint8_t rtcMinute = 0; 

uint8_t rtcSecond = 0; 

uint8_t rtcMonth = 1;  // Default month (e.g., January) 

uint8_t rtcDay = 1;    // Default day (e.g., 1st) 

uint16_t rtcYear = 2023;  // Default year (e.g., 2023) 

char rtcDayOfWeek[4] = "SUN"; // Default day of the week (e.g., Sunday) 

 

#define DHTTYPE DHT11   

#define DHTPIN 8 

 

RTC_DS3231 rtc; 

DHT dht(DHTPIN, DHTTYPE); 

BH1750 lightSensor; 

File dataFile; 

 

typedef struct { 

float luminosity; 

float temperature; 

float humidity; 

// float capteur[Indice_max_Tableau]; 

// float moy_instant; 

DateTime rtcTime; 

int erreur; 

 

  void printLum() { 

  String luminosityAsString = String(luminosity); ................................ 

  Serial.print(F("Luminosity: ")); 

  Serial.println(luminosityAsString); 

  }; 

  void printTemp() { 

    Serial.print("Temperature: "); 

    Serial.println(temperature); 

  }; 

  void printHum() { 

    Serial.print("Humidity: "); 

    Serial.println(humidity); 

  }; 

 

}Sensor; 

 

Sensor capteurs[Nb_Capteurs]; 

///////////////// mode standar/////////////////////////: 

 

void standardMode() { 

  // Read luminosity data from the light sensor 

  if(!inConfigurationMode){ 

  capteurs[0].luminosity = lightSensor.readLightLevel(); 

  capteurs[1].temperature = dht.readTemperature(); 

  capteurs[2].humidity = dht.readHumidity(); 

  } 

 

  // Check if the elapsed time exceeds the TIMEOUT 

  Serial.println(F("standard mode"));  ..................................... 

  analogWrite(ledRpin, 255);     

  analogWrite(ledBPin, 255); 

  analogWrite(ledVPin, 0); 

  DateTime now = rtc.now();  

 int dayOfWeek = now.dayOfTheWeek(); 

const char* PROGMEM dayNames[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"}; /// les variable fixe bdeplace falsh memoire  

Serial.println(dayNames[dayOfWeek]); 

  Serial.println(now.timestamp()); 

  capteurs[0].printLum(); 

  capteurs[1].printTemp(); 

  capteurs[2].printHum(); 

} 

........................................................................................................................................................................................................................................................................... 

void handleLogIntervalChange() { 

  Serial.println(F("Enter new log interval: "));//// mamoire flash  

  while (Serial.available() == 0) { 

    // Wait for user input 

  } 

  LOG_INTERVAL = Serial.parseInt(); 

  Serial.print(F("Log interval changed to: ")); 

  Serial.println(LOG_INTERVAL); 

} 

 

void handleFileMaxSizeChange() {  

  Serial.println(F("Enter new file max size: ")); 

  while (Serial.available() == 0) { 

    // Wait for user input 

  } 

  FILE_MAX_SIZE = Serial.parseInt(); 

  Serial.print(F("File max size changed to: ")); 

  Serial.println(FILE_MAX_SIZE); 

} 

 

void handleResetParameters() { 

  // Reset parameters to their default values 

  LOG_INTERVAL = 5000; 

  FILE_MAX_SIZE = 2048; 

  Serial.println(F("Parameters reset to default")); 

} 

 

void handleSetClock() { 

  char buffer[15]; 

  // Step 1: Prompt User 

  Serial.println(F("Set the clock")); 

 

  while (Serial.available() == 0) { 

    // Step 2: Wait for user input 

  } 

 

  // Step 3: Read User Input 

  Serial.readBytesUntil('\n', buffer, sizeof(buffer)); 

 

  // Step 4: Parse Input 

  int parsedValues = sscanf(buffer, "CLOCK %hhu:%hhu:%hhu", &rtcHour, &rtcMinute, &rtcSecond); 

 

  if (parsedValues == 3) { 

    // Step 5: Update RTC 

    // Use the DateTime constructor with the correct argument types 

    DateTime newTime = DateTime(rtcYear, rtcMonth, rtcDay, rtcHour, rtcMinute, rtcSecond); 

    rtc.adjust(newTime); 

    Serial.print(F("Clock set to: ")); 

    Serial.println(buffer); 

  } else { 

    // Step 6: Provide Feedback 

    Serial.println(F("Invalid format.")); 

  } 

} 

 

void handleSetDate() { 

  char buffer[16]; 

  while (Serial.available() == 0) { 

    // Wait for user input 

  } 

  Serial.readBytesUntil('\n', buffer, sizeof(buffer)); 

  if (sscanf(buffer, "DATE %hhu,%hhu,%u", &rtcMonth, &rtcDay, &rtcYear) == 3) { 

    rtc.adjust(DateTime(rtcYear, rtcMonth, rtcDay, rtcHour, rtcMinute, rtcSecond)); 

    Serial.print(F("Date set to: ")); 

    Serial.println(buffer); 

  } else { 

    Serial.println(F("Invalid format")); 

  } 

} 

 

 

void handleSetDay() { 

  char buffer[4]; 

  while (Serial.available() == 0) { 

    // Wait for user input 

  } 

  Serial.readBytesUntil('\n', buffer, sizeof(buffer)); 

  buffer[3] = '\0'; 

  if (strcmp(buffer, "MON") == 0 || strcmp(buffer, "TUE") == 0 || strcmp(buffer, "WED") == 0 || 

      strcmp(buffer, "THU") == 0 || strcmp(buffer, "FRI") == 0 || strcmp(buffer, "SAT") == 0 || strcmp(buffer, "SUN") == 0) { 

    strncpy(rtcDayOfWeek, buffer, 3); 

    Serial.print(F("Day set to: ")); 

    Serial.println(rtcDayOfWeek); 

  } else { 

    Serial.println(F("Invalid format")); 

  } 

} 

///////////////////mode configuration//////////////////////////////////////////////// 

void ConfigurationMode() { 

  inConfigurationMode = true; 

  Serial.println(F("In Configuration Mode")); 

  Serial.println(F("'q' to exit Configuration Mode")); 

  Serial.println(F("'c' to change the log interval")); 

  Serial.println(F("'r' to reset parameters")); 

  Serial.println(F("'t' to set the clock "));  //(format: CLOCK HH:MM:SS) 

  Serial.println(F("'d' to set the date ")); // (format: DATE MM,DD,YYYY) 

  Serial.println(F("'w' to set the day ")); // (format: DAY MON/TUE/WED/THU/FRI/SAT/SUN) 

 

  analogWrite(ledRpin, 0); 

 

  while (inConfigurationMode) { 

    if (Serial.available() > 0) { 

      char key = Serial.read(); 

      switch (key) { 

        case 'q': 

        case 'Q': 

          inConfigurationMode = false; 

          Serial.println(F("Exiting Configuration Mode")); 

          break; 

        case 'c': 

        case 'C': 

          Serial.println(F("You can change the interval LOG_INTERVAL.")); 

          handleLogIntervalChange(); 

          break; 

        case 'r': 

        case 'R': 

          handleResetParameters(); 

          break; 

        case 't': 

        case 'T': 

          Serial.println(F("Set the clock (format: CLOCK HH:MM:SS):")); 

          handleSetClock(); 

          break; 

        case 'd': 

        case 'D': 

          Serial.println(F("Set the date (format: DATE MM,DD,YYYY):")); 

          handleSetDate(); 

          break; 

        case 'w': 

        case 'W': 

          Serial.println(F("Set the day (format: DAY MON/TUE/WED/THU/FRI/SAT/SUN):")); 

          handleSetDay(); 

          break; 

        default: 

          // Handle other configuration commands as needed 

          break; 

      } 

    } 

  } 

} 

//////////////////////////////////////////////////////////////////////////////////////////////////////////////mode maintenance /////////////////////////  

void MaintenanceMode() { 

  inMaintenanceMode = true; 

  // Add initialization for maintenance mode here 

   Serial.println(F("Maintenance Mode")); 

  //  Serial.println(F("SD card dont work in this mode you can remove it safely")); 

    analogWrite(ledRpin, 0);  

    analogWrite(ledVPin, 165);  

    SD.end(); 

    unsigned long currentTime = millis(); 

     if (initialWrite || (currentTime - lastLOGInterval >= LOG_INTERVAL)) { 

         DateTime now = rtc.now(); 

         Serial.println(now.timestamp()); 

         capteurs[0].printLum(); 

         capteurs[1].printTemp(); 

         capteurs[2].printHum(); 

        lastLOGInterval = currentTime; // Update last write time 

        initialWrite = false; // Set initialWrite to false after the first write 

      } 

} 

.............................................................................................................................................................................................................................................................. 

//////////////mode econom/////////////// 

void EconomMode() { 

  // Set economical mode flag 

  inEconomeMode = true; 

 

  // Store the original LOG_INTERVAL for later restoration 

  unsigned long originalLogInterval = LOG_INTERVAL; 

 

  // Double the LOG_INTERVAL 

  LOG_INTERVAL = 10000; 

 

  Serial.println(F("Economic Mode")); 

  Serial.println(LOG_INTERVAL); 

 

  // Disable the LED indicating data logging 

  analogWrite(ledVPin, 255); 

  analogWrite(ledBPin, 0); 

 

  unsigned long currentTime = millis(); 

 

  // Log data at the new interval 

  if (initialWrite || (currentTime - lastLOGInterval >= LOG_INTERVAL)) { 

    DateTime now = rtc.now(); 

    Serial.println(now.timestamp()); 

    capteurs[0].printLum(); 

    capteurs[1].printTemp(); 

    capteurs[2].printHum(); 

    lastLOGInterval = currentTime; // Update last write time 

    initialWrite = false; // Set initialWrite to false after the first write 

  } 

 

  // Restore the original LOG_INTERVAL when exiting economical mode 

  if (!inEconomeMode) { 

    LOG_INTERVAL = originalLogInterval; 

  } 

} 

 

.................................................//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 

void buttonInterrupt() { 

  // This function will be called when the button state changes (pressed or released) 

  int buttonState = digitalRead(buttonRPin); 

  int buttonState2 = digitalRead(buttonBPin); 

  if (buttonState == LOW) { 

    // Button is pressed 

    buttonPressed = true; 

    buttonPressStartTime = millis(); 

  } else { 

    // Button is released 

    // Reset the flag to allow for another press detection 

    buttonPressed = false; 

  } 

  if (buttonState2 == LOW) { 

    // Button is pressed 

    buttonPressed2 = true; 

    buttonPressStartTime = millis(); 

  } else { 

    // Button is released 

    // Reset the flag to allow for another press detection 

    buttonPressed2 = false; 

  } 

} 

///////////////////////////////////////////////////////////////////////////////////////////////////////////////// 

void setup() { 

Serial.begin(9600); 

Wire.begin(); 

rtc.begin(); 

dht.begin(); 

lightSensor.begin(); 

 

pinMode(ledVPin, OUTPUT);    

pinMode(ledBPin, OUTPUT);    

pinMode(ledRpin,OUTPUT); 

digitalWrite(ledVPin,HIGH); 

digitalWrite(ledRpin,HIGH); 

digitalWrite(ledBPin,HIGH); 

pinMode(buttonRPin, INPUT_PULLUP); // Set the button pin as an input with a pull-up resistor 

attachInterrupt(digitalPinToInterrupt(buttonRPin), buttonInterrupt, CHANGE);/////////////////////////////////// 

pinMode(buttonBPin, INPUT_PULLUP); // Set the button pin as an input with a pull-up resistor 

attachInterrupt(digitalPinToInterrupt(buttonBPin), buttonInterrupt, CHANGE);///////////////////////////////// 

 

 

 

 

  if (SD.begin(10)) { 

    Serial.println(F("SD card is ready to use.")); 

  } else { 

    Serial.println(F("SD card initialization failed.")); 

  } 

  lastSdWriteTime = millis(); 

 

// Set the time for the RTC if necessary 

rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); 

} 

//////////////////////////////////////////////////////////////////////////////////////////////// 

void SD_Card() { 

  DateTime now = rtc.now(); 

  unsigned long currentFileSize = dataFile.size(); 

 

  // Check if the file size exceeds the maximum allowed size 

  if (currentFileSize >= FILE_MAX_SIZE) { 

    // Close the current file 

    dataFile.close(); 

  } 

 

  int year = now.year() % 100; 

  int month = now.month(); 

  int day = now.day(); 

  int revisionNumber = 0; 

  char fileName[13];  // Assuming a maximum of 12 characters for the file name 

 

  // Attempt to open the existing file to extract the date and revision number 

  while (true) { 

    snprintf_P(fileName, 13, "%02d%02d%02d_%d.LOG", year, month, day, revisionNumber); 

    if (!SD.exists(fileName)) { 

      break;  // The file doesn't exist, so the date and revision number are available 

    } 

    // Increment the revision number 

    revisionNumber++; 

  } 

 

  // Create and open a new data file with an incremented revision number 

  snprintf_P(fileName, 13, "%02d%02d%02d_%d.LOG", year, month, day, revisionNumber); 

  dataFile = SD.open(fileName, FILE_WRITE); 

  Serial.println(fileName); 

   

  if (dataFile) { 

    // Serial.println("Writing data to SD card: " + String(fileName)); 

    float luminosity = capteurs[0].luminosity; 

    float temperature = capteurs[1].temperature; 

    float humidity = capteurs[2].humidity; 

 

    // Use pointers to write data to the file 

    dataFile.write((uint8_t*)&luminosity, sizeof(luminosity)); 

    dataFile.write((uint8_t*)&temperature, sizeof(temperature)); 

    dataFile.write((uint8_t*)&humidity, sizeof(humidity)); 

 

    dataFile.flush(); 

    dataFile.close(); 

     

    Serial.println(F("Data written to SD card.")); 

  } else { 

    Serial.println(F("Error opening data file.")); 

  } 

} 

/////////////////////////////////////////////////////////////////////////////////////////////////// 

void loop() { 

 

  if (inEconomeMode) { 

    // econome mode Mode 

    // EconomMode(); 

     if (buttonPressed == true) { 

      unsigned long currentTime = millis(); 

      if (currentTime - buttonPressStartTime >= 5000) { 

        inEconomeMode = false; 

        buttonPressed = false; // Reset buttonPressed flag 

      } 

    } 

  } else if (inMaintenanceMode) { 

    // Maintenance Mode 

       MaintenanceMode(); 

      if (buttonPressed2 == true) { 

      unsigned long currentTime = millis(); 

      if (currentTime - buttonPressStartTime >= 5000) { 

        ConfigurationMode(); 

        buttonPressed2 = false; // Reset buttonPressed flag 

      } 

    } 

 

    if (buttonPressed == true) { 

      unsigned long currentTime = millis(); 

      if (currentTime - buttonPressStartTime >= 5000) { 

        inMaintenanceMode = false; 

        buttonPressed = false; // Reset buttonPressed flag 

      } 

    } 

  } else if(inConfigurationMode) { 

    ConfigurationMode(); 

 

  }  else { 

    unsigned long currentTime = millis(); 

     if (initialWrite || (currentTime - lastLOGInterval >= LOG_INTERVAL)) { 

        standardMode(); 

        analogWrite(ledVPin, 0); 

        lastLOGInterval = currentTime; // Update last write time 

        initialWrite = false; // Set initialWrite to false after the first write 

      } 

 

      unsigned long SDcurrentime = millis(); 

     if (initialWriteSD || (SDcurrentime - lastSdWriteTime >= sdWriteInterval)) { 

        SD_Card(); 

        lastSdWriteTime = SDcurrentime; // Update last write time 

        initialWriteSD = false; // Set initialWrite to false after the first write 

      } 

 

    // Check for entering Configuration Mode 

    if (buttonPressed == true) { 

      unsigned long currentTime = millis(); 

      if (currentTime - buttonPressStartTime >= 5000) { 

        EconomMode(); 

        buttonPressed = false; // Reset buttonPressed flag 

      } 

    } 

 

    // Check for entering Maintenance Mode 

    if (buttonPressed2 == true) { 

      unsigned long currentTime = millis(); 

      if (currentTime - buttonPressStartTime >= 5000) { 

        MaintenanceMode(); 

        buttonPressed2 = false; // Reset buttonPressed flag 

      } 

    } 

  } 

 

   

  delay(1000); 

} 