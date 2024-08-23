#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include "OneButton.h"
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// WiFi credentials
const char* ssid = "Hasib Hasan";
const char* password = "90909090";

// AP credentials
const char* ap_ssid = "HS INCUBATOR";
const char* ap_password = "12345678";

// Static IP configuration for Station Mode
IPAddress local_IP(192, 168, 0, 110);
IPAddress gateway(192, 168, 0, 20);
IPAddress subnet(255, 255, 255, 0);

// Static IP configuration for AP mode
IPAddress apIP(192, 168, 4, 70);

// Create a web server object
WebServer server(80);




// for nodemcu
#define HEATER_PIN 16
#define BAC_HEATER_PIN 17
#define HFIRE_PIN 21
#define DFIRE_PIN 22
#define MOTOR_F_PIN 25
#define MOTOR_B_PIN 26
#define FAN_PIN 32
#define ALARM_PIN 33

const int btn1 = 12;
const int btn2 = 13;
const int btn3 = 14;
const int btn4 = 27;


// SD Card Module
#define SD_CS      15   // Keep unchanged
#define SD_MOSI    23  // Keep unchanged
#define SD_MISO    19   // MISO pin for SD card module
#define SD_CLK     18   // Keep unchanged





#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 20, 4); // I2C address, 20 columns, 4 rows



OneButton button1(btn1, true);
OneButton button2(btn2, true);
OneButton button3(btn3, true);
OneButton button4(btn4, true);


float temp;
float hum;

// Define thresholds
float Heater_ON;
float Heater_OFF;
float Bac_Heater_ON;
float Bac_Heater_OFF;
float Humi_ON;
float Humi_OFF;
float HT_Fan_ON;
float HH_Fan_ON;
float HT_Alarm_ON;
float HH_Alarm_ON;
float LT_Alarm_ON;
float LH_Alarm_ON;
float motor_off_minute;
float motor_running_second;
float ventilation_off_minute;
float ventilation_running_second;
int day;

// Motor timings
int motor_forward;
bool motor_func_called = true;
bool motor_status = true;

// ventilation timings
bool ventilation_status = false;

bool alarm_status=true;

unsigned long alarm_off_time = 0;
unsigned long day_count_time = 0;
unsigned long motor_start_time;
unsigned long ventilation_start_time;
unsigned long current_time;

// Button state flags
boolean btn1_clicked = false, btn1_double_clicked = false, btn1_long_clicked = false, btn1_long_pressing = false, btn1_long_clicked_stop = false;
boolean btn2_clicked = false, btn2_double_clicked = false, btn2_long_clicked = false, btn2_long_pressing = false, btn2_long_clicked_stop = false;
boolean btn3_clicked = false, btn3_double_clicked = false, btn3_long_clicked = false, btn3_long_pressing = false, btn3_long_clicked_stop = false;
boolean btn4_clicked = false, btn4_double_clicked = false, btn4_long_clicked = false, btn4_long_pressing = false, btn4_long_clicked_stop = false;

enum pageType { ROOT_MENU, SET_MENU };
enum pageType currPage = ROOT_MENU;

// Index for the current threshold being adjusted
int currentThreshold = 0;
const int NUM_THRESHOLDS = 17;


void readThresholdsFromSD() {
  File dataFile = SD.open("value.txt");

  if (dataFile) {
    Heater_ON = dataFile.parseFloat();
    Heater_OFF = dataFile.parseFloat();
    Bac_Heater_ON = dataFile.parseFloat();
    Bac_Heater_OFF = dataFile.parseFloat();
    Humi_ON = dataFile.parseFloat();
    Humi_OFF = dataFile.parseFloat();
    HT_Fan_ON = dataFile.parseFloat();
    HH_Fan_ON = dataFile.parseFloat();
    HT_Alarm_ON = dataFile.parseFloat();
    HH_Alarm_ON = dataFile.parseFloat();
    LT_Alarm_ON = dataFile.parseFloat();
    LH_Alarm_ON = dataFile.parseFloat();
    motor_off_minute = dataFile.parseFloat();
    motor_running_second = dataFile.parseFloat();
    ventilation_off_minute = dataFile.parseFloat();
    ventilation_running_second = dataFile.parseFloat();
    day = dataFile.parseInt();
    motor_forward = dataFile.parseInt();

    dataFile.close();
    Serial.println("Thresholds read from SD card.");
  } else {
    Serial.println("Error opening value.txt");
  }
}

