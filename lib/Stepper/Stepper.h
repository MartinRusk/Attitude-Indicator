#ifndef Stepper_h
#define Stepper_h

class Stepper
{
public:
  Stepper(uint8_t pin_1, uint8_t pin_2, uint8_t pin_3, uint8_t pin_4);
  void handle();
  void move_abs(int16_t pos);
  void move_rel(int16_t steps);
  int16_t pos();
  bool in_target();
  void wait();
  void reset();
  void calibrate(int16_t range);

private:
  void step(int16_t step);
  int16_t step_act;
  int16_t step_target;
  // motor pin numbers:
  uint8_t motor_pin_1;
  uint8_t motor_pin_2;
  uint8_t motor_pin_3;
  uint8_t motor_pin_4;

  unsigned long step_delay;
  unsigned long step_next;
};

#endif