/**********

 * Some code copied from: https://lastminuteengineers.com/creating-esp32-web-server-arduino-ide/#configuring-the-esp32-web-server-in-wifi-station-sta-mode 
 * Also used this: https://randomnerdtutorials.com/esp32-vs-code-platformio-spiffs/ (and some other pages on the same website)
 * 
 * 
 * Current state: Can start a web server and allow connected client to control onboard blue LED using up/down buttons on web page

**********/

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>

#include "SPIFFS.h" // For file system (separate HTML file)

/* Network credentials are stored in network_credentials.h, enter them there */
#include "network_credentials.h"

#define ONBOARD_LED  2

const char* ssid[] = WIFI_SSID;
const char* password[] = WIFI_PASSWORD;
const char* id[] = WIFI_ID;

/*** Config ***/
const int wifi_connect_timeout_per_network = 10; // Seconds
const int wifi_scan_tries = 2; // How many scans it should do before giving up
const int wifi_scan_delay = 5; // How long to wait between scans

AsyncWebServer  server(80);

// Pre-declare functions to allow mentioning them before they are defined
void handle_OnConnect();

void handle_ledon();

void handle_ledoff();

void handle_NotFound();

String SendHTML();

void flash_led(int flashes, int on_time, int off_time);

void setup() {
  Serial.begin(115200);

  pinMode(ONBOARD_LED,OUTPUT);
  
  Serial.println("");

  // Setup WiFi  
  
  // Do up to wifi_scan_tries scans
  for (int s = 0; s < wifi_scan_tries; s++) {
    Serial.println("Scanning for available WiFi networks... ");
    flash_led(2, 100, 100);

    int n = WiFi.scanNetworks(); // n = number of networks found

    // If any network found...
    if (n != 0) {
      delay(100); // Probably not needed

      // Print out network names
      Serial.printf("%i networks found: \n", n);
      for (int j = 0; j < min(n, 10); j++) 
        Serial.printf(": %s \n", WiFi.SSID(j));
      Serial.print("\n");
      if (n > 10)
        Serial.printf("...and %i more \n", n - 10);

      // Check networks found in scan for ones provided in network_credentials.h
      for (int i = 0; i < sizeof(ssid) / sizeof(ssid[0]); i++) {  // Loop through network_credentials.h entries...
        for (int j = 0; j < n; j++) {  // Loop through scan results...

          if (WiFi.SSID(j) == ssid[i]) {
            // Matching entry found
            if (id[i] != "") { // Skolans nät (som kräver användarnamn (id) + lösen)
              Serial.print("Connecting to WPA2 network: "); 
              WiFi.begin(ssid[i], WPA2_AUTH_PEAP, "", id[i], password[i]);
            } else {
              // Connect to provided WiFi network
              Serial.print("Connecting to: ");
              WiFi.begin(ssid[i], password[i]);
            }
            Serial.println(WiFi.SSID(j));

            // Wait until connected
            Serial.print("Connecting...");
            for (int m = 1; m <= wifi_connect_timeout_per_network; m++) { 
              // Blink light (takes 1 second)
              flash_led(1, 500, 500);

              if (WiFi.status() == WL_CONNECTED)
                goto wifiConnected;

              // Connection timeout
              else if (m == wifi_connect_timeout_per_network) {
                Serial.println("");
                Serial.print("WiFi connection timeout. \n");
                WiFi.disconnect();
                delay(1000);
              } else  // Keep waiting
                Serial.print(".");
            }          
          }
        }
      }
    }
    // If no networks were found, or none were connected to
    Serial.printf("No networks found. Scanning again in %i seconds. \n", wifi_scan_delay);
    delay(wifi_scan_delay * 1000);
  }

  // Indicate connection failed
  Serial.println("\nFailed connecting to WiFi. Aborting setup.");
  flash_led(3, 1000, 1000);
  return;

wifiConnected:
  Serial.println("\n\nWiFi connected!");
  Serial.print(": IP address: ");  
  Serial.println(WiFi.localIP());
  flash_led(3, 100, 100);
   
  if(!MDNS.begin("rullgardin")) 
     Serial.println(": Error starting mDNS. ");
  else 
    Serial.println(": Also available at: rullgardin.local");

  // Handle button presses
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", SendHTML());
  });
  server.on("/ledon", [](AsyncWebServerRequest *request){
    handle_ledon();
    request->send(200);
  });
  server.on("/ledoff", [](AsyncWebServerRequest *request){
    handle_ledoff();
    request->send(200);
  });

  server.begin();
  Serial.println(": HTTP server started. ");
}

void loop() {
}

void handle_ledon() {
  digitalWrite(ONBOARD_LED, HIGH);
  Serial.println("LED turned on. ");
}

void handle_ledoff() {
  digitalWrite(ONBOARD_LED, LOW);
  Serial.println("LED turned off. ");
}

// Prepares HTML code to send to client, from PCInterface.html
String SendHTML(){

  // Mount HTML file
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return "";
  }

  File html_file = SPIFFS.open("/PCInterface.html");
  String html_string; // String for HTML file to return to client

  // Read HTML file to string
  if(!html_file){
    Serial.println("Failed to open file for reading");
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
  Serial.println("Failed loading HTML file. Returning default page. ");
  html_string = "<!DOCTYPE html> <html>\n";
  html_string +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  html_string +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  html_string +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  html_string +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  html_string +="</style>\n";
  html_string +="</head>\n";
  html_string +="<body>\n";
  html_string +="<h1>ESP32 Web Server</h1>\n";
  html_string +="<h3>Error: No HTML file was loaded</h3>\n";
  html_string +="</body>\n";
  html_string +="</html>\n";

  return html_string;
}

void flash_led(int flashes, int on_time, int off_time) {
  for (int i = 0; i < flashes; i++) {
      digitalWrite(ONBOARD_LED, HIGH);
      delay(on_time);
      digitalWrite(ONBOARD_LED, LOW);
      delay(off_time);
  }
}