void writeThresholdsToSD() {

   if (SD.exists("value.txt")) {
    SD.remove("value.txt");}
    
  File dataFile = SD.open("value.txt", FILE_WRITE);

  if (dataFile) {
    dataFile.print("Heater_ON = "); dataFile.print(Heater_ON); dataFile.print("\n");
    dataFile.print("Heater_OFF = "); dataFile.print(Heater_OFF); dataFile.print("\n");
    dataFile.print("Bac_Heater_ON = "); dataFile.print(Bac_Heater_ON); dataFile.print("\n");
    dataFile.print("Bac_Heater_OFF = "); dataFile.print(Bac_Heater_OFF); dataFile.print("\n");
    dataFile.print("Humi_ON = "); dataFile.print(Humi_ON); dataFile.print("\n");
    dataFile.print("Humi_OFF = "); dataFile.print(Humi_OFF); dataFile.print("\n");
    dataFile.print("HT_Fan_ON = "); dataFile.print(HT_Fan_ON); dataFile.print("\n");
    dataFile.print("HH_Fan_ON = "); dataFile.print(HH_Fan_ON); dataFile.print("\n");
    dataFile.print("HT_Alarm_ON = "); dataFile.print(HT_Alarm_ON); dataFile.print("\n");
    dataFile.print("HH_Alarm_ON = "); dataFile.print(HH_Alarm_ON); dataFile.print("\n");
    dataFile.print("LT_Alarm_ON = "); dataFile.print(LT_Alarm_ON); dataFile.print("\n");
    dataFile.print("LH_Alarm_ON = "); dataFile.print(LH_Alarm_ON); dataFile.print("\n");
    dataFile.print("motor_off_minute = "); dataFile.print(motor_off_minute); dataFile.print("\n");
    dataFile.print("motor_running_second = "); dataFile.print(motor_running_second); dataFile.print("\n");
    dataFile.print("ventilation_off_minute = "); dataFile.print(ventilation_off_minute); dataFile.print("\n");
    dataFile.print("ventilation_running_second = "); dataFile.print(ventilation_running_second); dataFile.print("\n");
    dataFile.print("day = "); dataFile.print(day); dataFile.print("\n");
    dataFile.print("motor_forward = "); dataFile.print(motor_forward); dataFile.print("\n");

    dataFile.close();
    Serial.println("Thresholds written to SD card.");
  } else {
    Serial.println("Error opening value.txt");
  }
}



