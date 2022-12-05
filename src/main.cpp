#include <Arduino.h>
#include <Stepper.h>
#include <XPLDirect.h>

XPLDirect xp(&Serial);
Stepper roll(1, 0, 2, 3); // pin definitions per MICRO_HAT board
Stepper pitch(4, 5, 6, 7);

#define cal_roll 600 // half mechanical range
#define max_roll 500 // full usable range
#define scale_roll -12 // steps/°
#define cal_pitch 96
#define max_pitch 70
#define scale_pitch -4

float roll_angle;
float pitch_angle;

void setup()
{
  // initialize the serial port
  Serial.begin(XPLDIRECT_BAUDRATE);
  xp.begin("AttitudeIndicator");
  xp.registerDataRef("sim/cockpit2/gauges/indicators/roll_electric_deg_pilot", XPL_READ, 50, 0.2, &roll_angle);
  xp.registerDataRef("sim/cockpit2/gauges/indicators/pitch_electric_deg_pilot", XPL_READ, 50, 0.5, &pitch_angle);

  // calibrate
  roll.calibrate(cal_roll);
  pitch.calibrate(cal_pitch);
}


void set_roll(float angle)
{
  int16_t pos = (int16_t)(angle * scale_roll);
  pos = min(max(pos, -max_roll), max_roll);
  roll.move_abs(pos);
}

void set_pitch(float angle)
{
  int16_t pos = (int16_t)(angle * scale_pitch);
  pos = min(max(pos, -max_pitch), max_pitch);
  pitch.move_abs(pos);
}

void wait()
{
  while (!roll.in_target() || !pitch.in_target())
  {
    roll.handle();
    pitch.handle();
  }
}

void test()
{
  delay(1000);
  for (int i = -60; i <= 60; i += 1)
  {
    set_roll((float)i);
    set_pitch((float)i * 0.5);
    wait();
  }
  for (int i = 60; i >= -60; i -= 1)
  {
    set_roll((float)i);
    set_pitch((float)i * 0.5);
    wait();
  }
}

void loop()
{
  // test();
  xp.xloop();
  set_roll(roll_angle);
  set_pitch(pitch_angle);
  roll.handle();
  pitch.handle();
}
