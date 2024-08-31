#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <DHT.h>
#include "OneButton.h"
#include <SPI.h>
#include <SD.h>

// AP credentials
const char* ap_ssid = "HS INCUBATOR";
const char* ap_password = "12345678";

// WiFi credentials
const char* ssid = "Hasib Hasan";
const char* password = "90909090";

// Static IP configuration for Station Mode
IPAddress local_IP(192, 168, 0, 110);
IPAddress gateway(192, 168, 0, 20);
IPAddress subnet(255, 255, 255, 0);

// Static IP configuration for AP mode
IPAddress apIP(192, 168, 4, 70);

// Create a web server object
WebServer server(80);





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


// TFT Display
#define TFT_CS     15   
#define TFT_DC     2   
// #define TFT_RST    -1  
// #define TFT_MOSI   23   
// #define TFT_CLK    18  


// SD Card Module
#define SD_CS      5   
// #define SD_MOSI    23  
// #define SD_MISO    19   
// #define SD_CLK     18   



Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);


#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

OneButton button1(btn1, true);
OneButton button2(btn2, true);
OneButton button3(btn3, true);
OneButton button4(btn4, true);




float temp=0;
float hum=0;
float t_cal=0;
float h_cal=0;

// Define thresholds
float Heater_ON;
float Heater_OFF;
float Bac_Heater_ON;
float Bac_Heater_OFF;
float Humi_ON;
float Humi_OFF;
float D_Humi_ON;
float D_Humi_OFF;
float HT_Fan_ON;
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
const int NUM_THRESHOLDS = 20;


void readThresholdsFromSD() {
  File dataFile = SD.open("/value.txt");

  if (dataFile) {
    Heater_ON = dataFile.parseFloat();
    Heater_OFF = dataFile.parseFloat();
    Bac_Heater_ON = dataFile.parseFloat();
    Bac_Heater_OFF = dataFile.parseFloat();
    Humi_ON = dataFile.parseFloat();
    Humi_OFF = dataFile.parseFloat();
    D_Humi_ON = dataFile.parseFloat();
    D_Humi_OFF = dataFile.parseFloat();
    HT_Fan_ON = dataFile.parseFloat();
    HT_Alarm_ON = dataFile.parseFloat();
    HH_Alarm_ON = dataFile.parseFloat();
    LT_Alarm_ON = dataFile.parseFloat();
    LH_Alarm_ON = dataFile.parseFloat();
    motor_off_minute = dataFile.parseFloat();
    motor_running_second = dataFile.parseFloat();
    ventilation_off_minute = dataFile.parseFloat();
    ventilation_running_second = dataFile.parseFloat();
    day = dataFile.parseInt();
    day_count_time = dataFile.parseFloat();
    motor_forward = dataFile.parseInt();
    t_cal = dataFile.parseFloat();
    h_cal = dataFile.parseFloat();

    dataFile.close();
    Serial.println("Thresholds read from SD card.");
  } else {
    Serial.println("Error opening value.txt");
  }
}

