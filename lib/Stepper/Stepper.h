#ifndef Stepper_h
#define Stepper_h

class Stepper
{
public:
  Stepper(uint8_t pin_1, uint8_t pin_2, uint8_t pin_3, uint8_t pin_4);
  void handle();
  void move_abs(int32_t pos);
  void move_rel(int32_t steps);
  int32_t pos();
  bool in_target();
  void wait();
  void reset();
  void calibrate(int32_t range);
  void set_freq(uint16_t freq);

private:
  void step(int32_t step);
  int32_t step_act;
  int32_t step_target;
  // motor pin numbers:
  uint8_t motor_pin_1;
  uint8_t motor_pin_2;
  uint8_t motor_pin_3;
  uint8_t motor_pin_4;

  unsigned long step_delay;
  unsigned long step_next;
};

#endif