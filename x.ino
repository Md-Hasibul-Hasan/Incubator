#include <WiFi.h>
#include <WiFiManager.h>
#include <WebServer.h>
#include "OneButton.h"
#include <DHT.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

IPAddress apIP(192, 168, 10, 1);  // New AP IP (e.g., 192.168.10.1)
IPAddress gateway(192, 168, 10, 1);  // Gateway, usually the same as the AP IP
IPAddress subnet(255, 255, 255, 0);  // Subnet mask

// Create a web server object
WebServer server(80);





#define HEATER_PIN 16
#define BAC_HEATER_PIN 17
#define HFIRE_PIN 26
#define DFIRE_PIN 5
#define MOTOR_F_PIN 33
#define MOTOR_B_PIN 32
#define FAN_PIN 4
#define ALARM_PIN 15

const int btn1 = 27;
const int btn2 = 14;
const int btn3 = 12;
const int btn4 = 13;


// SD Card Module
#define SD_CS      5   
// #define SD_MOSI    23  
// #define SD_MISO    19   
// #define SD_CLK     18   



#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

LiquidCrystal_I2C lcd(0x27, 20, 4); // I2C address, 20 columns, 4 rows



OneButton button1(btn1, true);
OneButton button2(btn2, true);
OneButton button3(btn3, true);
OneButton button4(btn4, true);



uint8_t esp_temp;
float temp=0;
float hum=0;
float t_cal=0;
float h_cal=0;

// Define thresholds
float Heater_ON = 37;
float Heater_OFF = 38;
float Bac_Heater_ON = 36;
float Bac_Heater_OFF = 38;
float Humi_ON = 55;
float Humi_OFF = 60;
float D_Humi_ON = 65;
float D_Humi_OFF = 60;
float HT_Fan_ON = 39;
float HT_Alarm_ON = 40;
float HH_Alarm_ON = 65;
float LT_Alarm_ON = 35;
float LH_Alarm_ON = 50;
float motor_off_minute =120;
float motor_running_second =10;
float ventilation_off_minute = 120;
float ventilation_running_second = 10;
int day = 0;


// Motor timings
int motor_forward = 1;
bool motor_func_called = true;
bool motor_status = true;

// ventilation timings
bool ventilation_status = false;

bool alarm_status=true;

unsigned long alarm_off_time = 0;
unsigned long day_count_time = 0;
unsigned long motor_start_time = 0;
unsigned long motor_ramain_time_millis =0;
unsigned long ventilation_start_time = 0;
unsigned long current_time;


bool error = false;
String sensor_msg;
String sd_msg;
String sd_write_msg;
String sd_read_msg;

// Button state flags
boolean btn1_clicked = false, btn1_double_clicked = false, btn1_long_clicked = false, btn1_long_pressing = false, btn1_long_clicked_stop = false;
boolean btn2_clicked = false, btn2_double_clicked = false, btn2_long_clicked = false, btn2_long_pressing = false, btn2_long_clicked_stop = false;
boolean btn3_clicked = false, btn3_double_clicked = false, btn3_long_clicked = false, btn3_long_pressing = false, btn3_long_clicked_stop = false;
boolean btn4_clicked = false, btn4_double_clicked = false, btn4_long_clicked = false, btn4_long_pressing = false, btn4_long_clicked_stop = false;

enum pageType { ROOT_MENU, SET_MENU, MSG_MENU };
enum pageType currPage = ROOT_MENU;

// Index for the current threshold being adjusted
int currentThreshold = 0;
const int NUM_THRESHOLDS = 20;


void readThresholdsFromSD() {
  File dataFile = SD.open("/value.txt");
  delay(20);

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
    motor_start_time = dataFile.parseFloat();
    ventilation_start_time = dataFile.parseFloat();
    t_cal = dataFile.parseFloat();
    h_cal = dataFile.parseFloat();

    delay(20);
    dataFile.close();
    Serial.println("Thresholds read from SD card.");
    sd_read_msg = "Data Reading Successful.";
  } else {
    Serial.println("Error reading value.txt");
    sd_read_msg = "Data Reading Failed!";
    error = true;
  }
}