void handleRoot() {
  String html = "<!DOCTYPE HTML><html>\
    <head>\
        <meta name='viewport' content='width=device-width, initial-scale=1'>\
        <style>\
        body { margin: 0; padding: 0; box-sizing: border-box; text-align: center; }\
        main { width: 90%; max-width: 800px; margin: 10px auto; border: 1px solid black; color: black; background-color: #f0f8f0; display: grid; grid-template-rows: auto auto auto; grid-template-columns: 1fr; grid-gap: 10px; }\
        .div1 { grid-column: 1 / -1; }\
        .div2, .div3 { text-align: left; padding-left: 10px; }\
        .div2 { background-color: rgb(212, 207, 218); }\
        .div3 { background-color: rgb(174, 221, 239); }\
        .div4 { grid-column: 1 / -1; }\
        h2, b { display: inline-block; margin: 10px 10px; }\
        span { display: inline; background-color: cyan; border: 1px solid black; padding: auto;}\
        input { width: 80px; }\
        button{background-color: rgb(241, 252, 144); }\
        @media (min-width: 600px) { main { grid-template-columns: 1fr 1fr; } }\
        @media (min-width: 800px) { h2 { margin: 0 20px; } }\
    </style>\
    </head>\
    <body>\
        <main>\
            <div class='div1'>\
                <h1>HS INCUBATOR</h1>\
                <h2 id='temperature'>Temperature: </h2>\
                <h2 id='humidity'>Humidity: </h2>\
                <br><br> <hr>\
                <b>Heater: <span id='heater'>OFF</span></b>\
                <b>B_Heater: <span id='backupheater'>OFF</span></b>\
                <b>Humidifier: <span id='humidifier'>OFF</span></b> <br> \
                <b>Fan: <span id='fan'>OFF</span></b>\
                <b>Alarm: <span id='alarm'>OFF</span></b>\
                <b>Motor: <span id='motor'>OFF</span></b>\
            </div>\
            <div class='div2'>\
                Heater ON: <input id='heaterOn' type='number' value=''> C <button onclick='setThreshold(\"heaterOn\", \"Heater_ON\")'>Set</button> <br><br>\
                Heater OFF: <input id='heaterOff' type='number' value=''> C <button onclick='setThreshold(\"heaterOff\", \"Heater_OFF\")'>Set</button> <br><br>\
                B_Heater ON: <input id='backupheaterOn' type='number' value=''> C <button onclick='setThreshold(\"backupheaterOn\", \"Bac_Heater_ON\")'>Set</button> <br><br>\
                B_Heater OFF: <input id='backupheaterOff' type='number' value=''> C <button onclick='setThreshold(\"backupheaterOff\", \"Bac_Heater_OFF\")'>Set</button> <br><br>\
                HT_Fan_ON: <input id='htFanOn' type='number' value=''> C <button onclick='setThreshold(\"htFanOn\", \"HT_Fan_ON\")'>Set</button> <br><br>\
                HT_Alarm_ON: <input id='htAlarmOn' type='number' value=''> C <button onclick='setThreshold(\"htAlarmOn\", \"HT_Alarm_ON\")'>Set</button> <br><br>\
                Motor OFF: <input id='motorOff' type='number' value=''> min <button onclick='setThreshold(\"motorOff\", \"motor_off_minute\")'>Set</button> <br><br>\
                Motor ON: <input id='motorOn' type='number' value=''> sec <button onclick='setThreshold(\"motorOn\", \"motor_running_second\")'>Set</button> <br><br>\
            </div>\
            <div class='div3'>\
                Humidifier ON: <input id='humidifierOn' type='number' value=''> % <button onclick='setThreshold(\"humidifierOn\", \"Humi_ON\")'>Set</button> <br><br>\
                Humidifier OFF: <input id='humidifierOff' type='number' value=''> % <button onclick='setThreshold(\"humidifierOff\", \"Humi_OFF\")'>Set</button> <br><br>\
                HH_Fan_ON: <input id='hhFanOn' type='number' value=''> C <button onclick='setThreshold(\"hhFanOn\", \"HH_Fan_ON\")'>Set</button> <br><br>\
                HH_Alarm_ON: <input id='hhAlarmOn' type='number' value=''> % <button onclick='setThreshold(\"hhAlarmOn\", \"HH_Alarm_ON\")'>Set</button> <br><br>\
                LH_Alarm_ON: <input id='lhAlarmOn' type='number' value=''> % <button onclick='setThreshold(\"lhAlarmOn\", \"LH_Alarm_ON\")'>Set</button> <br><br>\
                LT_Alarm_ON: <input id='ltAlarmOn' type='number' value=''> C <button onclick='setThreshold(\"ltAlarmOn\", \"LT_Alarm_ON\")'>Set</button> <br><br>\
                Vent OFF: <input id='ventOff' type='number' value=''> min <button onclick='setThreshold(\"ventOff\", \"ventilation_off_minute\")'>Set</button> <br><br>\
                Vent ON: <input id='ventOn' type='number' value=''> sec <button onclick='setThreshold(\"ventOn\", \"ventilation_running_second\")'>Set</button> <br><br>\
            </div>\
            <div class='div4'>\
                <button onclick='resetThresholds()'>Reset</button>\
            </div>\
        </main>\
        <script>\
            function updateData() {\
                var xhr = new XMLHttpRequest();\
                xhr.onreadystatechange = function() {\
                    if (xhr.readyState == 4 && xhr.status == 200) {\
                        var data = JSON.parse(xhr.responseText);\
                        document.getElementById('temperature').innerText = 'Temperature: ' + data.temp + ' C';\
                        document.getElementById('humidity').innerText = 'Humidity: ' + data.hum + ' %';\
                        document.getElementById('heater').innerText = data.heater;\
                        document.getElementById('backupheater').innerText = data.backupheater;\
                        document.getElementById('humidifier').innerText = data.humidifier;\
                        document.getElementById('fan').innerText = data.fan;\
                        document.getElementById('alarm').innerText = data.alarm;\
                        document.getElementById('motor').innerText = data.motor;\
                        document.getElementById('heaterOn').value = data.heaterOn;\
                        document.getElementById('heaterOff').value = data.heaterOff;\
                        document.getElementById('backupheaterOn').value = data.backupheaterOn;\
                        document.getElementById('backupheaterOff').value = data.backupheaterOff;\
                        document.getElementById('htFanOn').value = data.htFanOn;\
                        document.getElementById('htAlarmOn').value = data.htAlarmOn;\
                        document.getElementById('ltAlarmOn').value = data.ltAlarmOn;\
                        document.getElementById('motorOff').value = data.motorOff;\
                        document.getElementById('motorOn').value = data.motorOn;\
                        document.getElementById('humidifierOn').value = data.humidifierOn;\
                        document.getElementById('humidifierOff').value = data.humidifierOff;\
                        document.getElementById('hhFanOn').value = data.hhFanOn;\
                        document.getElementById('hhAlarmOn').value = data.hhAlarmOn;\
                        document.getElementById('lhAlarmOn').value = data.lhAlarmOn;\
                        document.getElementById('ventOff').value = data.ventOff;\
                        document.getElementById('ventOn').value = data.ventOn;\
                    }\
                };\
                xhr.open('GET', '/getData', true);\
                xhr.send();\
            }\
            function resetThresholds() {\
                var xhr = new XMLHttpRequest();\
                xhr.open('GET', '/reset', true);\
                xhr.send();\
            }\
            function setThreshold(id, param) {\
                var value = document.getElementById(id).value;\
                var xhr = new XMLHttpRequest();\
                xhr.open('GET', '/set?param=' + param + '&value=' + encodeURIComponent(value), true);\
                xhr.send();\
            }\
            setInterval(updateData, 5000); // Update data every 5 seconds\
        </script>\
    </body>\
    </html>";

  server.send(200, "text/html", html);
}






void handleGetData() {
  String json = "{\"temp\":" + String(temp) +
                ",\"hum\":" + String(hum) +
                ",\"heater\":\"" + (digitalRead(HEATER_PIN) ? "ON" : "OFF") + "\"" +
                ",\"backupheater\":\"" + (digitalRead(BAC_HEATER_PIN) ? "ON" : "OFF") + "\"" +
                ",\"humidifier\":\"" + (digitalRead(HFIRE_PIN) ? "ON" : "OFF") + "\"" +
                ",\"fan\":\"" + (digitalRead(FAN_PIN) ? "ON" : "OFF") + "\"" +
                ",\"alarm\":\"" + (digitalRead(ALARM_PIN) ? "ON" : "OFF") + "\"" +
                ",\"motor\":\"" + (digitalRead(MOTOR_F_PIN) ? "RunF" : (digitalRead(MOTOR_B_PIN) ? "RunB" : "OFF")) + "\"" +
                ",\"heaterOn\":" + String(Heater_ON) +
                ",\"heaterOff\":" + String(Heater_OFF) +
                ",\"backupheaterOn\":" + String(Bac_Heater_ON) +
                ",\"backupheaterOff\":" + String(Bac_Heater_OFF) +
                ",\"htFanOn\":" + String(HT_Fan_ON) +
                ",\"htAlarmOn\":" + String(HT_Alarm_ON) +
                ",\"ltAlarmOn\":" + String(LT_Alarm_ON) +
                ",\"motorOff\":" + String(motor_off_minute) +
                ",\"motorOn\":" + String(motor_running_second) +
                ",\"humidifierOn\":" + String(Humi_ON) +
                ",\"humidifierOff\":" + String(Humi_OFF) +
                ",\"hhFanOn\":" + String(HH_Fan_ON) +
                ",\"hhAlarmOn\":" + String(HH_Alarm_ON) +
                ",\"lhAlarmOn\":" + String(LH_Alarm_ON) +
                ",\"ventOff\":" + String(ventilation_off_minute) +
                ",\"ventOn\":" + String(ventilation_running_second) +
                "}";
  server.send(200, "application/json", json);
}

