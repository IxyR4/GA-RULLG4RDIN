#ifndef RULLGARDIN_cpp
#define RULLGARDIN_cpp

// #define CW 1
// #define CCW -1

#include "rullgardin.h"

// Rullgardin(uint8_t motorInterfaceType, uint8_t stepPin, int8_t dirPin, uint8_t enablePin, uint8_t resetPin) {
Rullgardin::Rullgardin() {
    // Create a new instance of the AccelStepper class
    AccelStepper motor = AccelStepper(MOTOR_INTERFACE_TYPE, STEP_PIN, DIR_PIN);

    // Set the maximum motor speed in steps per second
    motor.setMaxSpeed(2000);
    motor.setEnablePin(ENABLE_PIN);
    motor.disableOutputs();
    
    pinMode(RESET_PIN,OUTPUT);
    digitalWrite(RESET_PIN, HIGH);
}

bool Rullgardin::isRunning() { return running; }

bool Rullgardin::set_up_direction(int8_t direction) {
    if (direction == 1 || direction == -1) {
        up_direction = direction;
        down_direction = direction * -1;
        return true;
    }
    return false;
}

bool Rullgardin::run() {
    if (running) {
        motor.setSpeed(speed * current_direction);
        motor.run();
        motor.runSpeed();
        return true;
    }
    return false;
}

void Rullgardin::stop() {
    motor.disableOutputs();
    running = false;
}

void Rullgardin::open() {
    if (running && current_direction != up_direction) {
        stop();
        delay(10);
    }
    motor.enableOutputs();
    current_direction = up_direction;
    running = true;
    move_to_position(0);
}

void Rullgardin::close() {
    if (running && current_direction == up_direction) {
        stop();
        delay(10);
    }
    motor.enableOutputs();
    current_direction = up_direction * -1;
    running = true;
    move_to_position(100);
}

// Not currently implemented
void Rullgardin::move_to_position(uint8_t position) {
    return;
    motor.enableOutputs();
    motor.moveTo(position * max_steps / 100);
    running = true;
}

void Rullgardin::set_current_position_as_top() {
    motor.setCurrentPosition(0);
}

void Rullgardin::set_current_position_as_max() {
    max_steps = (motor.currentPosition() < theoretical_max_steps ? motor.currentPosition() : theoretical_max_steps); // Minimum
}

void Rullgardin::remove_max_position_limit() {
    max_steps = theoretical_max_steps;
}

bool Rullgardin::set_speed(uint16_t set_speed) {
    if (speed != set_speed && 0 < speed < motor.maxSpeed()) {
        speed = set_speed;
        return true;
    } 
    else 
        return false;
}


// private:
//     AccelStepper motor;
//     bool running = false;

//     uint16_t speed = 800;
//     uint16_t target_speed = 800;

//     int8_t up_direction = CW;
//     int8_t down_direction = CCW;
//     int8_t current_direction = CW;

//     uint32_t max_steps = 200*4*10;
//     uint32_t theoretical_max_steps = 200*4*100;

#endif