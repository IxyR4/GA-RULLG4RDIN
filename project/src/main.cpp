#ifndef MAIN
#define MAIN

/**********

 * Some code copied from: https://lastminuteengineers.com/creating-esp32-web-server-arduino-ide/#configuring-the-esp32-web-server-in-wifi-station-sta-mode 
 * Also used this: https://randomnerdtutorials.com/esp32-vs-code-platformio-spiffs/ (and some other pages on the same website)
 * 
 * 
 * Current state: Can run a web server and allow connected clients to control connected motor position and speed

**********/

// #include "interface.h"

#include <stdlib.h>
#include <Arduino.h>

#include "multiLog.h"
/* Network credentials are stored in network_credentials.h, enter them there */
//-- Network related --//
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
//Local mDNS
#include <ESPmDNS.h>
// HTTP post for rullgardin.duckdns.org
#include <HTTPClient.h>
/* Network credentials are stored in network_credentials.h, enter them there */
#include "network_credentials.h"
// Wireless log output
#include <WebSerial.h>

// OTA updates
#include <AsyncElegantOTA.h>
#define OTA_USERNAME ""
#define OTA_PASSWORD ""

// File system
#include "SPIFFS.h"

#include "rullgardin.h"

#define ONBOARD_LED  2

// Remote server, where to upload internal IP
String serverURL = "http://rullgardin.duckdns.org/update-ip";


const char* ssid[] = WIFI_SSID;
const char* password[] = WIFI_PASSWORD;
const char* id[] = WIFI_ID;

/*** Config ***/
const uint8_t wifi_connect_seconds_timeout_per_network = 10; // Seconds
const uint8_t wifi_scan_tries = 2; // How many scans it should do before giving up
const uint8_t wifi_scan_delay_seconds = 5; // How long to wait between scans

bool darkMode = false;

#define DEBUG 0

AsyncWebServer  server(80);

MultiLogger multiLog;

// Pre-declare functions to allow mentioning them before they are defined
bool setup_wifi_success();
bool connect_wifi_network(String ssid, String password, String id);
void send_ip_to_remote_server();

void handle_OnConnect();
void handle_auto();
void handle_up();
void handle_down();
void handle_slider(String url);
void handle_NotFound();
void recvMsg(uint8_t *data, size_t len);

String SendHTML();

void darkmode_on();
void darkmode_off();
bool get_dark_mode();

Rullgardin rullgardin = Rullgardin();

void flash_led(uint8_t flashes, uint16_t on_time, uint16_t off_time);

void setup() {
  Serial.begin(115200);

  pinMode(ONBOARD_LED,OUTPUT);
  
  multiLog.println("");

  if (!setup_wifi_success())
    multiLog.println("Network connection failed, continuing in offline mode (which is the exact same thing as online mode).");
}

void loop() {
  rullgardin.run();
  
  // multiLog.println("Hello!");
}

bool setup_wifi_success() {
  // If only one network is configured, skip WiFi scan and connect immediately 
  if (sizeof(ssid) / sizeof(ssid[0]) == 1)
      return connect_wifi_network(ssid[0], password[0], id[0]);    

  // Do up to wifi_scan_tries scans
  for (uint8_t s = 0; s < wifi_scan_tries; s++) {
    multiLog.println("Scanning for available WiFi networks... ");
    flash_led(2, 100, 100);

    uint8_t networks_found = WiFi.scanNetworks();

    if (networks_found == 0) 
      goto noNetworksFound;

    delay(100); // Probably not needed
    
    // Print out network names
    multiLog.print(networks_found + " networks found: ");
    for (uint8_t j = 0; j < min((int)networks_found, 10); j++)
      multiLog.println(": " + WiFi.SSID(j));
    if (networks_found > 10)
      multiLog.print("...and " + String(networks_found - 10) + " more");
    multiLog.print("\n");

    // Check networks found in scan for ones provided in network_credentials.h
    for (uint8_t i = 0; i < sizeof(ssid) / sizeof(ssid[0]); i++) {  // Loop through network_credentials.h entries...
      for (uint8_t j = 0; j < networks_found; j++) {  // Loop through scan results...
        // When matching entry found, try connecting
        if (WiFi.SSID(j) == ssid[i] && connect_wifi_network(ssid[i], password[i], id[i])) 
          goto wifiConnected;
      }
    }
    // If no networks were found, or none were connected to
  noNetworksFound: 
    multiLog.print("No networks found. Scanning again in " + String(wifi_scan_delay_seconds) + " seconds.");
    delay(wifi_scan_delay_seconds * 1000);
  }
  wifiConnected:
  multiLog.println("\n\nWiFi connected!");
  multiLog.print(": IP address: ");  
  multiLog.println(WiFi.localIP().toString());
  flash_led(3, 100, 100);
  
  if(!MDNS.begin("rullgardin")) 
    multiLog.println(": Error starting mDNS. ");
  else 
    multiLog.println(": Also available at: rullgardin.local");
  
  send_ip_to_remote_server();

  // Setup Server //
  // Handle button presses
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){  // Redirect mDNS connections to internal IP due to encountering delays when using mDNS
    // request->redirect("http://" + WiFi.localIP().toString() + "/home");
    request->redirect("/home");
  });
  server.on("/home", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", SendHTML());
  });
  server.on("/auto", [](AsyncWebServerRequest *request){
    handle_auto();
    request->send(200);
  });
  server.on("/ledon", [](AsyncWebServerRequest *request){
    handle_up();
    request->send(200);
  });
  server.on("/ledoff", [](AsyncWebServerRequest *request){
    handle_down();
    request->send(200);
  });
  server.on("/slider", [](AsyncWebServerRequest *request){
    handle_slider(request->url().c_str());
    request->send(200);
  });
  server.on("/darkmode_on", [](AsyncWebServerRequest *request){
    darkmode_on();
    request->send(200);
  });
  server.on("/darkmode_off", [](AsyncWebServerRequest *request){
    darkmode_off();
    request->send(200);
  });
  server.on("/get_dark_mode", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", (get_dark_mode() ? "true" : "false"));
  });

  AsyncElegantOTA.begin(&server, OTA_USERNAME, OTA_PASSWORD);    // Start ElegantOTA
  
  WebSerial.begin(&server);
  WebSerial.msgCallback(recvMsg);
  multiLog.set_web_serial_enabled(true);

  server.begin();
  multiLog.println(": HTTP server started. ");

  return true;
}

