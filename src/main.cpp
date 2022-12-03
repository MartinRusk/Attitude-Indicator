#include <Arduino.h>
#include <Stepper.h>

Stepper bank(2, 3, 4, 5);
Stepper pitch(6, 7, 8, 9);

void setup() 
{
  // initialize the serial port:
  Serial.begin(115200);

  // calibrate
  bank.calibrate(290);
  pitch.calibrate(50);
}

void loop() 
{
  delay(100);
  bank.move_abs(180);
  bank.wait();
  delay(100);
  bank.move_abs(-180);
  bank.wait();
  delay(100);
  bank.move_abs(0);
  bank.wait();
  delay(100);
  pitch.move_abs(30);
  pitch.wait();
  delay(100);
  pitch.move_abs(-30);
  pitch.wait();
  delay(100);
  pitch.move_abs(0);
  pitch.wait();
}