void writeThresholdsToSD() {

   if (SD.exists("/value.txt")) {
    SD.remove("/value.txt");}
    
  File dataFile = SD.open("/value.txt", FILE_WRITE);

  if (dataFile) {
    dataFile.print("Heater_ON = "); dataFile.print(Heater_ON); dataFile.print("\n");
    dataFile.print("Heater_OFF = "); dataFile.print(Heater_OFF); dataFile.print("\n");
    dataFile.print("Bac_Heater_ON = "); dataFile.print(Bac_Heater_ON); dataFile.print("\n");
    dataFile.print("Bac_Heater_OFF = "); dataFile.print(Bac_Heater_OFF); dataFile.print("\n");
    dataFile.print("Humi_ON = "); dataFile.print(Humi_ON); dataFile.print("\n");
    dataFile.print("Humi_OFF = "); dataFile.print(Humi_OFF); dataFile.print("\n");
    dataFile.print("D_Humi_ON = "); dataFile.print(D_Humi_ON); dataFile.print("\n");
    dataFile.print("D_Humi_OFF = "); dataFile.print(D_Humi_OFF); dataFile.print("\n");
    dataFile.print("HT_Fan_ON = "); dataFile.print(HT_Fan_ON); dataFile.print("\n");
    dataFile.print("HT_Alarm_ON = "); dataFile.print(HT_Alarm_ON); dataFile.print("\n");
    dataFile.print("HH_Alarm_ON = "); dataFile.print(HH_Alarm_ON); dataFile.print("\n");
    dataFile.print("LT_Alarm_ON = "); dataFile.print(LT_Alarm_ON); dataFile.print("\n");
    dataFile.print("LH_Alarm_ON = "); dataFile.print(LH_Alarm_ON); dataFile.print("\n");
    dataFile.print("motor_off_minute = "); dataFile.print(motor_off_minute); dataFile.print("\n");
    dataFile.print("motor_running_second = "); dataFile.print(motor_running_second); dataFile.print("\n");
    dataFile.print("ventilation_off_minute = "); dataFile.print(ventilation_off_minute); dataFile.print("\n");
    dataFile.print("ventilation_running_second = "); dataFile.print(ventilation_running_second); dataFile.print("\n");
    dataFile.print("day = "); dataFile.print(day); dataFile.print("\n");
    dataFile.print("day_count_time = "); dataFile.print(day_count_time); dataFile.print("\n");
    dataFile.print("motor_forward = "); dataFile.print(motor_forward); dataFile.print("\n");
    dataFile.print("t_cal = "); dataFile.print(t_cal); dataFile.print("\n");
    dataFile.print("h_cal = "); dataFile.print(h_cal); dataFile.print("\n");

    dataFile.close();
    Serial.println("Thresholds written to SD card.");
  } else {
    Serial.println("Error opening value.txt");
  }
}



void resetThresholds() {
  Heater_ON = 35;
  Heater_OFF = 40;
  Bac_Heater_ON = 25;
  Bac_Heater_OFF = 40;
  Humi_ON = 60;
  Humi_OFF = 70;
  D_Humi_ON = 75;
  D_Humi_OFF = 70;
  HT_Fan_ON = 45;
  HT_Alarm_ON = 50;
  HH_Alarm_ON = 80;
  LT_Alarm_ON = 20;
  LH_Alarm_ON = 40;
  motor_off_minute = 1;
  motor_running_second = 8;
  ventilation_off_minute = 1;
  ventilation_running_second = 8;
  day = 0;
  day_count_time = 0;
  motor_forward = 1;
  t_cal = 0;
  h_cal = 0;

  writeThresholdsToSD();
  
  motor_start_time = millis();
  ventilation_start_time = millis();
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
                <b>B_Heater: <span id='backupheater'>OFF</span></b> <br>\
                <b>Humidifier: <span id='humidifier'>OFF</span></b>\
                <b>DeHumidifier: <span id='dehumidifier'>OFF</span></b> <br>\
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
                LT_Alarm_ON: <input id='ltAlarmOn' type='number' value=''> C <button onclick='setThreshold(\"ltAlarmOn\", \"LT_Alarm_ON\")'>Set</button> <br><br>\
                Motor OFF: <input id='motorOff' type='number' value=''> min <button onclick='setThreshold(\"motorOff\", \"motor_off_minute\")'>Set</button> <br><br>\
                Motor ON: <input id='motorOn' type='number' value=''> sec <button onclick='setThreshold(\"motorOn\", \"motor_running_second\")'>Set</button> <br><br>\
            </div>\
            <div class='div3'>\
                Humidifier ON: <input id='humidifierOn' type='number' value=''> % <button onclick='setThreshold(\"humidifierOn\", \"Humi_ON\")'>Set</button> <br><br>\
                Humidifier OFF: <input id='humidifierOff' type='number' value=''> % <button onclick='setThreshold(\"humidifierOff\", \"Humi_OFF\")'>Set</button> <br><br>\
                DeHumidifier ON: <input id='dehumidifierOn' type='number' value=''> % <button onclick='setThreshold(\"dehumidifierOn\", \"D_Humi_ON\")'>Set</button> <br><br>\
                DeHumidifier OFF: <input id='dehumidifierOff' type='number' value=''> % <button onclick='setThreshold(\"dehumidifierOff\", \"D_Humi_OFF\")'>Set</button> <br><br>\
                HH_Alarm_ON: <input id='hhAlarmOn' type='number' value=''> % <button onclick='setThreshold(\"hhAlarmOn\", \"HH_Alarm_ON\")'>Set</button> <br><br>\
                LH_Alarm_ON: <input id='lhAlarmOn' type='number' value=''> % <button onclick='setThreshold(\"lhAlarmOn\", \"LH_Alarm_ON\")'>Set</button> <br><br>\
                Vent OFF: <input id='ventOff' type='number' value=''> min <button onclick='setThreshold(\"ventOff\", \"ventilation_off_minute\")'>Set</button> <br><br>\
                Vent ON: <input id='ventOn' type='number' value=''> sec <button onclick='setThreshold(\"ventOn\", \"ventilation_running_second\")'>Set</button> <br><br>\
                Day: <input id='day' type='number' value=''> <button onclick='setThreshold(\"day\", \"day\")'>Set</button> <br><br>\
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
                        document.getElementById('dehumidifier').innerText = data.dehumidifier;\
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
                        document.getElementById('dehumidifierOn').value = data.dehumidifierOn;\
                        document.getElementById('dehumidifierOff').value = data.dehumidifierOff;\
                        document.getElementById('hhAlarmOn').value = data.hhAlarmOn;\
                        document.getElementById('lhAlarmOn').value = data.lhAlarmOn;\
                        document.getElementById('ventOff').value = data.ventOff;\
                        document.getElementById('ventOn').value = data.ventOn;\
                        document.getElementById('day').value = data.day;\
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
                ",\"dehumidifier\":\"" + (digitalRead(DFIRE_PIN) ? "ON" : "OFF") + "\"" +
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
                ",\"dehumidifierOn\":" + String(D_Humi_ON) +
                ",\"dehumidifierOff\":" + String(D_Humi_OFF) +
                ",\"hhAlarmOn\":" + String(HH_Alarm_ON) +
                ",\"lhAlarmOn\":" + String(LH_Alarm_ON) +
                ",\"ventOff\":" + String(ventilation_off_minute) +
                ",\"ventOn\":" + String(ventilation_running_second) +
                ",\"day\":" + String(day) +
                "}";
  server.send(200, "application/json", json);
}


