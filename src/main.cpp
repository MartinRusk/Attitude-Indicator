#include <Arduino.h>
#include <Stepper.h>
#include <Servo.h>
#include <Encoder.h>
#include <Button.h>
#include <XPLDirect.h>

// activate test mode (alternative loop function)
#define TEST 0

// XPLDirect connection
XPLDirect xp(&Serial);

// Stepper motors
Stepper stpRoll(0, 1, 2, 3);
Stepper stpPitch(4, 5, 6, 7);
Stepper stpAltitude(A0, A1, A2, A3);

// Servo drives
Servo srvSpeed;
Servo srvVario;

// Input devices (TODO)
Button btnUp(15);
Button btnDn(16);
Encoder encBaro(8, 9, 4);

// Speed Indicator
#define SPEED_PIN 10
#define SPEED_MIN 40.0
#define SPEED_MAX 175.0

// Variometer
#define VARIO_PIN 14
#define VARIO_MIN -1800.0
#define VARIO_MAX 1800.0

// Attitude Indicator
#define CAL_ROLL 600   // half mechanical range
#define MAX_ROLL 500   // full usable range
#define SCALE_ROLL -12 // steps/°
#define CAL_PITCH 96
#define MAX_PITCH 70
#define SCALE_PITCH -4

// Altimeter (steps / foot)
#define SCALE_ALT 4.096

// synchronized variables
float roll_electric_deg_pilot;
float pitch_electric_deg_pilot;
float airspeed_kts_pilot;
float vvi_fpm_pilot;
float altitude_ft_pilot;

// commands
int barometer_down;
int barometer_up;
int barometer_std;

// set roll angle (°)
void set_roll(float angle)
{
  int16_t pos = (int16_t)(angle * SCALE_ROLL);
  pos = min(max(pos, -MAX_ROLL), MAX_ROLL);
  stpRoll.move_abs(pos);
}

// set pitch angle (°)
void set_pitch(float angle)
{
  int32_t pos = (int32_t)(angle * SCALE_PITCH);
  pos = min(max(pos, -MAX_PITCH), MAX_PITCH);
  stpPitch.move_abs(pos);
}

// handle steppers and wait until pitch and roll settled (fot testing)
void wait()
{
  while (!stpRoll.in_target() || !stpPitch.in_target())
  {
    stpRoll.handle();
    stpPitch.handle();
  }
}

// set altitude (feet)
void set_altitude(float altitude)
{
  int32_t pos = (int32_t)(altitude * SCALE_ALT);
  stpAltitude.move_abs(pos);
}

// set airspeed (kts)
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

// set vertical speed (fpm)
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

// make a short sweep on the speed indicator
void sweep_speed()
{
  float speed = 0.0;
  while (speed < 100)
  {
    set_speed(speed);
    speed += 4.0;
    delay(20);
  }
  while (speed > 0.0)
  {
    set_speed(speed);
    speed -= 4.0;
    delay(20);
  }
  set_speed(0.0);
}

// make a short sweep on the variometer
void sweep_vario()
{
  float vario = 0.0;
  while (vario < 500)
  {
    set_vario(vario);
    vario += 50.0;
    delay(20);
  }
  while (vario > -500)
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

// make a short sweep on the altimeter
void sweep_altitude()
{
  set_altitude(500.0);
  stpAltitude.wait();
  set_altitude(0.0);
  stpAltitude.wait();
}

// initialization
void setup()
{
  // initialize the serial port and register device
  Serial.begin(XPLDIRECT_BAUDRATE);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
  xp.begin("Sixpack");
#pragma GCC diagnostic pop

  // register DataRefs
  xp.registerDataRef("sim/cockpit2/gauges/indicators/roll_electric_deg_pilot", XPL_READ, 50, 0.2, &roll_electric_deg_pilot);
  xp.registerDataRef("sim/cockpit2/gauges/indicators/pitch_electric_deg_pilot", XPL_READ, 50, 0.5, &pitch_electric_deg_pilot);
  xp.registerDataRef("sim/cockpit2/gauges/indicators/airspeed_kts_pilot", XPL_READ, 50, 0.5, &airspeed_kts_pilot);
  xp.registerDataRef("sim/cockpit2/gauges/indicators/vvi_fpm_pilot", XPL_READ, 50, 1.0, &vvi_fpm_pilot);
  xp.registerDataRef("sim/cockpit2/gauges/indicators/altitude_ft_pilot", XPL_READ, 50, 1.0, &altitude_ft_pilot);

  // register Commands
  barometer_down = xp.registerCommand("sim/instruments/barometer_down");
  barometer_up = xp.registerCommand("sim/instruments/barometer_up");
  barometer_std = xp.registerCommand("sim/instruments/barometer_std");

  // servos
  srvSpeed.attach(SPEED_PIN);
  srvSpeed.writeMicroseconds(MIN_PULSE_WIDTH);
  srvVario.attach(VARIO_PIN);
  srvVario.writeMicroseconds((MAX_PULSE_WIDTH + MIN_PULSE_WIDTH) / 2);

  // calibrate attitude indicator
  stpRoll.set_freq(800);
  stpRoll.calibrate(CAL_ROLL);
  stpPitch.set_freq(800);
  stpPitch.calibrate(CAL_PITCH);

  // initialize altimeter
  stpAltitude.set_freq(800);
  stpAltitude.reset();

  // Sweep instruments once to check function
  sweep_speed();
  sweep_vario();
  sweep_altitude();
}

// Main loop
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
  // if XPLDirect not connected: zero instruments and enable adjustment for altimeter
  if (!xp.xloop())
  {
    roll_electric_deg_pilot = 0.0;
    pitch_electric_deg_pilot = 0.0;
    airspeed_kts_pilot = 0.0;
    vvi_fpm_pilot = 0.0;
    altitude_ft_pilot = 0.0;
    // use encoder for zero adjustment
    if (encBaro.up() || btnUp.is_pressed())
    {
      altitude_ft_pilot = 20;
    }
    if (encBaro.down() || btnDn.is_pressed())
    {
      altitude_ft_pilot = -20;
    }
    // wait until altitude corrected
    set_altitude(altitude_ft_pilot);
    stpAltitude.wait();
    stpAltitude.reset();
  }
  else
  {
    // Encoder to adjust barometer up/down
    if (encBaro.up())
    {
      xp.commandTrigger(barometer_up);
    }
    if (encBaro.down())
    {
      xp.commandTrigger(barometer_down);
    }
  }

  // evaluate DataRefs
  set_roll(roll_electric_deg_pilot);
  set_pitch(pitch_electric_deg_pilot);
  set_speed(airspeed_kts_pilot);
  set_vario(vvi_fpm_pilot);
  set_altitude(altitude_ft_pilot);

  // handle steppers
  stpRoll.handle();
  stpPitch.handle();
  stpAltitude.handle();

  // handle encoder
  encBaro.handle();
}
#endif