void handleReset() {
  resetThresholds();
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("   .....Reset....");
  delay(500); // Show the reset message for a short time
  lcd.clear();
  server.send(200, "text/plain", "Thresholds Reset");
}


void handleSet() {
  if (server.hasArg("param") && server.hasArg("value")) {
    String param = server.arg("param");
    String value = server.arg("value");

    // Update thresholds based on the parameter
    if (param == "Heater_ON") {
      Heater_ON = value.toFloat();
    } else if (param == "Heater_OFF") {
      Heater_OFF = value.toFloat();
    } else if (param == "Bac_Heater_ON") {
      Bac_Heater_ON = value.toFloat();
    } else if (param == "Bac_Heater_OFF"){
      Bac_Heater_OFF = value.toFloat();
    } else if (param == "HT_Fan_ON") {
      HT_Fan_ON = value.toFloat();
    } else if (param == "HT_Alarm_ON") {
      HT_Alarm_ON = value.toFloat();
    } else if (param == "LT_Alarm_ON") {
      LT_Alarm_ON = value.toFloat();
    } else if (param == "motor_off_minute") {
      motor_off_minute = value.toFloat();
    } else if (param == "motor_running_second") {
      motor_running_second = value.toFloat();
    } else if (param == "Humi_ON") {
      Humi_ON = value.toFloat();
    } else if (param == "Humi_OFF") {
      Humi_OFF = value.toFloat();
    } else if (param == "HH_Fan_ON") {
      HH_Fan_ON = value.toFloat();
    } else if (param == "HH_Alarm_ON") {
      HH_Alarm_ON = value.toFloat();
    } else if (param == "LH_Alarm_ON") {
      LH_Alarm_ON = value.toFloat();
    } else if (param == "ventilation_off_minute") {
      ventilation_off_minute = value.toFloat();
    } else if (param == "ventilation_running_second") {
      ventilation_running_second = value.toFloat();
    }

    writeThresholdsToSD(); 

  }
  server.send(200, "text/plain", "Threshold updated");
}






