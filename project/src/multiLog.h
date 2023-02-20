#ifndef multiLog_h
#define multiLog_h

#include <stdlib.h>
#include <Arduino.h>
#include <WebSerial.h>

class MultiLogger{
public:
    void set_web_serial_enabled(bool state);

    // Print

    void print(String m = "");

    void print(const char *m);

    void print(char *m);

    void print(int m);

    void print(uint8_t m);

    void print(uint16_t m);

    void print(uint32_t m);

    void print(double m);

    void print(float m);


    // Print with New Line

    void println(String m = "");

    void println(const char *m);

    void println(char *m);

    void println(int m);

    void println(uint8_t m);

    void println(uint16_t m);

    void println(uint32_t m);

    void println(float m);

    void println(double m);


private:
    bool WebSerialEnabled = false;
};

#endif