void writeThresholdsToSD() {

   if (SD.exists("/value.txt")) {
    SD.remove("/value.txt");
    delay(30);
    }
    
    
  File dataFile = SD.open("/value.txt", FILE_WRITE);
  delay(30);

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
    dataFile.print("motor_start_time = "); dataFile.print(motor_start_time); dataFile.print("\n");
    dataFile.print("ventilation_start_time = "); dataFile.print(ventilation_start_time); dataFile.print("\n");
    dataFile.print("t_cal = "); dataFile.print(t_cal); dataFile.print("\n");
    dataFile.print("h_cal = "); dataFile.print(h_cal); dataFile.print("\n");

    delay(30);
    dataFile.close();
    Serial.println("Thresholds written to SD card.");
    sd_write_msg = "Data Written Successful.";
  } else {
    Serial.println("Error writing value.txt");
    sd_write_msg = "Data Written Failed!";
    error = true;
  }
}



void resetThresholds() {
  Heater_ON = 37;
  Heater_OFF = 38;
  Bac_Heater_ON = 36;
  Bac_Heater_OFF = 38;
  Humi_ON = 55;
  Humi_OFF = 60;
  D_Humi_ON = 65;
  D_Humi_OFF = 60;
  HT_Fan_ON = 39;
  HT_Alarm_ON = 40;
  HH_Alarm_ON = 65;
  LT_Alarm_ON = 35;
  LH_Alarm_ON = 50;
  motor_off_minute = 120;
  motor_running_second = 10;
  ventilation_off_minute = 120;
  ventilation_running_second = 10;
  day = 0;
  day_count_time = millis();
  motor_forward = 1;
  motor_start_time = millis();
  ventilation_start_time = millis();
  t_cal = 0;
  h_cal = 0;

  writeThresholdsToSD();
  

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
                OT_Fan_ON: <input id='htFanOn' type='number' value=''> C <button onclick='setThreshold(\"htFanOn\", \"HT_Fan_ON\")'>Set</button> <br><br>\
                OT_Alarm_ON: <input id='htAlarmOn' type='number' value=''> C <button onclick='setThreshold(\"htAlarmOn\", \"HT_Alarm_ON\")'>Set</button> <br><br>\
                LT_Alarm_ON: <input id='ltAlarmOn' type='number' value=''> C <button onclick='setThreshold(\"ltAlarmOn\", \"LT_Alarm_ON\")'>Set</button> <br><br>\
                Motor OFF: <input id='motorOff' type='number' value=''> min <button onclick='setThreshold(\"motorOff\", \"motor_off_minute\")'>Set</button> <br><br>\
                Motor ON: <input id='motorOn' type='number' value=''> sec <button onclick='setThreshold(\"motorOn\", \"motor_running_second\")'>Set</button> <br><br>\
            </div>\
            <div class='div3'>\
                Humidifier ON: <input id='humidifierOn' type='number' value=''> % <button onclick='setThreshold(\"humidifierOn\", \"Humi_ON\")'>Set</button> <br><br>\
                Humidifier OFF: <input id='humidifierOff' type='number' value=''> % <button onclick='setThreshold(\"humidifierOff\", \"Humi_OFF\")'>Set</button> <br><br>\
                DeHumidifier ON: <input id='dehumidifierOn' type='number' value=''> % <button onclick='setThreshold(\"dehumidifierOn\", \"D_Humi_ON\")'>Set</button> <br><br>\
                DeHumidifier OFF: <input id='dehumidifierOff' type='number' value=''> % <button onclick='setThreshold(\"dehumidifierOff\", \"D_Humi_OFF\")'>Set</button> <br><br>\
                OH_Alarm_ON: <input id='hhAlarmOn' type='number' value=''> % <button onclick='setThreshold(\"hhAlarmOn\", \"HH_Alarm_ON\")'>Set</button> <br><br>\
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
                ",\"heater\":\"" + (digitalRead(HEATER_PIN) ? "OFF" : "ON") + "\"" +
                ",\"backupheater\":\"" + (digitalRead(BAC_HEATER_PIN) ? "OFF" : "ON") + "\"" +
                ",\"humidifier\":\"" + (digitalRead(HFIRE_PIN) ? "OFF" : "ON") + "\"" +
                ",\"dehumidifier\":\"" + (digitalRead(DFIRE_PIN) ? "OFF" : "ON") + "\"" +
                ",\"fan\":\"" + (digitalRead(FAN_PIN) ? "OFF" : "ON") + "\"" +
                ",\"alarm\":\"" + (digitalRead(ALARM_PIN) ? "OFF" : "ON") + "\"" +
                ",\"motor\":\"" + (!digitalRead(MOTOR_F_PIN) ? "RunF" : (!digitalRead(MOTOR_B_PIN) ? "RunB" : "OFF")) + "\"" +
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
    lcd.clear();
    lcd.setCursor(3, 2);
    lcd.print("....Reset....");
    delay(500); // Show the reset message for a short time
    lcd.clear();
    server.send(200, "text/plain", "Thresholds Reset");
}


