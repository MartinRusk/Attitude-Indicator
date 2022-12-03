#ifndef Stepper_h
#define Stepper_h

class Stepper {
  public:
    Stepper(int16_t motor_pin_1, int16_t motor_pin_2, int16_t motor_pin_3, int16_t motor_pin_4);
    void handle();
    void move_abs(int16_t pos);
    void move_rel(int16_t steps);
    bool in_target();
    void wait();
    void reset();
    void calibrate(int range);

  private:
    void step(int16_t step);

    int16_t step_act;
    int16_t step_target;

    // motor pin numbers:
    int16_t motor_pin_1;
    int16_t motor_pin_2;
    int16_t motor_pin_3;
    int16_t motor_pin_4;

    unsigned long step_delay; 
    unsigned long step_next; 
};

#endif