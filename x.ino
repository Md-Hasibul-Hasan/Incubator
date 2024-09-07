#include <WiFi.h>
#include <WiFiManager.h>
#include <WebServer.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <DHT.h>
#include "OneButton.h"
#include <SPI.h>
#include <SD.h>



// Create a web server object
WebServer server(80);


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


// Button state flags
boolean btn1_clicked = false, btn1_double_clicked = false, btn1_long_clicked = false, btn1_long_pressing = false, btn1_long_clicked_stop = false;
boolean btn2_clicked = false, btn2_double_clicked = false, btn2_long_clicked = false, btn2_long_pressing = false, btn2_long_clicked_stop = false;
boolean btn3_clicked = false, btn3_double_clicked = false, btn3_long_clicked = false, btn3_long_pressing = false, btn3_long_clicked_stop = false;
boolean btn4_clicked = false, btn4_double_clicked = false, btn4_long_clicked = false, btn4_long_pressing = false, btn4_long_clicked_stop = false;

enum pageType { ROOT_MENU, SET_MENU, MSG_MENU };
enum pageType currPage = ROOT_MENU;


void setup() {
  Serial.begin(115200);
  dht.begin();
  tft.begin();
  tft.setRotation(3);  
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);
  tft.fillScreen(ILI9341_BLACK);


  // Try to connect to WiFi with previously saved credentials
  WiFi.begin(); // Previous credentials are used automatically
  Serial.print("Connecting to WiFi");

  // Wait a little while to see if it connects
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 10) {
    delay(500);
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

    IPAddress apIP(192, 168, 10, 1);  // New AP IP (e.g., 192.168.10.1)
    IPAddress gateway(192, 168, 10, 1);  // Gateway, usually the same as the AP IP
    IPAddress subnet(255, 255, 255, 0);  // Subnet mask
    // Configuring Access Point (AP Mode) as fallback
    WiFi.softAPConfig(apIP, gateway, subnet);
    WiFi.softAP("HS_INCUBATOR");
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

}


void loop() {


  server.handleClient();

  current_time = millis(); 
  switch (currPage) {
    case ROOT_MENU: page_RootMenu(); break;
    case SET_MENU: page_SetMenu(); break;
    case MSG_MENU: page_MsgMenu(); break;
    default: page_RootMenu(); break;
  }
}



void page_MsgMenu(void) {
  resetButtonStates();  // Reset button states to avoid unintended actions
  button1.tick();       // Update button 1 state
  button2.tick();       // Update button 2 state
  button3.tick();       // Update button 3 state
  button4.tick();       // Update button 4 state

  tft.setTextSize(2);
  tft.fillRect(0, 0, tft.width(), 100, ILI9341_RED);

  tft.setCursor(50, 40);
  // Check WiFi status and display IP address
  if (WiFi.status() == WL_CONNECTED) {
     tft.print("SIP: ");
    tft.print(WiFi.localIP());
    tft.setCursor(50, 10);
    tft.print("WiFi Connected.");
  } else {
    tft.print("AIP: ");
    tft.print(WiFi.softAPIP());
    tft.setCursor(50, 10);
    tft.print("WiFi Not Connected.");
  }

  // Handle long press on button 1 for WiFiManager configuration
  if (btn1_long_clicked) {
    btn1_long_clicked = false;  // Reset long press state
    tft.setCursor(50, 10);
    tft.print("WiFi Config...");
    Serial.println("Starting WiFi Config...");   

    // Disconnect any ongoing WiFi connections (helps with portal start)
    WiFi.disconnect(true);
    delay(1000);

    // Custom IP for WiFiManager portal
    IPAddress wm_apIP(192, 168, 20, 1);
    IPAddress wm_gateway(192, 168, 20, 1);
    IPAddress wm_subnet(255, 255, 255, 0);

    // Start WiFiManager configuration portal
    WiFiManager wm;
    wm.setAPStaticIPConfig(wm_apIP, wm_gateway, wm_subnet);
    wm.setConfigPortalTimeout(180); // Close portal after 3 minutes
    bool res = wm.startConfigPortal("Connect Wifi","12345678");


    // After configuration, ESP32 will attempt to reconnect to WiFi
    if (res) {
      tft.setCursor(50, 10);
      tft.print("Connected to WiFi!");
      tft.setCursor(50, 40);
      tft.print("IP: ");
      tft.print(WiFi.localIP());
    } else {
      tft.setCursor(50, 10);
      tft.print("Config  Failed");
    }
  }

}