#ifndef INTERFACE_h
#define INTERFACE_h

// Define stepper motor connections and motor interface type. Motor interface type must be set to 1 when using a driver:
#define DIR_PIN 27
#define STEP_PIN 26
#define ENABLE_PIN 25
#define RESET_PIN 33
#define MOTOR_INTERFACE_TYPE 1

#define OTA_USERNAME ""
#define OTA_PASSWORD ""

#define ONBOARD_LED  2

#include <stdlib.h>
#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <AsyncElegantOTA.h>

#include <HTTPClient.h>

#include "SPIFFS.h" // For file system (separate HTML file)


#include <AccelStepper.h>

/* Network credentials are stored in network_credentials.h, enter them there */
#include "network_credentials.h"

#include "rullgardin.h"


#endif