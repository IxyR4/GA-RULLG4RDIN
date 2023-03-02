#ifndef RULLGARDIN_h
#define RULLGARDIN_h

// #include "interface.h"

#include <AccelStepper.h>
#include "multiLog.h"

// Define stepper motor connections and motor interface type. Select connection version here:
// Possible values: CABLES, V1
#define V1

#if defined(CABLES)
    #define DIR_PIN 27
    #define STEP_PIN 26
    #define ENABLE_PIN 25
    #define RESET_PIN 33
    #define MOTOR_INTERFACE_TYPE 1
#elif defined(V1)
    #define DIR_PIN 32
    #define STEP_PIN 33
    #define ENABLE_PIN 25
    #define RESET_PIN 26
    #define MOTOR_INTERFACE_TYPE 1
#endif

class Rullgardin {
public:
    #define CW 1
    #define CCW -1
    
    // Rullgardin(uint8_t motorInterfaceType, uint8_t stepPin, int8_t dirPin, uint8_t enablePin, uint8_t resetPin) {
    Rullgardin();

    bool isRunning();

    bool set_up_direction(int8_t direction = CW);

    bool run();

    void stop();

    void open();

    void close();

    int get_position();

    void move_to_position(uint8_t position);

    void set_current_position_as_top();

    void set_current_position_as_max();

    void remove_max_position_limit();

    uint16_t get_speed();

    bool set_speed(uint16_t set_speed);

    uint16_t get_max_steps();


private:
    AccelStepper motor;
    bool running = false;

    uint16_t hard_max_speed = 9001;

    int8_t up_direction = CW;
    int8_t down_direction = CCW;
    int8_t current_direction = CW;

    uint32_t max_steps = 200*4*10;
    uint32_t theoretical_max_steps = 200*4*100;
};

#endif