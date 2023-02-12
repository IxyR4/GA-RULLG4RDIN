#ifndef MAIN
#define MAIN

/**********

 * Some code copied from: https://lastminuteengineers.com/creating-esp32-web-server-arduino-ide/#configuring-the-esp32-web-server-in-wifi-station-sta-mode 
 * Also used this: https://randomnerdtutorials.com/esp32-vs-code-platformio-spiffs/ (and some other pages on the same website)
 * 
 * 
 * Current state: Can run a web server and allow connected clients to control connected motor position and speed

**********/

#include "interface.h"

/* Network credentials are stored in network_credentials.h, enter them there */

// Remote server, where to upload internal IP
String serverURL = "http://rullgardin.duckdns.org/update-ip";


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
void send_ip_to_remote_server();

void handle_OnConnect();
void handle_auto();
void handle_up();
void handle_down();
void handle_slider(String url);
void handle_NotFound();

String SendHTML();

Rullgardin rullgardin = Rullgardin();

void flash_led(uint8_t flashes, uint16_t on_time, uint16_t off_time);

void setup() {
  Serial.begin(115200);

  pinMode(ONBOARD_LED,OUTPUT);
  
  Serial.println("");

  if (!setup_wifi_success())
    Serial.println("Network connection failed, continuing in offline mode (which is the exact same thing as online mode).");
}

void loop() {
  rullgardin.run();
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

  AsyncElegantOTA.begin(&server, OTA_USERNAME, OTA_PASSWORD);    // Start ElegantOTA

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

void send_ip_to_remote_server() {
  HTTPClient http;

  String serverPath = serverURL + "?newip=" + WiFi.localIP().toString();
  
  // Prepare http request
  http.begin(serverPath.c_str());
  
  Serial.println(": Sending http GET request: " + serverPath);
  int httpResponseCode = http.GET();
  if (httpResponseCode)
    Serial.println(": IP uploaded to remote server.");
  else
    Serial.println(": Failed to upload IP to remote server.");
}

void handle_auto() {
  rullgardin.stop();
  digitalWrite(ONBOARD_LED, LOW);
  Serial.println("Stopping.");
}

void handle_up() {
  rullgardin.open();
  digitalWrite(ONBOARD_LED, HIGH);
  Serial.println("Moving up");
}

void handle_down() {
  rullgardin.close();
  digitalWrite(ONBOARD_LED, HIGH);
  Serial.println("Moving down");
}

void handle_slider(String url) {
  uint16_t slider_position = url.substring(-1, 8).toInt(); // Remove '/slider/' from url
  rullgardin.set_speed(slider_position);
  Serial.printf("Setting speed: %i\n", slider_position);
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