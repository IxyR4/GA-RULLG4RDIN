#ifndef RULLGARDIN_h
#define RULLGARDIN_h

// #include "interface.h"

#include <AccelStepper.h>

// Define stepper motor connections and motor interface type. Motor interface type must be set to 1 when using a driver:
#define DIR_PIN 27
#define STEP_PIN 26
#define ENABLE_PIN 25
#define RESET_PIN 33
#define MOTOR_INTERFACE_TYPE 1

class Rullgardin {
public:
    #define CW 1
    #define CCW -1
    
    // Rullgardin(uint8_t motorInterfaceType, uint8_t stepPin, int8_t dirPin, uint8_t enablePin, uint8_t resetPin) {
    Rullgardin();

    bool isRunning();

    bool set_up_direction(int8_t direction = CW);

    void run();

    void stop();

    void open();

    void close();

    // Not currently implemented
    void move_to_position(uint8_t position);

    void set_current_position_as_top();

    void set_current_position_as_max();

    void remove_max_position_limit();

    bool set_speed(uint16_t set_speed);


private:
    AccelStepper motor;
    bool running = false;

    uint16_t speed = 800;
    uint16_t target_speed = 800;

    int8_t up_direction = CW;
    int8_t down_direction = CCW;
    int8_t current_direction = CW;

    uint32_t max_steps = 200*4*10;
    uint32_t theoretical_max_steps = 200*4*100;
};

#endif