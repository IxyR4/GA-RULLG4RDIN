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
#include <AsyncElegantOTA.h>

#include "SPIFFS.h" // For file system (separate HTML file)

/* Network credentials are stored in network_credentials.h, enter them there */
#include "network_credentials.h"

#include <AccelStepper.h>

// Define stepper motor connections and motor interface type. Motor interface type must be set to 1 when using a driver:
#define dirPin 27
#define stepPin 26
#define sleepPin 25
#define resetPin 33
#define motorInterfaceType 1
 
// Create a new instance of the AccelStepper class
AccelStepper stepper = AccelStepper(motorInterfaceType, stepPin, dirPin);
bool motor_running = false;
uint16_t motor_speed = 1000;
uint16_t motor_target_speed = 1000;
int8_t motor_direction = 1;

#define ONBOARD_LED  2

const char* ssid[] = WIFI_SSID;
const char* password[] = WIFI_PASSWORD;
const char* id[] = WIFI_ID;

/*** Config ***/
const uint8_t wifi_connect_seconds_timeout_per_network = 10; // Seconds
const uint8_t wifi_scan_tries = 2; // How many scans it should do before giving up
const uint8_t wifi_scan_delay_seconds = 5; // How long to wait between scans

AsyncWebServer  server(80);

// Pre-declare functions to allow mentioning them before they are defined
bool setup_wifi_success();
bool connect_wifi_network(String ssid, String password, String id);

void handle_OnConnect();
void handle_auto();
void handle_up();
void handle_down();
void handle_slider(String url);
void handle_NotFound();

String SendHTML();

void flash_led(uint8_t flashes, uint16_t on_time, uint16_t off_time);

void setup() {
  Serial.begin(115200);

  pinMode(ONBOARD_LED,OUTPUT);
  pinMode(resetPin,OUTPUT);
  // pinMode(sleepPin,OUTPUT);

  digitalWrite(resetPin, HIGH);
  // digitalWrite(sleepPin, LOW);

  // Set the maximum motor speed in steps per second
  stepper.setMaxSpeed(2000);
  stepper.setEnablePin(sleepPin);
  stepper.disableOutputs();
  
  Serial.println("");

  if (!setup_wifi_success())
    Serial.println("Network connection failed, continuing in offline mode (which is the exact same thing as online mode).");
}

void loop() {
  if (motor_running) {
    stepper.setSpeed(motor_speed * motor_direction);
    stepper.run();
    stepper.runSpeed();
  }
}

bool setup_wifi_success() {
  // If only one network is configured, skip WiFi scan and connect immediately 
  if (sizeof(ssid) / sizeof(ssid[0]) == 1)
      return connect_wifi_network(ssid[0], password[0], id[0]);    

  // Do up to wifi_scan_tries scans
  for (uint8_t s = 0; s < wifi_scan_tries; s++) {
    Serial.println("Scanning for available WiFi networks... ");
    flash_led(2, 100, 100);

    uint8_t networks_found = WiFi.scanNetworks();

    if (networks_found == 0) 
      goto noNetworksFound;

    delay(100); // Probably not needed
    
    // Print out network names
    Serial.printf("%i networks found: \n", networks_found);
    for (uint8_t j = 0; j < min((int)networks_found, 10); j++) 
      Serial.printf(": %s \n", WiFi.SSID(j));
    if (networks_found > 10)
      Serial.printf("...and %i more", networks_found - 10);
    Serial.print("\n");

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
    Serial.printf("No networks found. Scanning again in %i seconds. \n", wifi_scan_delay_seconds);
    delay(wifi_scan_delay_seconds * 1000);
  }
  wifiConnected:
  Serial.println("\n\nWiFi connected!");
  Serial.print(": IP address: ");  
  Serial.println(WiFi.localIP());
  flash_led(3, 100, 100);
  
  if(!MDNS.begin("rullgardin")) 
    Serial.println(": Error starting mDNS. ");
  else 
    Serial.println(": Also available at: rullgardin.local");

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

  AsyncElegantOTA.begin(&server);    // Start ElegantOTA

  server.begin();
  Serial.println(": HTTP server started. ");

  return true;
}

bool connect_wifi_network(String ssid, String password, String id="") {
  if (id != "") { // Skolans nät (som kräver användarnamn (id) + lösen)
    Serial.print("Connecting to WPA2 network: "); 
    WiFi.begin(ssid.c_str(), WPA2_AUTH_PEAP, "", id.c_str(), password.c_str());
  } else {
    // Connect to provided WiFi network
    Serial.print("Connecting to: ");
    WiFi.begin(ssid.c_str(), password.c_str());
  }
  Serial.println(ssid.c_str());

  // Wait until connected
  Serial.print("Connecting...");
  for (uint8_t m = 1; m <= wifi_connect_seconds_timeout_per_network; m++) { 
    // Blink light (takes 1 second)
    flash_led(1, 500, 500);

    if (WiFi.status() == WL_CONNECTED)
      return true;

    // Connection timeout
    else if (m == wifi_connect_seconds_timeout_per_network) {
      Serial.println("");
      Serial.print("WiFi connection timeout. \n");
      WiFi.disconnect();
      delay(1000);
      return false;
    } else  // Keep waiting
      Serial.print(".");
  }
  return false;
}

void handle_auto() {
  // digitalWrite(sleepPin, LOW);
  stepper.disableOutputs();
  digitalWrite(ONBOARD_LED, LOW);
  Serial.println("Stopping.");
  motor_running = false;
}

void handle_up() {
  stepper.enableOutputs();
  digitalWrite(ONBOARD_LED, HIGH);
  Serial.println("Moving CW");
  
  delay(10);
  motor_direction = 1;
  motor_running = true;
}

void handle_down() {
  stepper.enableOutputs();
  digitalWrite(ONBOARD_LED, HIGH);
  Serial.println("Moving CCW");
  
  delay(10);
  motor_direction = -1;
  motor_running = true;
}

void handle_slider(String url) {
  uint16_t slider_position = url.substring(-1, 8).toInt(); // Remove '/slider/' from url

  if (motor_speed != slider_position) {
    Serial.printf("Setting speed: %i\n", slider_position);
    motor_speed = slider_position;
  }
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

void flash_led(uint8_t flashes, uint16_t on_time, uint16_t off_time) {
  for (uint8_t i = 0; i < flashes; i++) {
      digitalWrite(ONBOARD_LED, HIGH);
      delay(on_time);
      digitalWrite(ONBOARD_LED, LOW);
      delay(off_time);
  }
}