void handleReset() {
  resetThresholds();
    // Clear the screen and display a reset message
    tft.fillScreen(ILI9341_BLACK);
    tft.setTextSize(2);
    tft.setCursor(70, 80);
    tft.print("....Reset....");
    delay(500); // Show the reset message for a short time
    tft.fillScreen(ILI9341_BLACK);
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
    } else if (param == "D_Humi_ON") {
      D_Humi_ON = value.toFloat();
    } else if (param == "D_Humi_OFF") {
      D_Humi_OFF = value.toFloat();
    } else if (param == "HH_Alarm_ON") {
      HH_Alarm_ON = value.toFloat();
    } else if (param == "LH_Alarm_ON") {
      LH_Alarm_ON = value.toFloat();
    } else if (param == "ventilation_off_minute") {
      ventilation_off_minute = value.toFloat();
    } else if (param == "ventilation_running_second") {
      ventilation_running_second = value.toFloat();
    } else if (param == "day") {
      day = value.toInt();
    }

    writeThresholdsToSD(); 

  }
  server.send(200, "text/plain", "Threshold updated");
}



void setup() {
  Serial.begin(115200);
  dht.begin();
  tft.begin();
  tft.setRotation(3);  
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_WHITE);
  tft.fillScreen(ILI9341_BLACK);

  // Station Mode configuration
  tft.setCursor(0, 0); // Set cursor to the start of the screen
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("\nSTA Failed to configure");
    tft.print("STA Failed to configure");
    tft.setCursor(0, 20); // Move cursor to next line
  } else {
    Serial.println("\nStatic IP configuration successful");
    tft.print("Static IP configuration successful");
    tft.setCursor(0, 20); // Move cursor to next line
  }

  // Connecting to WiFi (Station Mode)
  tft.setCursor(0, 40); // Move cursor to next line
  WiFi.begin(ssid, password);
  Serial.print("\nConnecting to WiFi");
  tft.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    tft.print(".");
  }

  Serial.println("\nConnected ");
  tft.setCursor(0, 60); // Move cursor to next line
  tft.print("Connected ");
  tft.setCursor(0, 80); // Move cursor to next line
  Serial.print("STA IP Address: ");
  tft.print("STA IP Address: ");
  Serial.println(WiFi.localIP());
  tft.print(WiFi.localIP());

  // // Configuring the access point (AP Mode)
  // tft.setCursor(0, 100); // Move cursor to next line
  // WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  // WiFi.softAP(ap_ssid, ap_password);

  // Serial.println("Access Point created");
  // tft.print("Access Point created");
  // tft.setCursor(0, 120); // Move cursor to next line
  // Serial.print("AP IP Address: ");
  // tft.print("AP IP Address: ");
  // Serial.println(WiFi.softAPIP());
  // tft.print(WiFi.softAPIP());


  // Start the server
  tft.setCursor(0, 140); // Move cursor to next line
  server.on("/", handleRoot);
  server.on("/getData", handleGetData); 
  server.on("/reset", handleReset);
  server.on("/set", handleSet);
  server.begin();

  Serial.println("HTTP server started");
  tft.print("HTTP server started");

  // Initialize pins
  pinMode(HEATER_PIN, OUTPUT);
  pinMode(BAC_HEATER_PIN, OUTPUT);
  pinMode(HFIRE_PIN, OUTPUT);
  pinMode(DFIRE_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(ALARM_PIN, OUTPUT);
  pinMode(MOTOR_F_PIN, OUTPUT);
  pinMode(MOTOR_B_PIN, OUTPUT);

  // Link button functions
  button1.attachClick(click1);
  button1.attachDoubleClick(doubleclick1);
  button1.attachLongPressStart(longclick1);
  button1.attachDuringLongPress(longpressing1);
  button1.attachLongPressStop(longclickstop1);

  button2.attachClick(click2);
  button2.attachDoubleClick(doubleclick2);
  button2.attachLongPressStart(longclick2);
  button2.attachDuringLongPress(longpressing2);
  button2.attachLongPressStop(longclickstop2);

  button3.attachClick(click3);
  button3.attachDoubleClick(doubleclick3);
  button3.attachLongPressStart(longclick3);
  button3.attachDuringLongPress(longpressing3);
  button3.attachLongPressStop(longclickstop3);

  button4.attachClick(click4);
  button4.attachDoubleClick(doubleclick4);
  button4.attachLongPressStart(longclick4);
  button4.attachDuringLongPress(longpressing4);
  button4.attachLongPressStop(longclickstop4);

  // Initialize SD card
  tft.setCursor(0, 160); // Move cursor to next line
  if (!SD.begin(SD_CS)) { // Adjust the pin according to your setup
    Serial.println("SD card initialization failed!");
    tft.print("SD card initialization failed!");
    // return;
  } else {
    Serial.println("SD card initialization successful");
    tft.print("SD card initialization successful");
  }

  delay(3000);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(2);

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
    
  unsigned long remainingTimeMillis = 0;
  unsigned long motor_off = motor_off_minute * 60 * 1000;
  unsigned long motor_running = motor_running_second * 1000;

  if (current_time - motor_start_time <= motor_off) {
    digitalWrite(MOTOR_F_PIN, LOW);
    digitalWrite(MOTOR_B_PIN, LOW);
    // Clear previous motor status text
    tft.fillRect(82, 110, 84, 16, ILI9341_RED);
    tft.setCursor(10, 110);
    tft.print("MOTOR:  OFF ");
    remainingTimeMillis = motor_off - (current_time - motor_start_time);
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
    remainingTimeMillis = (motor_off + motor_running) - (current_time - motor_start_time);
  } else {
    motor_start_time = millis();
    // motor_forward = !motor_forward;
    if (motor_forward == 1){motor_forward = 0;}
    else{motor_forward = 1;}
    remainingTimeMillis = 0; // Reset remaining time after transition
    // Clear previous motor status text
    tft.fillRect(82, 110, 84, 16, ILI9341_RED);
    tft.setCursor(10, 110);
    tft.print("MOTOR:  OFF ");
  }

  // Call this function with the appropriate remaining time
  displayRemainingTime(remainingTimeMillis);
}