void setup() {
  Serial.begin(115200);
  dht.begin();
  lcd.begin(20, 4);
  lcd.backlight();
  lcd.clear();

  // Station Mode configuration
  // if (!WiFi.config(local_IP, gateway, subnet)) {
  //   Serial.println("\nSTA Failed to configure");
  // } else {
  //   Serial.println("\nStatic IP configuration successful");
  // }

  // Connecting to WiFi (Station Mode)
  WiFi.begin(ssid, password);
  Serial.print("\nConnecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  // Serial.println("\nConnected ");
  // Serial.print("STA IP Address: ");
  // Serial.println(WiFi.localIP());

  // // Configuring the access point (AP Mode)
  // WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  // WiFi.softAP(ap_ssid, ap_password);

  // Serial.println("Access Point created");
  // Serial.print("AP IP Address: ");
  // Serial.println(WiFi.softAPIP());

  // Start the server
  server.on("/", handleRoot);
  server.on("/getData", handleGetData); 
  server.on("/reset", handleReset);
  server.on("/set", handleSet);


  server.begin();
  Serial.println("HTTP server started");

  // pinMode(DHTPIN, INPUT);

  pinMode(HEATER_PIN, OUTPUT);
  pinMode(HFIRE_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(ALARM_PIN, OUTPUT);
  pinMode(MOTOR_F_PIN, OUTPUT);
  pinMode(MOTOR_B_PIN, OUTPUT);

  // no need while using onebutton library
  // pinMode(btn1, INPUT_PULLUP);
  // pinMode(btn2, INPUT_PULLUP);
  // pinMode(btn3, INPUT_PULLUP);
  // pinMode(btn4, INPUT_PULLUP);

  // link the button 1 functions.
  button1.attachClick(click1);
  button1.attachDoubleClick(doubleclick1);
  button1.attachLongPressStart(longclick1);
  button1.attachDuringLongPress(longpressing1);
  button1.attachLongPressStop(longclickstop1);


  // link the button 2 functions.
  button2.attachClick(click2);
  button2.attachDoubleClick(doubleclick2);
  button2.attachLongPressStart(longclick2);
  button2.attachDuringLongPress(longpressing2);
  button2.attachLongPressStop(longclickstop2);


  // link the button 3 functions.
  button3.attachClick(click3);
  button3.attachDoubleClick(doubleclick3);
  button3.attachLongPressStart(longclick3);
  button3.attachDuringLongPress(longpressing3);
  button3.attachLongPressStop(longclickstop3);


  // link the button 4 functions.
  button4.attachClick(click4);
  button4.attachDoubleClick(doubleclick4);
  button4.attachLongPressStart(longclick4);
  button4.attachDuringLongPress(longpressing4);
  button4.attachLongPressStop(longclickstop4);

    if (!SD.begin(SD_CS)) { // Adjust the pin according to your setup
    Serial.println("SD card initialization failed!");
    return;
  }

  // Read thresholds from SD card
  readThresholdsFromSD();

  motor_start_time = millis();
  ventilation_start_time = millis();
}



void loop() {

  //   if (Serial.available() > 0) {
  //   String input = Serial.readStringUntil('\n');
  //   Serial.print("Received input: ");
  //   Serial.println(input); // Print the received input for debugging

  //   // Handle temp input
  //   if (input.startsWith("t:")) {
  //     String tempValue = input.substring(2); // Remove "Temp: "
  //     temp = tempValue.toFloat();
  //     Serial.print("Temperature set to: ");
  //     Serial.println(temp);
  //   }
  //   // Handle hum input
  //   else if (input.startsWith("h:")) {
  //     String humiValue = input.substring(2); // Remove "Humi: "
  //     hum = humiValue.toFloat();
  //     Serial.print("Humidity set to: ");
  //     Serial.println(hum);
  //   }
  // }


  server.handleClient();

  current_time = millis(); 
  switch (currPage) {
    case ROOT_MENU: page_RootMenu(); break;
    case SET_MENU: page_SetMenu(); break;
    default: page_RootMenu(); break;
  }
}



void resetButtonStates() {
  btn1_clicked = btn1_double_clicked = btn1_long_clicked = btn1_long_pressing = btn1_long_clicked_stop = false;
  btn2_clicked = btn2_double_clicked = btn2_long_clicked = btn2_long_pressing = btn2_long_clicked_stop = false;
  btn3_clicked = btn3_double_clicked = btn3_long_clicked = btn3_long_pressing = btn3_long_clicked_stop = false;
  btn4_clicked = btn4_double_clicked = btn4_long_clicked = btn4_long_pressing = btn4_long_clicked_stop = false;
}


void displayRemainingTime(unsigned long timeInMillis) {
  unsigned long hours = timeInMillis / 3600000;
  unsigned long minutes = (timeInMillis % 3600000) / 60000;
  unsigned long seconds = (timeInMillis % 60000) / 1000;

  lcd.setCursor(0, 3);
  lcd.print("T:");
  if (hours < 10) lcd.print("0");
  lcd.print(hours);
  lcd.print(":");
  if (minutes < 10) lcd.print("0");
  lcd.print(minutes);
  lcd.print(":");
  if (seconds < 10) lcd.print("0");
  lcd.print(seconds);
}

void motor_working() {
  
  // current_time = millis(); 

  unsigned long remainingTimeMillis = 0;
  unsigned long motor_off = motor_off_minute * 60 * 1000;
  unsigned long motor_running = motor_running_second * 1000;

  if (current_time - motor_start_time <= motor_off) {
    digitalWrite(MOTOR_F_PIN, LOW);
    digitalWrite(MOTOR_B_PIN, LOW);
    lcd.setCursor(0, 2);
    lcd.print("Motor:OFF ");
    remainingTimeMillis = motor_off - (current_time - motor_start_time);
  } else if (current_time - motor_start_time > motor_off && current_time - motor_start_time <= (motor_off + motor_running)) {
    lcd.setCursor(0, 2);
    if (motor_forward == 1) {
      digitalWrite(MOTOR_F_PIN, HIGH);
      lcd.print("Motor:RunF");
    } else {
      digitalWrite(MOTOR_B_PIN, HIGH);
      lcd.print("Motor:RunB");
    }
    remainingTimeMillis = (motor_off + motor_running) - (current_time - motor_start_time);
  } else {
    motor_start_time = millis();
    // motor_forward = !motor_forward;
    if (motor_forward == 1){motor_forward = 0;}
    else{motor_forward = 1;}
    remainingTimeMillis = 0; // Reset remaining time after transition
    lcd.setCursor(0, 2);
    lcd.print("Motor:OFF ");
  }

  // Call this function with the appropriate remaining time
  displayRemainingTime(remainingTimeMillis);
}






void ventilation_working(){

  // current_time = millis(); 

  unsigned long ventilation_off = ventilation_off_minute * 60 * 1000;
  unsigned long ventilation_running = ventilation_running_second * 1000;

  if (current_time - ventilation_start_time <= ventilation_off) {
    ventilation_status = false;
  } else if (current_time - ventilation_start_time > ventilation_off && current_time - ventilation_start_time <= (ventilation_off + ventilation_running)) {
    ventilation_status = true;
  } else {
    ventilation_start_time = millis();
  }

}


void dayCount() {

  // Check if 24 hours have passed 86400000
  if (millis() - day_count_time >= 10000) {
    day++;
    day_count_time = millis(); // Reset the timer
  }
}


void resetThresholds() {
  Heater_ON = 35;
  Heater_OFF = 40;
  Bac_Heater_ON = 25;
  Bac_Heater_OFF = 40;
  Humi_ON = 60;
  Humi_OFF = 70;
  HT_Fan_ON = 45;
  HH_Fan_ON = 75;
  HT_Alarm_ON = 50;
  HH_Alarm_ON = 80;
  LT_Alarm_ON = 20;
  LH_Alarm_ON = 40;
  motor_off_minute = 1;
  motor_running_second = 8;
  ventilation_off_minute = 1;
  ventilation_running_second = 8;
  day = 1;
  motor_forward = 1;

  writeThresholdsToSD();
  
  motor_start_time = millis();
  ventilation_start_time = millis();
}





void page_RootMenu(void) {
    resetButtonStates();

    button1.tick();
    button2.tick();
    button3.tick();
    button4.tick();


    if (motor_func_called) {
        motor_working();
    }

    ventilation_working();

    dayCount();

  temp = dht.readTemperature();
  hum = dht.readHumidity();

    // Check if any reads failed and exit early (to try again).
  if (isnan(hum) || isnan(temp)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

    lcd.setCursor(0, 0);
    lcd.print("Tem:");
    lcd.print(temp,1);
    lcd.setCursor(9, 0);
    lcd.print("C");

    lcd.setCursor(0, 1);
    lcd.print("Hum:");
    lcd.print(hum,1);
    lcd.setCursor(9, 1);
    lcd.print("%");

    // Temperature control
    if (temp <= Heater_ON) {
        digitalWrite(HEATER_PIN, HIGH);
        lcd.setCursor(12, 0);
        lcd.print("Heat: ON-I");
    } else if (temp >= Heater_OFF) {
        digitalWrite(HEATER_PIN, LOW);
        lcd.setCursor(12, 0);
        lcd.print("Heat:OFF");
    }
    if (temp <= Bac_Heater_ON) {
        digitalWrite(BAC_HEATER_PIN, HIGH);
        lcd.setCursor(12, 0);
        lcd.print("Heat: ON-II");
    } else if (temp >= Bac_Heater_OFF) {
        digitalWrite(BAC_HEATER_PIN, LOW);
        lcd.setCursor(12, 0);
        lcd.print("Heat:OFF");
    }

    // Humidity control
    if (hum <= Humi_ON) {
        digitalWrite(HFIRE_PIN, HIGH);
        lcd.setCursor(12, 1);
        lcd.print("Humi: ON");
    } else if (hum >= Humi_OFF) {
        digitalWrite(HFIRE_PIN, LOW);
        lcd.setCursor(12, 1);
        lcd.print("Humi:OFF");
    }

    // Fan control
    if (temp >= HT_Fan_ON || hum >= HH_Fan_ON || ventilation_status) {
        digitalWrite(FAN_PIN, HIGH);
        lcd.setCursor(13, 2);
        lcd.print("Fan: ON");
    } else {
        digitalWrite(FAN_PIN, LOW);
        lcd.setCursor(13, 2);
        lcd.print("Fan:OFF");
    }

    // Alarm control
    if (alarm_status){
      if (temp >= HT_Alarm_ON || hum >= HH_Alarm_ON || temp <= LT_Alarm_ON || hum <= LH_Alarm_ON) {
          digitalWrite(ALARM_PIN, HIGH);
          lcd.setCursor(13, 3);
          lcd.print("Ala: ON");
      } else {
          digitalWrite(ALARM_PIN, LOW);
          lcd.setCursor(13, 3);
          lcd.print("Ala:OFF");
      }
    } else{
      if (temp >= HT_Alarm_ON || hum >= HH_Alarm_ON || temp <= LT_Alarm_ON || hum <= LH_Alarm_ON) {
          digitalWrite(ALARM_PIN, LOW);
          lcd.setCursor(13, 3);
          lcd.print("Ala: ON");
      } else {
          digitalWrite(ALARM_PIN, LOW);
          lcd.setCursor(13, 3);
          lcd.print("Ala:OFF");
      }
    }

  if (!alarm_status && (millis() - alarm_off_time >= 5000)) {
    alarm_status = true; // Reset alaram_status to true after 5 seconds
  }

  if (btn2_clicked){
      btn2_clicked = false;
      alarm_status = false;
      alarm_off_time = millis();
  }

    // Handle Button 2 long click for resetting thresholds
    if (btn2_long_clicked) {
        btn2_long_clicked = false;
        resetThresholds();
        // Optionally, display a message indicating reset
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("   .....Reset....");
        delay(500); // Show the reset message for a short time
        lcd.clear();
    }
    
    // Move to the selected page
    if (btn1_long_clicked) {
        currentThreshold = 0;
        lcd.clear();
        currPage = SET_MENU;
        return;
    }


    //Long pressing funtionality
    if (btn3_long_pressing) {
        btn3_long_pressing = false;
        motor_func_called = false;
        digitalWrite(MOTOR_F_PIN, HIGH);
        lcd.setCursor(0, 2);
        lcd.print("Motor:RunF");
    }

    if (btn3_long_clicked_stop) {
        btn3_long_clicked_stop = false;
        motor_start_time = millis();
        motor_func_called = true;
        motor_forward = 0;
    }

    if (btn4_long_pressing) {
        btn4_long_pressing = false;
        motor_func_called = false;
        digitalWrite(MOTOR_B_PIN, HIGH);
        lcd.setCursor(0, 2);
        lcd.print("Motor:RunB");
    }

    if (btn4_long_clicked_stop) {
        btn4_long_clicked_stop = false;
        motor_start_time = millis();
        motor_func_called = true;
        motor_forward = 1;
    }

    //click funtionality
  
    if (btn3_clicked){
      btn3_clicked = false;
      motor_func_called = false;
      motor_start_time = millis();

      if (motor_status){
        digitalWrite(MOTOR_F_PIN, HIGH);
        lcd.setCursor(0, 2);
        lcd.print("Motor:RunF");
        motor_status = false;
        motor_forward = 0;
      }else{
        digitalWrite(MOTOR_F_PIN, LOW);
        digitalWrite(MOTOR_B_PIN, LOW);
        lcd.setCursor(0, 2);
        lcd.print("Motor:Off");
        motor_status = true;
        motor_func_called = true;
      }

    }

    if (btn4_clicked){
      btn4_clicked = false;
      motor_func_called = false;
      motor_start_time = millis();
    
      if (motor_status){
        digitalWrite(MOTOR_B_PIN, HIGH);
        lcd.setCursor(0, 2);
        lcd.print("Motor:RunB");
        motor_status = false;
        motor_forward = 1;
      }else{
        digitalWrite(MOTOR_B_PIN, LOW);
        digitalWrite(MOTOR_F_PIN, LOW);
        lcd.setCursor(0, 2);
        lcd.print("Motor:Off");
        motor_status = true;
        motor_func_called = true;
      }

    }
}




void page_SetMenu(void) {
  lcd.clear();
  resetButtonStates();
  boolean updateDisplay = true;
  
  digitalWrite(HEATER_PIN, LOW);
  digitalWrite(BAC_HEATER_PIN, LOW);
  digitalWrite(HFIRE_PIN, LOW);
  digitalWrite(MOTOR_F_PIN, LOW);
  digitalWrite(MOTOR_B_PIN, LOW);
  digitalWrite(FAN_PIN, LOW);
  digitalWrite(ALARM_PIN, LOW);

  lcd.setCursor(3, 0);
  lcd.println(F(">>SET_MENU<<"));  

  // Inner loop for setting menu
  while (true) {
    // Capture start time
    // uint32_t loopStartMs = millis();

    button1.tick();
    button2.tick();
    button3.tick();
    button4.tick();
 
    
    if (updateDisplay){
      updateDisplay = false; 
      lcd.setCursor(0, 2);
      lcd.print("                  "); // Clear the old value
      displayCurrentThreshold(); // Redisplay the updated value

    }

    if (btn3_clicked || btn3_long_pressing ) {
      incrementThreshold();
      btn3_clicked = false;
      btn3_long_pressing = false;
      updateDisplay = true;
    }

    if (btn4_clicked || btn4_long_pressing) {
      decrementThreshold();
      btn4_clicked = false;
      btn4_long_pressing = false;
      updateDisplay = true;
    }

    if (btn1_clicked) {
      currentThreshold = (currentThreshold + 1) % NUM_THRESHOLDS;
      btn1_clicked = false;
      updateDisplay = true;
    }

    if (btn2_clicked) {
      currentThreshold = (currentThreshold - 1 + NUM_THRESHOLDS) % NUM_THRESHOLDS; // Ensure wrapping correctly
      btn2_clicked = false;
      updateDisplay = true;
    }

    if (btn1_long_clicked) {
      btn1_long_clicked = false;
      lcd.clear();
      // motor_start_time = millis();
      // ventilation_start_time = millis();
      lcd.setCursor(0, 1);
      lcd.print(" .....updated....");
      delay(500); // Show the reset message for a short time
      lcd.clear();
      currPage = ROOT_MENU;
      return;
    }

    // Keep a specific pace
    // while (millis() - loopStartMs < 25) { delay(2); }
  }
}

void displayCurrentThreshold() {
  lcd.setCursor(2, 2);
  switch (currentThreshold) {
    case 0: lcd.print("Heater_ON:"); lcd.print(Heater_ON, 1); break;
    case 1: lcd.print("Heater_OFF:"); lcd.print(Heater_OFF, 1); break;
    case 2: lcd.print("B_Heater_ON:"); lcd.print(Bac_Heater_ON, 1); break;
    case 3: lcd.print("B_Heater_OFF:"); lcd.print(Bac_Heater_OFF, 1); break;
    case 4: lcd.print("Humi_ON:"); lcd.print(Humi_ON, 1); break;
    case 5: lcd.print("Humi_OFF:"); lcd.print(Humi_OFF, 1); break;
    case 6: lcd.print("HT_Fan_ON:"); lcd.print(HT_Fan_ON, 1); break;
    case 7: lcd.print("HH_Fan_ON:"); lcd.print(HH_Fan_ON, 1); break;
    case 8: lcd.print("HT_Alarm_ON:"); lcd.print(HT_Alarm_ON, 1); break;
    case 9: lcd.print("HH_Alarm_ON:"); lcd.print(HH_Alarm_ON, 1); break;
    case 10: lcd.print("LT_Alarm_ON:"); lcd.print(LT_Alarm_ON, 1); break;
    case 11: lcd.print("LH_Alarm_ON:"); lcd.print(LH_Alarm_ON, 1); break;
    case 12: lcd.print("Motor Off:"); lcd.print(motor_off_minute); lcd.print(" min"); break;
    case 13: lcd.print("Motor Run:"); lcd.print(motor_running_second); lcd.print(" sec"); break;
    case 14: lcd.print("vent Off:"); lcd.print(ventilation_off_minute); lcd.print(" min"); break;
    case 15: lcd.print("vent Run:"); lcd.print(ventilation_running_second); lcd.print(" sec"); break;
    case 16: lcd.print("Day:"); lcd.print(day); break;
  }
}


void incrementThreshold() {
  switch (currentThreshold) {
    case 0: if (Heater_ON < 99.8) {Heater_ON += 0.1;} break;
    case 1: if (Heater_OFF < 99.8) {Heater_OFF += 0.1;} break;
    case 2: if (Bac_Heater_ON < 99.8) {Bac_Heater_ON += 0.1;} break;
    case 3: if (Bac_Heater_OFF < 99.8) {Bac_Heater_OFF += 0.1;} break;
    case 4: if (Humi_ON < 99.8) {Humi_ON += 0.1;} break;
    case 5: if (Humi_OFF < 99.8) {Humi_OFF += 0.1;} break;
    case 6: if (HT_Fan_ON < 99.8) {HT_Fan_ON += 0.1;} break;
    case 7: if (HH_Fan_ON < 99.8) {HH_Fan_ON += 0.1;} break;
    case 8: if (HT_Alarm_ON < 99.8) {HT_Alarm_ON += 0.1;} break;
    case 9: if (HH_Alarm_ON < 99.8) {HH_Alarm_ON += 0.1;} break;
    case 10: if (LT_Alarm_ON < 99.8) {LT_Alarm_ON += 0.1;} break;
    case 11: if (LH_Alarm_ON < 99.8) {LH_Alarm_ON += 0.1;} break;
    case 12: if (motor_off_minute < 999) { motor_off_minute += 1;} break; 
    case 13: if (motor_running_second < 999) { motor_running_second += 1;} break;
    case 14: if(ventilation_off_minute < 999) { ventilation_off_minute += 1;} break; 
    case 15: if(ventilation_running_second < 999) { ventilation_running_second += 1;} break;
    case 16: if(day < 31) { day += 1;} break;
  }
  writeThresholdsToSD(); // Save to SD card after incrementing
}

void decrementThreshold() {
  switch (currentThreshold) {
    case 0: if (Heater_ON > 0) {Heater_ON -= 0.1;} break;
    case 1: if (Heater_OFF > 0) {Heater_OFF -= 0.1;} break;
    case 2: if (Bac_Heater_ON > 0) {Bac_Heater_ON -= 0.1;} break;
    case 3: if (Bac_Heater_OFF > 0) {Bac_Heater_OFF -= 0.1;} break;
    case 4: if (Humi_ON > 0) {Humi_ON -= 0.1;} break;
    case 5: if (Humi_OFF > 0) {Humi_OFF -= 0.1;} break;
    case 6: if (HT_Fan_ON > 0) {HT_Fan_ON -= 0.1;} break;
    case 7: if (HH_Fan_ON > 0) {HH_Fan_ON -= 0.1;} break;
    case 8: if (HT_Alarm_ON > 0) {HT_Alarm_ON -= 0.1;} break;
    case 9: if (HH_Alarm_ON > 0) {HH_Alarm_ON -= 0.1;} break;
    case 10: if (LT_Alarm_ON > 0) {LT_Alarm_ON -= 0.1;} break;
    case 11: if (LH_Alarm_ON > 0) {LH_Alarm_ON -= 0.1;} break;
    case 12: if (motor_off_minute > 0) { motor_off_minute -= 1;} break; 
    case 13: if (motor_running_second > 0) { motor_running_second -= 1;} break;
    case 14: if(ventilation_off_minute > 0) { ventilation_off_minute -= 1;} break; 
    case 15: if(ventilation_running_second > 0) { ventilation_running_second -= 1;} break;
    case 16: if(day > 1) { day -= 1;} break;

  }
  writeThresholdsToSD(); // Save to SD card after incrementing
}


////////////////////////button ///////////////////////

// Callback functions for buttons
void click1() { btn1_clicked = true; }
void doubleclick1() { btn1_double_clicked = true; }
void longclick1() { btn1_long_clicked = true; }
void longpressing1() { btn1_long_pressing = true; }
void longclickstop1() { btn1_long_clicked_stop = true; }

void click2() { btn2_clicked = true; }
void doubleclick2() { btn2_double_clicked = true; }
void longclick2() { btn2_long_clicked = true; }
void longpressing2() { btn2_long_pressing = true; }
void longclickstop2() { btn2_long_clicked_stop = true; }

void click3() { btn3_clicked = true; }
void doubleclick3() { btn3_double_clicked = true; }
void longclick3() { btn3_long_clicked = true; }
void longpressing3() { btn3_long_pressing = true; }
void longclickstop3() { btn3_long_clicked_stop = true; }

void click4() { btn4_clicked = true; }
void doubleclick4() { btn4_double_clicked = true; }
void longclick4() { btn4_long_clicked = true; }
void longpressing4() { btn4_long_pressing = true; }
void longclickstop4() { btn4_long_clicked_stop = true; }