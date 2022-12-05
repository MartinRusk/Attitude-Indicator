#include <Arduino.h>
#include "Stepper.h"

// stepping scheme for the motor
const uint8_t phase_scheme[8][4] = 
{
  {1,1,0,0},
  {0,1,0,0},
  {0,1,1,0},
  {0,0,1,0},
  {0,0,1,1},
  {0,0,0,1},
  {1,0,0,1},
  {1,0,0,0}
};

// constructor
Stepper::Stepper(uint8_t pin_1, uint8_t pin_2, uint8_t pin_3, uint8_t pin_4)
{
  // Initialize variables
  step_act = 0;
  step_target = 0;
  step_delay = 3000;
  step_next = micros() + step_delay;

  // Arduino pins for the motor control connection:
  motor_pin_1 = pin_1;
  motor_pin_2 = pin_2;
  motor_pin_3 = pin_3;
  motor_pin_4 = pin_4;

  // setup the pins on the microcontroller:
  pinMode(motor_pin_1, OUTPUT);
  pinMode(motor_pin_2, OUTPUT);
  pinMode(motor_pin_3, OUTPUT);
  pinMode(motor_pin_4, OUTPUT);
}

// cyclic handle of motion (call in loop)
void Stepper::handle()
{
  // check if next step can be executed (rate limitation)
  unsigned long now = micros();
  if (now > step_next)
  {
    step_next = now + step_delay;
    // if necessary, make a step towards target
    if (step_target > step_act)
    {
      step(++step_act);
    }
    if (step_target < step_act)
    {
      step(--step_act);
    }
  }
}

// set new target position
void Stepper::move_abs(int16_t pos)
{
  step_target = pos;
}

// set relative target position
void Stepper::move_rel(int16_t steps)
{
  step_target += steps;
}

// return actual position
int16_t Stepper::pos()
{
  return (step_act);
}

// check if target position reached
bool Stepper::in_target()
{
  return (step_target == step_act);
}

// wait and handle steps until target position reached
void Stepper::wait()
{
  while (!in_target())
  {
    handle();
  }
}

// set actual position to zero
void Stepper::reset()
{
  step_act = 0;
  step_target = 0;
}

// make a calibration until block and return to center position
void Stepper::calibrate(int16_t range)
{
  move_rel(2 * range);
  wait();
  move_rel(-range);
  wait();
  reset();
}

// execute one step
void Stepper::step(int16_t step)
{
  int phase = step & 0x07;
  digitalWrite(motor_pin_1, phase_scheme[phase][0]);
  digitalWrite(motor_pin_2, phase_scheme[phase][1]);
  digitalWrite(motor_pin_3, phase_scheme[phase][2]);
  digitalWrite(motor_pin_4, phase_scheme[phase][3]);
}