void handleSet() {
  if (server.hasArg("param") && server.hasArg("value")) {
    String param = server.arg("param");
    String value = server.arg("value");
    lcd.setCursor(0, 2);
    lcd.print("                  "); // Clear the old value

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

#ifdef __cplusplus
extern "C" {
    #endif
    uint8_t temprature_sens_read();
    #ifdef __cplusplus
}
#endif


void setup() {

    // Initialize pins as INPUT_PULLUP first to prevent relay triggering
    pinMode(HEATER_PIN, INPUT_PULLUP);
    pinMode(BAC_HEATER_PIN, INPUT_PULLUP);
    pinMode(HFIRE_PIN, INPUT_PULLUP);
    pinMode(DFIRE_PIN, INPUT_PULLUP);
    pinMode(MOTOR_F_PIN, INPUT_PULLUP);
    pinMode(MOTOR_B_PIN, INPUT_PULLUP);
    pinMode(FAN_PIN, INPUT_PULLUP);
    // pinMode(ALARM_PIN, INPUT_PULLUP);

    delay(100);

    // Set all pins as OUTPUT after ensuring they are HIGH
    pinMode(HEATER_PIN, OUTPUT);
    pinMode(BAC_HEATER_PIN, OUTPUT);
    pinMode(HFIRE_PIN, OUTPUT);
    pinMode(DFIRE_PIN, OUTPUT);
    pinMode(MOTOR_F_PIN, OUTPUT);
    pinMode(MOTOR_B_PIN, OUTPUT);
    pinMode(FAN_PIN, OUTPUT);
    pinMode(ALARM_PIN, OUTPUT);

    // Ensure relays are OFF (HIGH) on startup (for active-low relays)
    digitalWrite(HEATER_PIN, HIGH);
    digitalWrite(BAC_HEATER_PIN, HIGH);
    digitalWrite(HFIRE_PIN, HIGH);
    digitalWrite(DFIRE_PIN, HIGH);
    digitalWrite(MOTOR_F_PIN, HIGH);
    digitalWrite(MOTOR_B_PIN, HIGH);
    digitalWrite(FAN_PIN, HIGH);
    // digitalWrite(ALARM_PIN, HIGH);

    delay(200);

  Serial.begin(115200);
  dht.begin();
  Wire.begin();
  lcd.begin(20, 4);
  lcd.backlight();
  lcd.clear();

  // Try to connect to WiFi with previously saved credentials
  WiFi.begin(); // Previous credentials are used automatically
  Serial.print("Connecting to WiFi");

  // Wait a little while to see if it connects
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 10) {
    // delay(100);
    Serial.print(".");
    retries++;
  }

  // Check if connected
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nCant connect to WiFi");


    // Configuring Access Point (AP Mode) as fallback
    WiFi.softAPConfig(apIP, gateway, subnet);
    WiFi.softAP("HS INCUBATOR","12345678");
    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());  
  }




  // Start the server
  server.on("/", handleRoot);
  server.on("/getData", handleGetData); 
  server.on("/reset", handleReset);
  server.on("/set", handleSet);
  server.begin();
  Serial.println("HTTP server started");



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
  if (!SD.begin(SD_CS)) { // Adjust the pin according to your setup
    Serial.println("SD card initialization failed!");
    sd_msg = "SD CARD ERROR!";
    error = true;
    // return;
  } else {
    Serial.println("SD card initialization successful");
    sd_msg = "SD CARD OK!";
  }


  // Read thresholds from SD card
  readThresholdsFromSD();
  delay(50);

  day_count_time = millis() - day_count_time;
  motor_start_time = millis() - motor_start_time;
  ventilation_start_time = millis() - ventilation_start_time;
  
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

  esp_temp = (temprature_sens_read() - 32)/1.8;
  temp = dht.readTemperature() + t_cal;
  hum = dht.readHumidity() + h_cal;

  switch (currPage) {
    case ROOT_MENU: page_RootMenu(); break;
    case SET_MENU: page_SetMenu(); break;
    case MSG_MENU: page_MsgMenu(); break;
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

  if (currPage == ROOT_MENU){
  lcd.setCursor(0, 3);
  lcd.print("T:");
  if (hours < 10) lcd.print("0");
  lcd.print(hours);
  lcd.print(":");
  if (minutes < 10) lcd.print("0");
  lcd.print(minutes);
  lcd.print(":");
  if (seconds < 10) lcd.print("0");
  lcd.print(seconds);}
}

