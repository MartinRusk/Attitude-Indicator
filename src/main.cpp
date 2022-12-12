#include <Arduino.h>
#include <Stepper.h>
#include <Servo.h>
#include <XPLDirect.h>

XPLDirect xp(&Serial);
Stepper roll(0, 1, 2, 3);
Stepper pitch(4, 5, 6, 7);
Servo srvSpeed;
Servo srvVario;

#define TEST 0

// Speed Indicator
#define SPEED_PIN 8
#define SPEED_MIN 40.0
#define SPEED_MAX 175.0

// Variometer
#define VARIO_PIN 9
#define VARIO_MIN -1800.0
#define VARIO_MAX 1800.0

// Attitude Indicator
#define CAL_ROLL 600   // half mechanical range
#define MAX_ROLL 500   // full usable range
#define SCALE_ROLL -12 // steps/°
#define CAL_PITCH 96
#define MAX_PITCH 70
#define SCALE_PITCH -4

// synchronized variables
float roll_angle;
float pitch_angle;
float indicated_speed;
float variometer;

void set_roll(float angle)
{
  int16_t pos = (int16_t)(angle * SCALE_ROLL);
  pos = min(max(pos, -MAX_ROLL), MAX_ROLL);
  roll.move_abs(pos);
}

void set_pitch(float angle)
{
  int16_t pos = (int16_t)(angle * SCALE_PITCH);
  pos = min(max(pos, -MAX_PITCH), MAX_PITCH);
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

void set_speed(float speed)
{
  int time;
  if (speed < SPEED_MIN)
  {
    time = MIN_PULSE_WIDTH;
  }
  else if (speed > SPEED_MAX)
  {
    time = MAX_PULSE_WIDTH;
  }
  else
  {
    time = MIN_PULSE_WIDTH + (int)((speed - SPEED_MIN) * (MAX_PULSE_WIDTH - MIN_PULSE_WIDTH) / (SPEED_MAX - SPEED_MIN));
  }
  srvSpeed.writeMicroseconds(time);
}

void set_vario(float vario)
{
  int time;
  if (vario < VARIO_MIN)
  {
    time = MIN_PULSE_WIDTH;
  }
  else if (vario > VARIO_MAX)
  {
    time = MAX_PULSE_WIDTH;
  }
  else
  {
    time = MIN_PULSE_WIDTH + (int)((vario - VARIO_MIN) * (MAX_PULSE_WIDTH - MIN_PULSE_WIDTH) / (VARIO_MAX - VARIO_MIN));
  }
  srvVario.writeMicroseconds(time);
}

void sweep_speed()
{
  float speed = 0.0;
  while (speed < SPEED_MAX)
  {
    set_speed(speed);
    speed += 4.0;
    delay(20);
  }
  delay(2000);
  while (speed > 0.0)
  {
    set_speed(speed);
    speed -= 4.0;
    delay(20);
  }
  set_speed(0.0);
}

void sweep_vario()
{
  float vario = 0.0;
  while (vario < VARIO_MAX)
  {
    set_vario(vario);
    vario += 50.0;
    delay(20);
  }
  while (vario > VARIO_MIN)
  {
    set_vario(vario);
    vario -= 50.0;
    delay(20);
  }
  while (vario < 0.0)
  {
    set_vario(vario);
    vario += 50.0;
    delay(20);
  }
  set_vario(0.0);
}

void setup()
{
  // initialize the serial port
  Serial.begin(XPLDIRECT_BAUDRATE);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
  xp.begin("Sixpack");
#pragma GCC diagnostic pop
  xp.registerDataRef("sim/cockpit2/gauges/indicators/roll_electric_deg_pilot", XPL_READ, 50, 0.2, &roll_angle);
  xp.registerDataRef("sim/cockpit2/gauges/indicators/pitch_electric_deg_pilot", XPL_READ, 50, 0.5, &pitch_angle);
  xp.registerDataRef("sim/cockpit2/gauges/indicators/airspeed_kts_pilot", XPL_READ, 50, 0.5, &indicated_speed);
  xp.registerDataRef("sim/cockpit2/gauges/indicators/vvi_fpm_pilot", XPL_READ, 50, 0.5, &variometer);

  // servos
  srvSpeed.attach(SPEED_PIN);
  srvSpeed.writeMicroseconds(MIN_PULSE_WIDTH);
  srvVario.attach(VARIO_PIN);
  srvVario.writeMicroseconds((MAX_PULSE_WIDTH + MIN_PULSE_WIDTH) / 2);

  // calibrate
  roll.calibrate(CAL_ROLL);
  pitch.calibrate(CAL_PITCH);

  sweep_speed();
  sweep_vario();
}

#if TEST == 1
void loop()
{
  delay(1000);
  for (int i = -60; i <= 60; i += 1)
  {
    set_roll((float)i);
    set_pitch((float)i * 0.5);
    set_speed(60 + 2 * i);
    set_vario(i * 30.0);
    wait();
    delay(20);
  }
  for (int i = 60; i >= -60; i -= 1)
  {
    set_roll((float)i);
    set_pitch((float)i * 0.5);
    set_speed(60 + 2 * i);
    set_vario(i * 30.0);
    wait();
    delay(20);
  }
}
#else
void loop()
{
  xp.xloop();
  set_roll(roll_angle);
  set_pitch(pitch_angle);
  set_speed(indicated_speed);
  set_vario(variometer);
  roll.handle();
  pitch.handle();
}
#endif