#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include "OneButton.h"
#include <SPI.h>
#include <SD.h>


#define MOTOR_F_PIN 25
#define MOTOR_B_PIN 26

// TFT Display
#define TFT_CS     15   
#define TFT_DC     2   

// SD Card Module
#define SD_CS      5   


Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);


float motor_off_minute =.5;
float motor_running_second =10;


// Motor timings
int motor_forward = 1;
bool motor_func_called = true;
bool motor_status = true;


unsigned long motor_start_time = 0;
unsigned long motor_ramain_time_millis =0;
unsigned long current_time;

void readThresholdsFromSD() {
  File dataFile = SD.open("/value.txt");

  if (dataFile) {
    motor_forward = dataFile.parseInt();
    motor_start_time = dataFile.parseFloat();
    dataFile.close();
    Serial.println("Thresholds read from SD card.");
  } else {
    Serial.println("Error reading value.txt");
  }
}

void writeThresholdsToSD() {

   if (SD.exists("/value.txt")) {
    SD.remove("/value.txt");
    delay(10);
    }
    
    
  File dataFile = SD.open("/value.txt", FILE_WRITE);

  if (dataFile) {
    dataFile.print("motor_forward = "); dataFile.print(motor_forward); dataFile.print("\n");
    dataFile.print("motor_start_time = "); dataFile.print(motor_start_time); dataFile.print("\n");

    dataFile.close();
    Serial.println("Thresholds written to SD card.");
  } else {
    Serial.println("Error writing value.txt");
  }
}



void resetThresholds() {
  motor_forward = 1;
  motor_start_time = millis();
  writeThresholdsToSD();

}


void setup() {
  Serial.begin(115200);
  tft.begin();
  tft.setRotation(3);  
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);
  tft.fillScreen(ILI9341_BLACK);


  // Initialize pins
 
  pinMode(MOTOR_F_PIN, OUTPUT);
  pinMode(MOTOR_B_PIN, OUTPUT);



  // Initialize SD card
  if (!SD.begin(SD_CS)) { // Adjust the pin according to your setup
    Serial.println("SD card initialization failed!");
    error = true;
    // return;
  } else {
    Serial.println("SD card initialization successful");
  }


  resetThresholds();
  delay(500);
  readThresholdsFromSD();
  delay(500);

  motor_start_time = millis();
  
}

void loop() {

  current_time = millis(); 
  switch (currPage) {
    case ROOT_MENU: page_RootMenu(); break;
    default: page_RootMenu(); break;
  }
}


void displayRemainingTime(unsigned long timeInMillis) {
  unsigned long hours = timeInMillis / 3600000;
  unsigned long minutes = (timeInMillis % 3600000) / 60000;
  unsigned long seconds = (timeInMillis % 60000) / 1000;

  // Clear the previous text area
  tft.fillRect(68, 140, 98, 16, ILI9341_RED);
  tft.setCursor(10, 140);
  tft.print("TIME:");
  if (hours < 10) tft.print("0");
  tft.print(hours);
  tft.print(":");
  if (minutes < 10) tft.print("0");
  tft.print(minutes);
  tft.print(":");
  if (seconds < 10) tft.print("0");
  tft.print(seconds);
}

void motor_working() {
  
  unsigned long motor_off = motor_off_minute * 60 * 1000;
  unsigned long motor_running = motor_running_second * 1000;

  if (current_time - motor_start_time <= motor_off) {
    digitalWrite(MOTOR_F_PIN, LOW);
    digitalWrite(MOTOR_B_PIN, LOW);
    // Clear previous motor status text
    tft.fillRect(82, 110, 84, 16, ILI9341_RED);
    tft.setCursor(10, 110);
    tft.print("MOTOR:  OFF ");
    motor_ramain_time_millis = motor_off - (current_time - motor_start_time);
  } else if (current_time - motor_start_time > motor_off && current_time - motor_start_time <= (motor_off + motor_running)) {
    // Clear previous motor status text
     tft.fillRect(82, 110, 84, 16, ILI9341_RED);
    tft.setCursor(10, 110);
    if (motor_forward == 1) {
      digitalWrite(MOTOR_B_PIN, LOW);
      digitalWrite(MOTOR_F_PIN, HIGH);
      tft.print("MOTOR: Run F");
    } else {
      digitalWrite(MOTOR_F_PIN, LOW);
      digitalWrite(MOTOR_B_PIN, HIGH);
      tft.print("MOTOR: Run B");
    }
    motor_ramain_time_millis = (motor_off + motor_running) - (current_time - motor_start_time);
  } else {
    motor_start_time = millis();
    writeThresholdsToSD(); 
    if (motor_forward == 1){motor_forward = 0;}
    else{motor_forward = 1;}
    motor_ramain_time_millis = 0; // Reset remaining time after transition
    // Clear previous motor status text
    tft.fillRect(82, 110, 84, 16, ILI9341_RED);
    tft.setCursor(10, 110);
    tft.print("MOTOR:  OFF ");
  }

  // Call this function with the appropriate remaining time
  displayRemainingTime(motor_ramain_time_millis);
}



void page_RootMenu(void) {
  tft.setTextSize(2);

  motor_working();
  
}