void motor_working() {
  
  unsigned long motor_off = motor_off_minute * 60 * 1000;
  unsigned long motor_running = motor_running_second * 1000;

  if (current_time - motor_start_time <= motor_off) {
    digitalWrite(MOTOR_F_PIN, HIGH);
    digitalWrite(MOTOR_B_PIN, HIGH);
    if (currPage == ROOT_MENU){
    lcd.setCursor(0, 2);
    lcd.print("Motor:OFF ");}
    motor_ramain_time_millis = motor_off - (current_time - motor_start_time);
  } else if (current_time - motor_start_time > motor_off && current_time - motor_start_time <= (motor_off + motor_running)) {
    if (motor_forward == 1) {
      digitalWrite(MOTOR_B_PIN, HIGH);
      digitalWrite(MOTOR_F_PIN, LOW);
      if (currPage == ROOT_MENU){
      lcd.setCursor(0, 2);
      lcd.print("Motor:RunF");}
    } else {
      digitalWrite(MOTOR_F_PIN, HIGH);
      digitalWrite(MOTOR_B_PIN, LOW);
      if (currPage == ROOT_MENU){
      lcd.setCursor(0, 2);
      lcd.print("Motor:RunB");}
    }
    motor_ramain_time_millis = (motor_off + motor_running) - (current_time - motor_start_time);
  } else {
    motor_start_time = millis();
    writeThresholdsToSD(); //**********************************************//
    if (motor_forward == 1){motor_forward = 0;}
    else{motor_forward = 1;}
    motor_ramain_time_millis = 0; // Reset remaining time after transition
    if (currPage == ROOT_MENU){
    lcd.setCursor(0, 2);
    lcd.print("Motor:OFF ");}
  }

  // Call this function with the appropriate remaining time
  displayRemainingTime(motor_ramain_time_millis);
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
    writeThresholdsToSD(); //**********************************************//
  }

}


void dayCount() {

  // Check if 24 hours have passed 86400000
  if (millis() - day_count_time >= 86400000) {
    day++;
    day_count_time = millis(); // Reset the timer
    writeThresholdsToSD(); 
  }
}

