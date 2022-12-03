#include <Arduino.h>
#include "Stepper.h"

Stepper::Stepper(int16_t motor_pin_1, int16_t motor_pin_2, int16_t motor_pin_3, int16_t motor_pin_4)
{
  this->step_act = 0;    
  this->step_target = 0; 
  this->step_delay = 2000;
  this->step_next = micros() + this->step_delay;

  // Arduino pins for the motor control connection:
  this->motor_pin_1 = motor_pin_1;
  this->motor_pin_2 = motor_pin_2;
  this->motor_pin_3 = motor_pin_3;
  this->motor_pin_4 = motor_pin_4;

  // setup the pins on the microcontroller:
  pinMode(this->motor_pin_1, OUTPUT);
  pinMode(this->motor_pin_2, OUTPUT);
  pinMode(this->motor_pin_3, OUTPUT);
  pinMode(this->motor_pin_4, OUTPUT);
}

void Stepper::handle()
{
  unsigned long now = micros();
  if (now > this->step_next)
  {
    this->step_next = now + this->step_delay;
    if (this->step_target > this->step_act)
    {
      step(++this->step_act);
    }
    if (this->step_target < this->step_act)
    {
      step(--this->step_act);
    }
  }
}

void Stepper::move_abs(int16_t pos)
{
  this->step_target = pos;
}

void Stepper::move_rel(int16_t steps)
{
  this->step_target += steps;
}

bool Stepper::in_target()
{
  return (this->step_target == this->step_act);
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
  this->step_act = 0;
}

void Stepper::calibrate(int range)
{
  move_rel(2 * range);
  wait();
  move_rel(-range);
  wait();
  reset();
}

void Stepper::step(int16_t step)
{
  switch ((step+16384) % 4) {  // modulo can only handle positive numbers
    case 0: 
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
    default:
      digitalWrite(motor_pin_1, LOW);
      digitalWrite(motor_pin_2, LOW);
      digitalWrite(motor_pin_3, LOW);
      digitalWrite(motor_pin_4, LOW);
  }
}