void ventilation_working() {
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
  if (millis() - day_count_time >= 60000) {
    day++;
    day_count_time = millis(); // Reset the timer
    writeThresholdsToSD(); 
  }
}



void page_RootMenu(void) {
  tft.setTextSize(2);
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

  temp = dht.readTemperature() + t_cal;
  hum = dht.readHumidity() + h_cal;

    // Check if any reads failed and exit early (to try again).
  if (isnan(hum) || isnan(temp)) {
    Serial.println("Failed to read from DHT sensor!");
    temp=0;
    hum=0;
    // return;
  }

  // Print the results to the serial monitor
  // Serial.print("Humidity: ");
  // Serial.print(hum);
  // Serial.print(" %\t");
  // Serial.print("Temperature: ");
  // Serial.print(temp);
  // Serial.println(" *C");

  tft.setCursor(0, 10);
  tft.setTextSize(3);
  tft.print(" ..HS INCUBATOR..");
  tft.fillRect(0, 37, tft.width(), 3 , ILI9341_BLUE);
  tft.setTextSize(2);

  tft.fillRect(70, 50, 96, 16, ILI9341_RED);
  tft.setCursor(10, 50);
  tft.print("TEMP: ");
  tft.print(temp, 1);
  tft.print(" C");

  tft.fillRect(70, 80, 96, 16, ILI9341_RED);
  tft.setCursor(10, 80);
  tft.print("HUMI: ");
  tft.print(hum, 1);
  tft.print(" %");

  tft.fillRect(70, 170, 98, 16, ILI9341_RED);
  tft.setCursor(10, 170);
  tft.print("DAY:    ");
  if (day < 10 ){ tft.print("0");}
  tft.print(day);

  // Temperature control
  if (temp <= Heater_ON) {
    digitalWrite(HEATER_PIN, HIGH);
    tft.fillRect(240, 50, tft.width(), 16, ILI9341_RED);
    tft.setCursor(180, 50);
    tft.print("HEAT:  ON-I");
  } else if (temp >= Heater_OFF) {
    digitalWrite(HEATER_PIN, LOW);
    tft.fillRect(240, 50, tft.width(), 16, ILI9341_RED);
    tft.setCursor(180, 50);
    tft.print("HEAT:  OFF");
  }
  if (temp <= Bac_Heater_ON) {
    digitalWrite(BAC_HEATER_PIN, HIGH);
    tft.fillRect(240, 50, tft.width(), 16, ILI9341_RED);
    tft.setCursor(180, 50);
    tft.print("HEAT: ON-II");
  } else if (temp >= Bac_Heater_OFF) {
    digitalWrite(BAC_HEATER_PIN, LOW);
    tft.fillRect(240, 50, tft.width(), 16, ILI9341_RED);
    tft.setCursor(180, 50);
    tft.print("HEAT:  OFF");
  }

  // Humidity control
  if (hum <= Humi_ON) {
    digitalWrite(HFIRE_PIN, HIGH);
    tft.fillRect(240, 80, tft.width(), 16, ILI9341_RED);
    tft.setCursor(180, 80);
    tft.print("HUMI:  ON");
  } else if (hum >= Humi_OFF) {
    digitalWrite(HFIRE_PIN, LOW);
    tft.fillRect(240, 80, tft.width(), 16, ILI9341_RED);
    tft.setCursor(180, 80);
    tft.print("HUMI:  OFF");
  }

  // DeHumidity control
  if (hum >= D_Humi_ON) {
    digitalWrite(DFIRE_PIN, HIGH);
    tft.fillRect(240, 110, tft.width(), 16, ILI9341_RED);
    tft.setCursor(180, 110);
    tft.print("DHUM:  ON");
  } else if (hum <= Humi_OFF) {
    digitalWrite(DFIRE_PIN, LOW);
    tft.fillRect(240, 110, tft.width(), 16, ILI9341_RED);
    tft.setCursor(180, 110);
    tft.print("DHUM:  OFF");
  }

  // Fan control
  if (temp >= HT_Fan_ON || ventilation_status) {
    digitalWrite(FAN_PIN, HIGH);
    tft.fillRect(240, 140, tft.width(), 16, ILI9341_RED);
    tft.setCursor(180, 140);
    tft.print("FAN:   ON");
  } else {
    digitalWrite(FAN_PIN, LOW);
    tft.fillRect(240, 140, tft.width(), 16, ILI9341_RED);
    tft.setCursor(180, 140);
    tft.print("FAN:   OFF");
  }

  // Alarm control
  if (alarm_status){
    if (temp >= HT_Alarm_ON || hum >= HH_Alarm_ON || temp <= LT_Alarm_ON || hum <= LH_Alarm_ON) {
      digitalWrite(ALARM_PIN, HIGH);
      tft.fillRect(252, 170, tft.width(), 16, ILI9341_RED);
      tft.setCursor(180, 170);
      tft.print("ALARM:  ON");
    } else {
      digitalWrite(ALARM_PIN, LOW);
      tft.fillRect(252, 170, tft.width(), 16, ILI9341_RED);
      tft.setCursor(180, 170);
      tft.print("ALARM: OFF");
    }
  } else{
    if (temp >= HT_Alarm_ON || hum >= HH_Alarm_ON || temp <= LT_Alarm_ON || hum <= LH_Alarm_ON) {
      digitalWrite(ALARM_PIN, LOW);
      tft.fillRect(252, 170, tft.width(), 16, ILI9341_RED);
      tft.setCursor(180, 170);
      tft.print("ALARM:  ON");
    } else {
      digitalWrite(ALARM_PIN, LOW);
      tft.fillRect(252, 170, tft.width(), 16, ILI9341_RED);
      tft.setCursor(180, 170);
      tft.print("ALARM: OFF");
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
    // Clear the screen and display a reset message
    tft.fillScreen(ILI9341_BLACK);
    tft.setCursor(70, 80);
    tft.print("....RESET....");
    delay(500); // Show the reset message for a short time
    tft.fillScreen(ILI9341_BLACK);
  }
  
  // Move to the selected page
  if (btn1_long_clicked) {
    currentThreshold = 0;
    tft.fillScreen(ILI9341_BLACK);
    currPage = SET_MENU;
    return;
  }

  // Long pressing functionality
  if (btn3_long_pressing) {
    btn3_long_pressing = false;
    motor_func_called = false;
    digitalWrite(MOTOR_B_PIN, LOW);
    delay(100);
    digitalWrite(MOTOR_F_PIN, HIGH);
    tft.fillRect(82, 110,  84, 16, ILI9341_RED);
    tft.setCursor(10, 110);
    tft.print("MOTOR: Run F");
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
    digitalWrite(MOTOR_F_PIN, LOW);
    delay(100);
    digitalWrite(MOTOR_B_PIN, HIGH);
    tft.fillRect(82, 110,  84, 16, ILI9341_RED);
    tft.setCursor(10, 110);
    tft.print("MOTOR: Run B");
  }

  if (btn4_long_clicked_stop) {
    btn4_long_clicked_stop = false;
    motor_start_time = millis();
    motor_func_called = true;
    motor_forward = 1;
  }

  // Click functionality
  if (btn3_clicked) {
    btn3_clicked = false;
    motor_func_called = false;
    motor_start_time = millis();

    if (motor_status) {
      digitalWrite(MOTOR_B_PIN, LOW);
      delay(100);
      digitalWrite(MOTOR_F_PIN, HIGH);
      tft.fillRect(82, 110,  84, 16, ILI9341_RED);
      tft.setCursor(10, 110);
      tft.print("MOTOR: Run F");
      motor_status = false;
      motor_forward = 0;
    } else {
      digitalWrite(MOTOR_F_PIN, LOW);
      digitalWrite(MOTOR_B_PIN, LOW);
      tft.fillRect(82, 110,  84, 16, ILI9341_RED);
      tft.setCursor(10, 110);
      tft.print("MOTOR: Off");
      motor_status = true;
      motor_func_called = true;
    }
  }

  if (btn4_clicked) {
    btn4_clicked = false;
    motor_func_called = false;
    motor_start_time = millis();
  
    if (motor_status) {
      digitalWrite(MOTOR_F_PIN, LOW);
      delay(100);
      digitalWrite(MOTOR_B_PIN, HIGH);
      tft.fillRect(82, 110,  84, 16, ILI9341_RED);
      tft.setCursor(10, 110);
      tft.print("MOTOR: Run B");
      motor_status = false;
      motor_forward = 1;
    } else {
      digitalWrite(MOTOR_B_PIN, LOW);
      digitalWrite(MOTOR_F_PIN, LOW);
      tft.fillRect(82, 110,  84, 16, ILI9341_RED);
      tft.setCursor(10, 110);
      tft.print("MOTOR: Off");
      motor_status = true;
      motor_func_called = true;
    }
  }


    tft.fillRect(0, 194,  tft.width(), 1 , ILI9341_RED);
    tft.fillRect(0, 235,  tft.width(), 1 , ILI9341_RED);
    
}



void page_SetMenu(void) {
  resetButtonStates();
  
  digitalWrite(HEATER_PIN, LOW);
  digitalWrite(BAC_HEATER_PIN, LOW);
  digitalWrite(HFIRE_PIN, LOW);
  digitalWrite(MOTOR_F_PIN, LOW);
  digitalWrite(MOTOR_B_PIN, LOW);
  digitalWrite(FAN_PIN, LOW);
  digitalWrite(ALARM_PIN, LOW);

  tft.setTextSize(2);
  tft.setCursor(90, 10);
  tft.print(">>SET_MENU<<");  

    // Update button states
    button1.tick();
    button2.tick();
    button3.tick();
    button4.tick();
 

    displayCurrentThreshold(); 


    if (btn3_clicked || btn3_long_pressing ) {
      incrementThreshold();
      btn3_clicked = false;
      btn3_long_pressing = false;
      tft.fillRect(0, 100, tft.width(), 24, ILI9341_BLACK);
    }

    if (btn4_clicked || btn4_long_pressing) {
      decrementThreshold();
      btn4_clicked = false;
      btn4_long_pressing = false;
      tft.fillRect(0, 100, tft.width(), 24, ILI9341_BLACK);
    }

    if (btn1_clicked) {
      currentThreshold = (currentThreshold + 1) % NUM_THRESHOLDS;
      btn1_clicked = false;
      tft.fillRect(0, 50, tft.width(), 24, ILI9341_BLACK);
      tft.fillRect(0, 100, tft.width(), 24, ILI9341_BLACK);
    }

    if (btn2_clicked) {
      currentThreshold = (currentThreshold - 1 + NUM_THRESHOLDS) % NUM_THRESHOLDS; // Ensure wrapping correctly
      btn2_clicked = false;
      tft.fillRect(0, 50, tft.width(), 24, ILI9341_BLACK);
      tft.fillRect(0, 100, tft.width(), 24, ILI9341_BLACK);
    }

    if (btn1_long_clicked) {
      btn1_long_clicked = false;
      // motor_start_time = millis();
      // ventilation_start_time = millis();
      tft.fillScreen(ILI9341_BLACK);
      tft.setCursor(70, 80);
      tft.print("....UPDATED....");
      delay(500);
      tft.fillScreen(ILI9341_BLACK);
      currPage = ROOT_MENU;
      return;
    }


  }


void displayCurrentThreshold() {
  switch (currentThreshold) {
    case 0: tft.setCursor(10,50); tft.print("1. HEATER ON:"); tft.setCursor(120,100); tft.print(Heater_ON, 1); tft.print(" C"); break;
    case 1: tft.setCursor(10,50); tft.print("2. HEATER OFF:"); tft.setCursor(120,100); tft.print(Heater_OFF, 1); tft.print(" C"); break;
    case 2: tft.setCursor(10,50); tft.print("3. BACKUP HEATER ON:"); tft.setCursor(120,100); tft.print(Bac_Heater_ON, 1); tft.print(" C"); break;
    case 3: tft.setCursor(10,50); tft.print("4. BACKUP HEATER OFF:"); tft.setCursor(120,100); tft.print(Bac_Heater_OFF, 1); tft.print(" C"); break;
    case 4: tft.setCursor(10,50); tft.print("5. HUMIDIFIER ON:"); tft.setCursor(120,100); tft.print(Humi_ON, 1); tft.print(" %"); break;
    case 5: tft.setCursor(10,50); tft.print("6. HUMIDIFIER OFF:"); tft.setCursor(120,100); tft.print(Humi_OFF, 1); tft.print(" %"); break;
    case 6: tft.setCursor(10,50); tft.print("7. DEHUMIDIFIER ON:"); tft.setCursor(120,100); tft.print(D_Humi_ON, 1); tft.print(" %"); break;
    case 7: tft.setCursor(10,50); tft.print("8. DEHUMIDIFIER OFF:"); tft.setCursor(120,100); tft.print(D_Humi_OFF, 1); tft.print(" %"); break;
    case 8: tft.setCursor(10,50); tft.print("9. HIGH TEMP FAN ON:"); tft.setCursor(120,100); tft.print(HT_Fan_ON, 1); tft.print(" C"); break;
    case 9: tft.setCursor(10,50); tft.print("10. HIGH TEMP ALARM ON:"); tft.setCursor(120,100); tft.print(HT_Alarm_ON, 1); tft.print(" C"); break;
    case 10: tft.setCursor(10,50); tft.print("11. HIGH HUMI ALARM ON:"); tft.setCursor(120,100); tft.print(HH_Alarm_ON, 1); tft.print(" %"); break;
    case 11: tft.setCursor(10,50); tft.print("12. LOW TEMP ALARM ON:"); tft.setCursor(120,100); tft.print(LT_Alarm_ON, 1); tft.print(" C"); break;
    case 12: tft.setCursor(10,50); tft.print("13. LOW HUMI ALARM ON:"); tft.setCursor(120,100); tft.print(LH_Alarm_ON, 1); tft.print(" %"); break;
    case 13: tft.setCursor(10,50); tft.print("14. MOTOR OFF PERIOD:"); tft.setCursor(120,100); tft.print(motor_off_minute,0); tft.print(" Min"); break;
    case 14: tft.setCursor(10,50); tft.print("15. MOTOR ON PERIOD:"); tft.setCursor(120,100); tft.print(motor_running_second,0); tft.print(" Sec"); break;
    case 15: tft.setCursor(10,50); tft.print("16. VENT OFF PERIOD:"); tft.setCursor(120,100); tft.print(ventilation_off_minute,0); tft.print(" Min"); break;
    case 16: tft.setCursor(10,50); tft.print("17. VENT ON PERIOD:"); tft.setCursor(120,100); tft.print(ventilation_running_second,0); tft.print(" Sec"); break;
    case 17: tft.setCursor(10,50); tft.print("18. Day:"); tft.setCursor(150,100); tft.print(day);  break;
    case 18: tft.setCursor(10,50); tft.print("19. TEMP CALIBRATION:"); tft.setCursor(120,100); tft.print(t_cal,1); tft.print(" C"); break;
    case 19: tft.setCursor(10,50); tft.print("20. HUMI CALIBRATION:"); tft.setCursor(120,100); tft.print(h_cal,1); tft.print(" %"); break;
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
    case 6: if (D_Humi_ON < 99.8) {D_Humi_ON += 0.1;} break;
    case 7: if (D_Humi_OFF < 99.8) {D_Humi_OFF += 0.1;} break;
    case 8: if (HT_Fan_ON < 99.8) {HT_Fan_ON += 0.1;} break;
    case 9: if (HT_Alarm_ON < 99.8) {HT_Alarm_ON += 0.1;} break;
    case 10: if (HH_Alarm_ON < 99.8) {HH_Alarm_ON += 0.1;} break;
    case 11: if (LT_Alarm_ON < 99.8) {LT_Alarm_ON += 0.1;} break;
    case 12: if (LH_Alarm_ON < 99.8) {LH_Alarm_ON += 0.1;} break;
    case 13: if (motor_off_minute < 999) { motor_off_minute += 1;} break; 
    case 14: if (motor_running_second < 999) { motor_running_second += 1;} break;
    case 15: if(ventilation_off_minute < 999) { ventilation_off_minute += 1;} break; 
    case 16: if(ventilation_running_second < 999) { ventilation_running_second += 1;} break;
    case 17: if(day < 31) { day += 1;} break;
    case 18: t_cal += .1; break;
    case 19: h_cal += .1; break;
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
    case 6: if (D_Humi_ON > 0) {D_Humi_ON -= 0.1;} break;
    case 7: if (D_Humi_OFF > 0) {D_Humi_OFF -= 0.1;} break;
    case 8: if (HT_Fan_ON > 0) {HT_Fan_ON -= 0.1;} break;
    case 9: if (HT_Alarm_ON > 0) {HT_Alarm_ON -= 0.1;} break;
    case 10: if (HH_Alarm_ON > 0) {HH_Alarm_ON -= 0.1;} break;
    case 11: if (LT_Alarm_ON > 0) {LT_Alarm_ON -= 0.1;} break;
    case 12: if (LH_Alarm_ON > 0) {LH_Alarm_ON -= 0.1;} break;
    case 13: if (motor_off_minute > 0) { motor_off_minute -= 1;} break; 
    case 14: if (motor_running_second > 0) { motor_running_second -= 1;} break;
    case 15: if(ventilation_off_minute > 0) { ventilation_off_minute -= 1;} break; 
    case 16: if(ventilation_running_second > 0) { ventilation_running_second -= 1;} break;
    case 17: if(day >= 1) { day -= 1;} break;
    case 18: t_cal -= .1; break;
    case 19: h_cal -= .1; break;

  }
  writeThresholdsToSD(); // Save to SD card after incrementing
}


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