void function(){

// Temperature control
  // lcd.fillRect(240, 48, lcd.width(), 18, ILI9341_RED);
  // lcd.setCursor(180, 50);
  // lcd.print("HEAT:  OFF");
  if (temp <= Heater_ON) {
    digitalWrite(HEATER_PIN, LOW);
    if (currPage == ROOT_MENU){
    lcd.setCursor(12, 0);
    lcd.print("Heat:ON1");}
  } else if (temp >= Heater_OFF) {
    digitalWrite(HEATER_PIN, HIGH);
    if (currPage == ROOT_MENU){
    lcd.setCursor(12, 0);
    lcd.print("Heat:OFF");}
  }
  if (temp <= Bac_Heater_ON) {
    digitalWrite(BAC_HEATER_PIN, LOW);
    if (currPage == ROOT_MENU){
    lcd.setCursor(12, 0);
    lcd.print("Heat:ON2");}
  } else if (temp >= Bac_Heater_OFF) {
    digitalWrite(BAC_HEATER_PIN, HIGH);
    if (currPage == ROOT_MENU){
    lcd.setCursor(12, 0);
    lcd.print("Heat:OFF");}
  }

  // Humidity control
  // lcd.fillRect(240, 78, lcd.width(), 18, ILI9341_RED);
  // lcd.setCursor(180, 80);
  // lcd.print("HUMI:  OFF");
  if (hum <= Humi_ON) {
    digitalWrite(HFIRE_PIN, LOW);
    if (currPage == ROOT_MENU){
    lcd.setCursor(12, 1);
    lcd.print("Humi: ON");}
  } else if (hum >= Humi_OFF) {
    digitalWrite(HFIRE_PIN, HIGH);
    if (currPage == ROOT_MENU){
    lcd.setCursor(12, 1);
    lcd.print("Humi:OFF");}
  }

  // DeHumidity control
  // lcd.fillRect(240, 108, lcd.width(), 18, ILI9341_RED);
  // lcd.setCursor(180, 110);
  // lcd.print("DHUM:  OFF");
  if (hum >= D_Humi_ON) {
    digitalWrite(DFIRE_PIN, LOW);
    if (currPage == ROOT_MENU){
    lcd.setCursor(12, 2);
    lcd.print("Dhum: ON");}
  } else if (hum <= Humi_OFF) {
    digitalWrite(DFIRE_PIN, HIGH);
    if (currPage == ROOT_MENU){
    lcd.setCursor(12, 2);
    lcd.print("Dhum:OFF");}
  }

  // Fan control
  // lcd.fillRect(240, 138, lcd.width(), 18, ILI9341_RED);
  // lcd.setCursor(180, 140);
  // lcd.print("FAN:   OFF");
  if (temp >= HT_Fan_ON || ventilation_status) {
    digitalWrite(FAN_PIN, LOW);
    if (currPage == ROOT_MENU){
    lcd.setCursor(12, 3);
    lcd.print("Fan : ON");}
  } else {
    digitalWrite(FAN_PIN, HIGH);
    if (currPage == ROOT_MENU){
    lcd.setCursor(12, 3);
    lcd.print("Fan :OFF");}
  }



  // Alarm control
  // lcd.fillRect(240, 168, lcd.width(), 18, ILI9341_RED);
  // lcd.setCursor(180, 170);
  // lcd.print("ALA:   OFF");

  if (alarm_status){
    if (temp >= HT_Alarm_ON || hum >= HH_Alarm_ON || temp <= LT_Alarm_ON || hum <= LH_Alarm_ON) {
      digitalWrite(ALARM_PIN, LOW);  // for relay
      // digitalWrite(ALARM_PIN, HIGH); // for active buzzer
      // tone(ALARM_PIN, 1000); // for passive buzzer

      // if (currPage == ROOT_MENU){
      // lcd.setCursor(0, 3);
      // lcd.print("ALA: ON");}
    } else {
      digitalWrite(ALARM_PIN, HIGH); // for relay
      // digitalWrite(ALARM_PIN, LOW); // for active buzzer
      // noTone(ALARM_PIN); // for passive buzzer

      // if (currPage == ROOT_MENU){
      // lcd.setCursor(180, 170);
      // lcd.print("ALA:   OFF");}
    }
  } else{
    if (temp >= HT_Alarm_ON || hum >= HH_Alarm_ON || temp <= LT_Alarm_ON || hum <= LH_Alarm_ON) {
      digitalWrite(ALARM_PIN, HIGH); // for relay
      // digitalWrite(ALARM_PIN, LOW); // for active buzzer
      // noTone(ALARM_PIN); // for passive buzzer

      // if (currPage == ROOT_MENU){
      // lcd.setCursor(180, 170);
      // lcd.print("ALA:   ON");}
    } else {
      digitalWrite(ALARM_PIN, HIGH); // for relay
      // digitalWrite(ALARM_PIN, LOW); // for active buzzer
      // noTone(ALARM_PIN); // for passive buzzer

      // if (currPage == ROOT_MENU){
      // lcd.setCursor(180, 170);
      // lcd.print("ALA:   OFF");}
    }
  }

  if (!alarm_status && (millis() - alarm_off_time >= 300000)) {
  alarm_status = true; // Reset alaram_status to true after 5 min
  }
}




