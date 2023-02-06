#ifndef RULLGARDIN_h
#define RULLGARDIN_h

#define CW 1
#define CCW -1

#include "interface.h"

class Rullgardin {
    public:
    
    // Rullgardin(uint8_t motorInterfaceType, uint8_t stepPin, int8_t dirPin, uint8_t enablePin, uint8_t resetPin) {
    Rullgardin(int8_t up_direction_ = CW) {
        // Create a new instance of the AccelStepper class
        AccelStepper motor = AccelStepper(MOTOR_INTERFACE_TYPE, STEP_PIN, DIR_PIN);

        // Set the maximum motor speed in steps per second
        motor.setMaxSpeed(2000);
        motor.setEnablePin(ENABLE_PIN);
        motor.disableOutputs();
        
        pinMode(RESET_PIN,OUTPUT);
        digitalWrite(RESET_PIN, HIGH);

        up_direction = up_direction_;
    }

    bool isRunning() { return running; }

    bool run() {
        if (running) {
            motor.setSpeed(speed * current_direction);
            motor.run();
            motor.runSpeed();
            return true;
        }
        return false;
    }

    bool stop() {
        motor.disableOutputs();
        if (running) {
            running = false;
            return true;
        }
        return false;
    }

    bool open() {
        if (running && current_direction != up_direction) {
            stop();
            delay(10);
        }
        motor.enableOutputs();
        current_direction = up_direction;
        running = true;
        set_position(100);
        return false;
    }

    bool close() {
        if (running && current_direction == up_direction) {
            stop();
            delay(10);
        }
        motor.enableOutputs();
        current_direction = up_direction * -1;
        running = true;
        set_position(0);
        return false;
    }

    bool set_position(uint8_t position) {
        return false;
        motor.enableOutputs();
        current_direction = 1;
        running = true;
    }

    bool set_speed(uint16_t set_speed) {
        if (speed != set_speed) {
            speed = set_speed;
            return true;
        } 
        else 
            return false;
    }


    private:
        AccelStepper motor;
        bool running = false;
        uint16_t speed = 1000;
        uint16_t target_speed = 1000;
        int8_t up_direction = CW;
        int8_t current_direction = CW;
};

#endif