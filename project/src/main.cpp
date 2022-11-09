/**********

 * Most code copied from: https://lastminuteengineers.com/creating-esp32-web-server-arduino-ide/#configuring-the-esp32-web-server-in-wifi-station-sta-mode 
 * Also used this: https://randomnerdtutorials.com/esp32-vs-code-platformio-spiffs/ (and some other pages on the same website)
 * 
 * There's still some of unnecessary code here caused by copying tutorials, sometimes marked 'relic'. 
 * 
 * Current state: Can start a web server and load a separate HTML file (/../data/PCInterface.html) to show when connecting to it 

**********/

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

#include "SPIFFS.h" // For file system (separate HTML file)

/* Network credentials are stored in network_credentials.h, enter them there */
#include "network_credentials.h"

#define ONBOARD_LED  2

const char* ssid = WIFI_SSID;  
const char* password = WIFI_PASSWORD;  
const char* id = WIFI_ID;  

/*** Config ***/
int wifi_connect_timeout = 30; // Seconds



WebServer server(80);

// Pre-declare functions to allow mentioning them before they are defined
void handle_OnConnect();

void handle_ledon();

void handle_ledoff();

void handle_NotFound();

String SendHTML();

// Code starts here
void setup() {
  Serial.begin(115200);

  pinMode(ONBOARD_LED,OUTPUT);
  
  Serial.println("");

  // Setup WiFi  
  if (id != "") { // Skolans nät (som kräver användarnamn (id) + lösen)
    Serial.print("Connecting to WPA2 network: "); 
    WiFi.begin(ssid, WPA2_AUTH_PEAP, "", id, password);
  } else {
    // Connect to provided WiFi network
    Serial.print("Connecting to: ");
    WiFi.begin(ssid, password);
  }
  Serial.println(ssid);

  // Wait until connected
  Serial.print("Connecting...");
  // delay(1000);

  for (int i = 1; i < wifi_connect_timeout; i++) { 
    // Blink light
    for (int j=0; j<2; j++) { 
      digitalWrite(ONBOARD_LED, HIGH);
      delay(250);
      digitalWrite(ONBOARD_LED, LOW);
      delay(250);
    }

    if (WiFi.status() == WL_CONNECTED)
      break;

    //Handle connection timeout
    else if (i == wifi_connect_timeout) {
      Serial.println("");
      Serial.println("WiFi connection timeout. Aborting setup. ");
      return;
    }
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");  
  Serial.println(WiFi.localIP()); 

  // Handle button presses
  server.on("/", handle_OnConnect);
  server.on("/ledon", handle_ledon);
  server.on("/ledoff", handle_ledoff);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started. ");
}
void loop() {
  server.handleClient();

  // Relic

  // if(LED1status)
  // {digitalWrite(LED1pin, HIGH);}
  // else
  // {digitalWrite(LED1pin, LOW);}
  
  // if(LED2status)
  // {digitalWrite(LED2pin, HIGH);}
  // else
  // {digitalWrite(LED2pin, LOW);}
}

// On client connection
void handle_OnConnect() {
  server.send(200, "text/html", SendHTML());
  
  // Blink LED 
  for (int i = 0; i<3; i++) {
    digitalWrite(ONBOARD_LED,HIGH);
    delay(100);
    digitalWrite(ONBOARD_LED,LOW);
    delay(100);
  }
}

void handle_ledon() {
  digitalWrite(ONBOARD_LED, HIGH);
  Serial.println("LED turned on. ");
  server.send(200, "text/html", SendHTML()); 
}

void handle_ledoff() {
  digitalWrite(ONBOARD_LED, LOW);
  Serial.println("LED turned off. ");
  server.send(200, "text/html", SendHTML()); 
}

void handle_NotFound(){
  server.send(200, "text/html", SendHTML()); 
}

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