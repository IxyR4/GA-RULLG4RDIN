/**********

 * Most code copied from: https://lastminuteengineers.com/creating-esp32-web-server-arduino-ide/#configuring-the-esp32-web-server-in-wifi-station-sta-mode 
 * Also used this: https://randomnerdtutorials.com/esp32-vs-code-platformio-spiffs/ (and some other pages on the same website)
 * 
 * There's still a lot of unnecessary code here caused by copying tutorials, sometimes marked 'relic'. 
 * 
 * Current state: Can start a web server and load a separate HTML file (/../data/PCInterface.html) to show when connecting to it 

**********/

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

#include "SPIFFS.h" // For file system (separate HTML file)

/*Put your SSID & Password*/
const char* ssid = "TP-Link Deco";  // Enter SSID here
const char* password = "19201920";  //Enter Password here

WebServer server(80);

uint8_t LED1pin = 4;
bool LED1status = LOW;

uint8_t LED2pin = 5;
bool LED2status = LOW;

// Pre-declare functions to allow mentioning them before they are defined
void handle_OnConnect();

void handle_led1on();

void handle_led1off();

void handle_led2on();

void handle_led2off();

void handle_NotFound();

String SendHTML(uint8_t led1stat,uint8_t led2stat);

// Code starts here
void setup() {
  Serial.begin(115200);

  // Relics
  delay(100);
  pinMode(LED1pin, OUTPUT);
  pinMode(LED2pin, OUTPUT);

  // Setup WiFi
  Serial.println("");
  Serial.print("Connecting to: ");
  Serial.println(ssid);

  // Connect to provided WiFi network
  WiFi.begin(ssid, password);

  // Wait until connected
  Serial.print("Connecting...");
  while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
  Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());

  // Handle button presses, relics
  server.on("/", handle_OnConnect);
  server.on("/led1on", handle_led1on);
  server.on("/led1off", handle_led1off);
  server.on("/led2on", handle_led2on);
  server.on("/led2off", handle_led2off);
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

// Mostly relics of example web page
void handle_OnConnect() {
  LED1status = LOW;
  LED2status = LOW;
  // Serial.println("GPIO4 Status: OFF | GPIO5 Status: OFF");
  server.send(200, "text/html", SendHTML(LED1status,LED2status)); // NOT COMPLETELY RELIC
}

void handle_led1on() {
  LED1status = HIGH;
  Serial.println("GPIO4 Status: ON");
  server.send(200, "text/html", SendHTML(true,LED2status)); 
}

void handle_led1off() {
  LED1status = LOW;
  Serial.println("GPIO4 Status: OFF");
  server.send(200, "text/html", SendHTML(false,LED2status)); 
}

void handle_led2on() {
  LED2status = HIGH;
  Serial.println("GPIO5 Status: ON");
  server.send(200, "text/html", SendHTML(LED1status,true)); 
}

void handle_led2off() {
  LED2status = LOW;
  Serial.println("GPIO5 Status: OFF");
  server.send(200, "text/html", SendHTML(LED1status,false)); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String SendHTML(uint8_t led1stat,uint8_t led2stat){

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

  // If HTML file was loaded, return it 
  if (html_string != "")
    return html_string;
  
  // Default HTML page, relic
  html_string = "<!DOCTYPE html> <html>\n";
  html_string +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  // html_string +="<title>LED Control</title>\n";
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