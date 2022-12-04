#include <Arduino.h>
#include "Stepper.h"

Stepper::Stepper(int16_t pin_1, int16_t pin_2, int16_t pin_3, int16_t pin_4)
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

void Stepper::handle()
{
  unsigned long now = micros();
  if (now > step_next)
  {
    step_next = now + step_delay;
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

void Stepper::move_abs(int16_t pos)
{
  step_target = pos;
}

void Stepper::move_rel(int16_t steps)
{
  step_target += steps;
}

int16_t Stepper::pos()
{
  return (step_act);
}

bool Stepper::in_target()
{
  return (step_target == step_act);
}

void Stepper::wait()
{
  while(!in_target())
  {
    handle();
  }
}

void Stepper::reset()
{
  step_act = 0;
  step_target = 0;
}

void Stepper::calibrate(int16_t range)
{
  move_rel(2 * range);
  wait();
  move_rel(-range);
  wait();
  reset();
}

void Stepper::step(int16_t step)
{
  switch (step & 0x3) {
    default:
      digitalWrite(motor_pin_1, HIGH);
      digitalWrite(motor_pin_2, HIGH);
      digitalWrite(motor_pin_3, LOW);
      digitalWrite(motor_pin_4, LOW);
      break;
    case 1: 
      digitalWrite(motor_pin_1, LOW);
      digitalWrite(motor_pin_2, HIGH);
      digitalWrite(motor_pin_3, HIGH);
      digitalWrite(motor_pin_4, LOW);
      break;
    case 2: 
      digitalWrite(motor_pin_1, LOW);
      digitalWrite(motor_pin_2, LOW);
      digitalWrite(motor_pin_3, HIGH);
      digitalWrite(motor_pin_4, HIGH);
      break;
    case 3: 
      digitalWrite(motor_pin_1, HIGH);
      digitalWrite(motor_pin_2, LOW);
      digitalWrite(motor_pin_3, LOW);
      digitalWrite(motor_pin_4, HIGH);
      break;
  }
}