void recvMsg(uint8_t *data, size_t len){
  multiLog.println("Received Data...");
  String d = "";
  for(int i=0; i < len; i++){
    d += char(data[i]);
  }
  multiLog.println(d);
}

bool connect_wifi_network(String ssid, String password, String id="") {
  if (id != "") { // Skolans nät (som kräver användarnamn (id) + lösen)
    multiLog.print("Connecting to WPA2 network: "); 
    WiFi.begin(ssid.c_str(), WPA2_AUTH_PEAP, "", id.c_str(), password.c_str());
  } else {
    // Connect to provided WiFi network
    multiLog.print("Connecting to: ");
    WiFi.begin(ssid.c_str(), password.c_str());
  }
  multiLog.println(ssid.c_str());

  // Wait until connected
  multiLog.print("Connecting...");
  for (uint8_t m = 1; m <= wifi_connect_seconds_timeout_per_network; m++) { 
    // Blink light (takes 1 second)
    flash_led(1, 500, 500);

    if (WiFi.status() == WL_CONNECTED)
      return true;

    // Connection timeout
    else if (m == wifi_connect_seconds_timeout_per_network) {
      multiLog.println("");
      multiLog.print("WiFi connection timeout. \n");
      WiFi.disconnect();
      delay(1000);
      return false;
    } else  // Keep waiting
      multiLog.print(".");
  }
  return false;
}

void send_ip_to_remote_server() {
  HTTPClient http;

  String serverPath = serverURL + "?newip=" + WiFi.localIP().toString();
  
  // Prepare http request
  http.begin(serverPath.c_str());
  
  multiLog.println(": Sending http GET request: " + serverPath);
  int httpResponseCode = http.GET();
  if (httpResponseCode)
    multiLog.println(": IP uploaded to remote server.");
  else
    multiLog.println(": Failed to upload IP to remote server.");
}

void handle_auto() {
  rullgardin.stop();
  digitalWrite(ONBOARD_LED, LOW);
  multiLog.println("Stopping.");
}

void handle_up() {
  rullgardin.open();
  digitalWrite(ONBOARD_LED, HIGH);
  multiLog.println("Moving up");
}

void handle_down() {
  rullgardin.close();
  digitalWrite(ONBOARD_LED, HIGH);
  multiLog.println("Moving down");
}

void handle_slider(String url) {
  uint16_t slider_position = url.substring(-1, 8).toInt(); // Remove '/slider/' from url
  
  if (rullgardin.set_speed(slider_position)) {
    multiLog.println("Setting speed: " + String(slider_position));
  } else {
    multiLog.println("Failed setting speed: " + String(slider_position));
  }
}

void darkmode_on() {darkMode = true;}
void darkmode_off() {darkMode = false;}
bool get_dark_mode() {return darkMode;}

// Prepares HTML code to send to client, from PCInterface.html
String SendHTML(){

  // Mount HTML file
  if(!SPIFFS.begin(true)){
    multiLog.println("An Error has occurred while mounting SPIFFS");
    return "";
  }

  File html_file = SPIFFS.open("/PCInterface.html");
  String html_string; // String for HTML file to return to client

  // Read HTML file to string
  if(!html_file){
    multiLog.println("Failed to open file for reading");
  } else {
    while(html_file.available()){
      html_string += (char)html_file.read();
    }
  }
  html_file.close();

  // If HTML file successfully loaded, return it 
  if (html_string != "")
    return html_string;
  
  // Default HTML page, "relic"-ish, useful for when file loading failed
  multiLog.println("Failed loading HTML file. Returning default page. ");
  html_string = "<style>html { font-family: Helvetica; } </style>\n";
  html_string +="<h1>ESP32 Web Server</h3>\n";
  html_string +="<h3>Error: No HTML file was loaded</h3>\n";

  return html_string;
}

void flash_led(uint8_t flashes, uint16_t on_time, uint16_t off_time) {
  for (uint8_t i = 0; i < flashes; i++) {
      digitalWrite(ONBOARD_LED, HIGH);
      delay(on_time);
      digitalWrite(ONBOARD_LED, LOW);
      delay(off_time);
  }
}

#endif