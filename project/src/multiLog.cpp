#include "multiLog.h"

void MultiLogger::set_web_serial_enabled(bool state) {
    WebSerialEnabled = state;
}

// Print
void MultiLogger::print(String m){
    Serial.print(m);
    if (WebSerialEnabled)
        WebSerial.print(m);
}

void MultiLogger::print(const char *m){
    Serial.print(m);
    if (WebSerialEnabled)
        WebSerial.print(m);
}

void MultiLogger::print(char *m){
    Serial.print(m);
    if (WebSerialEnabled)
        WebSerial.print(m);
}

void MultiLogger::print(int m){
    Serial.print(String(m));
    if (WebSerialEnabled)
        WebSerial.print(String(m));
}

void MultiLogger::print(uint8_t m){
    Serial.print(String(m));
    if (WebSerialEnabled)
        WebSerial.print(String(m));
}

void MultiLogger::print(uint16_t m){
    Serial.print(String(m));
    if (WebSerialEnabled)
        WebSerial.print(String(m));
}

void MultiLogger::print(uint32_t m){
    Serial.print(String(m));
    if (WebSerialEnabled)
        WebSerial.print(String(m));
}

void MultiLogger::print(double m){
    Serial.print(String(m));
    if (WebSerialEnabled)
        WebSerial.print(String(m));
}

void MultiLogger::print(float m){
    Serial.print(String(m));
    if (WebSerialEnabled)
        WebSerial.print(String(m));
}


// Print with New Line

void MultiLogger::println(String m){
    Serial.println(String(m) + "\n");
    if (WebSerialEnabled)
        WebSerial.println(String(m) + "\n");
}

void MultiLogger::println(const char *m){
    Serial.println(String(m) + "\n");
    if (WebSerialEnabled)
        WebSerial.println(String(m) + "\n");
}

void MultiLogger::println(char *m){
    Serial.println(String(m) + "\n");
    if (WebSerialEnabled)
        WebSerial.println(String(m) + "\n");
}

void MultiLogger::println(int m){
    Serial.println(String(m) + "\n");
    if (WebSerialEnabled)
        WebSerial.println(String(m) + "\n");
}

void MultiLogger::println(uint8_t m){
    Serial.println(String(m) + "\n");
    if (WebSerialEnabled)
        WebSerial.println(String(m) + "\n");
}

void MultiLogger::println(uint16_t m){
    Serial.println(String(m) + "\n");
    if (WebSerialEnabled)
        WebSerial.println(String(m) + "\n");
}

void MultiLogger::println(uint32_t m){
    Serial.println(String(m) + "\n");
    if (WebSerialEnabled)
        WebSerial.println(String(m) + "\n");
}

void MultiLogger::println(float m){
    Serial.println(String(m) + "\n");
    if (WebSerialEnabled)
        WebSerial.println(String(m) + "\n");
}

void MultiLogger::println(double m){
    Serial.println(String(m) + "\n");
    if (WebSerialEnabled)
        WebSerial.println(String(m) + "\n");
}