void page_RootMenu(void) {
  resetButtonStates();
  
  button1.tick();
  button2.tick();
  button3.tick();
  button4.tick();

  if (motor_func_called) {motor_working();}
  ventilation_working();
  dayCount();


  // Check if any reads failed and exit early (to try again).
  if (isnan(hum) || isnan(temp)) {
    // Serial.println("Failed to read from DHT sensor!");
    sensor_msg= "SENSOR ERROR!";
    error = true;
    temp=0;
    hum=0;
    // return;
  }else{
    sensor_msg= "SENSOR OK!";
  }

  // Print the results to the serial monitor
  // Serial.print("Humidity: ");
  // Serial.print(hum);
  // Serial.print(" %\t");
  // Serial.print("Temperature: ");
  // Serial.print(temp);
  // Serial.println(" *C");


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

    function();



  if (btn2_clicked){
    btn2_clicked = false;
    alarm_status = false;
    alarm_off_time = millis();
  }

  if (btn2_double_clicked){
    btn2_double_clicked = false;
    delay(500);
    ESP.restart();  // Restart the ESP32
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
  if (btn1_long_clicked && motor_func_called) {
    currentThreshold = 0;
    lcd.clear();
    currPage = SET_MENU;
    return;
  }

  if (btn1_clicked && motor_func_called) {
    btn1_clicked = false;
    lcd.clear();
    currPage = MSG_MENU;
    return;
  }

  // Long pressing functionality
  if (btn3_long_pressing) {
    btn3_long_pressing = false;
    motor_func_called = false;
    digitalWrite(MOTOR_B_PIN, HIGH);
    delay(150);
    digitalWrite(MOTOR_F_PIN, LOW);
    lcd.setCursor(0, 2);
    lcd.print("Motor:RunF");
  }

  if (btn3_long_clicked_stop) {
    btn3_long_clicked_stop = false;
    motor_start_time = millis();
    writeThresholdsToSD(); //************************************//
    motor_func_called = true;
    motor_forward = 0;
  }

  if (btn4_long_pressing) {
    btn4_long_pressing = false;
    motor_func_called = false;
    digitalWrite(MOTOR_F_PIN, HIGH);
    delay(150);
    digitalWrite(MOTOR_B_PIN, LOW);
    lcd.setCursor(0, 2);
    lcd.print("Motor:RunB");
  }

  if (btn4_long_clicked_stop) {
    btn4_long_clicked_stop = false;
    motor_start_time = millis();
    writeThresholdsToSD(); //************************************//
    motor_func_called = true;
    motor_forward = 1;
  }


  // Click functionality
  if (btn3_clicked) {
    btn3_clicked = false;
    motor_func_called = false;
    motor_start_time = millis();
    writeThresholdsToSD(); //************************************//

    if (motor_status) {
      digitalWrite(MOTOR_B_PIN, HIGH);
      delay(150);
      digitalWrite(MOTOR_F_PIN, LOW);
      lcd.setCursor(0, 2);
      lcd.print("Motor:RunF");
      motor_status = false;
      motor_forward = 0;
    }else {
      digitalWrite(MOTOR_F_PIN, HIGH);
      digitalWrite(MOTOR_B_PIN, HIGH);
      lcd.setCursor(0, 2);
      lcd.print("Motor:Off");
      motor_status = true;
      motor_func_called = true;
    }

  }

  if (btn4_clicked) {
    btn4_clicked = false;
    motor_func_called = false;
    motor_start_time = millis();
    writeThresholdsToSD(); //************************************//

    if (motor_status) {
      digitalWrite(MOTOR_F_PIN, HIGH);
      delay(150);
      digitalWrite(MOTOR_B_PIN, LOW);
      lcd.setCursor(0, 2);
      lcd.print("Motor:RunB");
      motor_status = false;
      motor_forward = 1;
    } else {  
      digitalWrite(MOTOR_B_PIN, HIGH);
      digitalWrite(MOTOR_F_PIN, HIGH);
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

    motor_working();
    ventilation_working();
    dayCount();
    function();

    button1.tick();
    button2.tick();
    button3.tick();
    button4.tick();

  

  lcd.setCursor(3, 0);
  lcd.println(F(">>SET_MENU<<"));  

  displayCurrentThreshold(); 



    if (btn3_clicked || btn3_long_pressing ) {
      incrementThreshold();
      btn3_clicked = false;
      btn3_long_pressing = false;

      
    }

    if (btn4_clicked || btn4_long_pressing) {
      decrementThreshold();
      btn4_clicked = false;
      btn4_long_pressing = false;

    }

    if (btn1_clicked) {
      currentThreshold = (currentThreshold + 1) % NUM_THRESHOLDS;
      btn1_clicked = false;
      lcd.setCursor(0, 2);
      lcd.print("                  ");
      lcd.setCursor(0, 3);
      lcd.print("                  ");
    }

    if (btn2_clicked) {
      currentThreshold = (currentThreshold - 1 + NUM_THRESHOLDS) % NUM_THRESHOLDS; // Ensure wrapping correctly
      btn2_clicked = false;
      lcd.setCursor(0, 2);
      lcd.print("                  ");
      lcd.setCursor(0, 3);
      lcd.print("                  ");
    }

    if (btn1_long_clicked) {
      btn1_long_clicked = false;
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print(" .....updated....");
      delay(500); // Show the reset message for a short time
      lcd.clear();
      currPage = ROOT_MENU;
      return;
    }

  }


void page_MsgMenu(void) {
  lcd.clear();
  resetButtonStates();  

  motor_working();
  ventilation_working();
  dayCount();
  function();

  button1.tick();       
  button2.tick();       
  button3.tick();       
  button4.tick();       


  tft.setCursor(0, 3);
  tft.print("CHIP TEMP: ");
  tft.print((esp_temp+esp_temp+esp_temp)/3);
  tft.print(" C");
  // Display WiFi connection status and IP address
  if (WiFi.status() == WL_CONNECTED) {
    lcd.setCursor(0, 0);
    lcd.print("Connected => ");
    lcd.print(WiFi.SSID());  // Connected SSID
    lcd.setCursor(0, 1);
    lcd.print("W_IP: ");
    lcd.print(WiFi.localIP());  // Local IP address
  } else {
    lcd.setCursor(0, 0);
    lcd.print("WiFi Not Connected");  // Connection status
    lcd.setCursor(0, 1);
    lcd.print("E_IP: ");
    lcd.print(WiFi.softAPIP());  // AP mode IP address
  }

  // Handle long press on button 1 for WiFiManager configuration
  if (btn1_long_clicked) {
    btn1_long_clicked = false;  // Reset button state

    // Activate all the devices
    digitalWrite(HEATER_PIN, HIGH);
    digitalWrite(BAC_HEATER_PIN, HIGH);
    digitalWrite(HFIRE_PIN, HIGH);
    digitalWrite(DFIRE_PIN, HIGH);
    digitalWrite(MOTOR_F_PIN, HIGH);
    digitalWrite(MOTOR_B_PIN, HIGH);
    digitalWrite(FAN_PIN, HIGH);
    digitalWrite(ALARM_PIN, HIGH);

    // Display WiFi configuration message
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("For WiFi Configuration");
    lcd.setCursor(0, 1);
    lcd.print("Visit: 192.168.10.1");
    Serial.println("Starting WiFi Config...");

    // Stop the web server and start WiFiManager
    server.stop();

    // Disconnect any ongoing WiFi connections (helps with portal start)
    // WiFi.disconnect(true);
    // delay(1000);

    WiFiManager wm;
    wm.setAPStaticIPConfig(apIP, gateway, subnet);
    wm.setConfigPortalTimeout(180);  // Timeout after 3 minutes
    bool res = wm.startConfigPortal("HS_INCUBATOR");

    if (res) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Connected => ");
      lcd.print(WiFi.SSID());
      lcd.setCursor(0, 1);
      lcd.print("W_IP: ");
      lcd.print(WiFi.localIP());
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("WiFi Config Failed");
      Serial.println("WiFiManager failed, attempting to reconnect...");

      // Try reconnecting to saved WiFi
      WiFi.begin();
      int retries = 0;
      lcd.clear();
      while (WiFi.status() != WL_CONNECTED && retries < 10) {
        lcd.setCursor(0, 0);
        lcd.print("Reconnecting...");
        retries++;
      }

      // Check reconnection status
      if (WiFi.status() == WL_CONNECTED) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Reconnected to: ");
        lcd.print(WiFi.SSID());
        lcd.setCursor(0, 1);
        lcd.print("W_IP: ");
        lcd.print(WiFi.localIP());
      } else {
        // Set AP mode if reconnection fails
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Reconnection Failed");
        lcd.setCursor(0, 1);
        lcd.print("E_IP: ");
        lcd.print(WiFi.softAPIP());

        WiFi.softAPConfig(apIP, gateway, subnet);
        WiFi.softAP("HS INCUBATOR", "12345678");
      }
    }

    // Restart the web server
    server.begin();
    Serial.println("HTTP server restarted");
  }

  // Display sensor or SD card messages if needed
  if (btn1_clicked) {
    btn1_clicked = false;  // Reset button state
    lcd.clear();
    currPage = ROOT_MENU;  // Return to ROOT_MENU
    return;
  }
}


void displayCurrentThreshold() {
  switch (currentThreshold) {
    case 0: lcd.setCursor(0,2); lcd.print("1.HEATER ON:"); lcd.setCursor(7,3); lcd.print(Heater_ON, 1); lcd.print(" C"); break;
    case 1: lcd.setCursor(0,2); lcd.print("2.HEATER OFF:"); lcd.setCursor(7,3); lcd.print(Heater_OFF, 1); lcd.print(" C"); break;
    case 2: lcd.setCursor(0,2); lcd.print("3.BACKUP HEATER ON:"); lcd.setCursor(7,3); lcd.print(Bac_Heater_ON, 1); lcd.print(" C"); break;
    case 3: lcd.setCursor(0,2); lcd.print("4.BACKUP HEATER OFF:"); lcd.setCursor(7,3); lcd.print(Bac_Heater_OFF, 1); lcd.print(" C"); break;
    case 4: lcd.setCursor(0,2); lcd.print("5.HUMIDIFIER ON:"); lcd.setCursor(7,3); lcd.print(Humi_ON, 1); lcd.print(" %"); break;
    case 5: lcd.setCursor(0,2); lcd.print("6.HUMIDIFIER OFF:"); lcd.setCursor(7,3); lcd.print(Humi_OFF, 1); lcd.print(" %"); break;
    case 6: lcd.setCursor(0,2); lcd.print("7.DEHUMIDIFIER ON:"); lcd.setCursor(7,3); lcd.print(D_Humi_ON, 1); lcd.print(" %"); break;
    case 7: lcd.setCursor(0,2); lcd.print("8.DEHUMIDIFIER OFF:"); lcd.setCursor(7,3); lcd.print(D_Humi_OFF, 1); lcd.print(" %"); break;
    case 8:lcd.setCursor(0,2); lcd.print("9.O_TEMP FAN ON:"); lcd.setCursor(7,3); lcd.print(HT_Fan_ON, 1); lcd.print(" C"); break;
    case 9: lcd.setCursor(0,2); lcd.print("10.O_TEMP ALARM ON: "); lcd.setCursor(7,3); lcd.print(HT_Alarm_ON, 1); lcd.print(" C"); break;
    case 10: lcd.setCursor(0,2); lcd.print("11.O_HUMI ALARM ON:"); lcd.setCursor(7,3); lcd.print(HH_Alarm_ON, 1); lcd.print(" %"); break;
    case 11: lcd.setCursor(0,2); lcd.print("12.L_TEMP ALARM ON:"); lcd.setCursor(7,3); lcd.print(LT_Alarm_ON, 1); lcd.print(" C"); break;
    case 12: lcd.setCursor(0,2); lcd.print("13.L_HUMI ALARM ON:"); lcd.setCursor(7,3); lcd.print(LH_Alarm_ON, 1); lcd.print(" %"); break;
    case 13: lcd.setCursor(0,2); lcd.print("14.MOTOR OFF PERIOD:"); lcd.setCursor(7,3); lcd.print(motor_off_minute, 0); lcd.print(" Min"); break;
    case 14: lcd.setCursor(0,2); lcd.print("15.MOTOR ON PERIOD:"); lcd.setCursor(7,3); lcd.print(motor_running_second, 0); lcd.print(" Sec"); break;
    case 15: lcd.setCursor(0,2); lcd.print("16.VENT OFF PERIOD:"); lcd.setCursor(7,3); lcd.print(ventilation_off_minute, 0); lcd.print(" Min"); break;
    case 16: lcd.setCursor(0,2); lcd.print("17.VENT ON PERIOD:"); lcd.setCursor(7,3); lcd.print(ventilation_running_second, 0); lcd.print(" Sec"); break;
    case 17: lcd.setCursor(0,2); lcd.print("18.Day:"); lcd.setCursor(7,3); lcd.print(day); break;
    case 18: lcd.setCursor(0,2); lcd.print("19.TEMP CALIBRATION:"); lcd.setCursor(7,3); lcd.print(t_cal, 1); lcd.print(" C"); break;
    case 19: lcd.setCursor(0,2); lcd.print("20.HUMI CALIBRATION:"); lcd.setCursor(7,3); lcd.print(h_cal, 1); lcd.print(" %"); break;
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
    case 15: if (ventilation_off_minute < 999) { ventilation_off_minute += 1;} break; 
    case 16: if (ventilation_running_second < 999) { ventilation_running_second += 1;} break;
    case 17: if (day < 31) { day += 1;} break;
    case 18: t_cal += 0.1; break;
    case 19: h_cal += 0.1; break;
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