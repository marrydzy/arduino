
#ifndef Program_h
#define Program_h


#define LEFT        0
#define RIGHT       1
#define NONE       -1


enum devices {    // scenario devices
  PROGRAM = 1, 
  ROTATION,
  GRABBLER,
  ARM,
  LED_RED,
  LED_GREEN,
  LED_BLUE,
  PHOTO_SENSOR,
  STOPPAGE,
};

enum actions {    // scenario actions
  STOP,
  SET_LOOP,
  LOOP,
  PAUSE,
  JUMP_TO,
  GET_INPUT,
  MOVE_TO,
  CYCLE_TO,
  LED_ON,
  LED_ON_SOFT,
  LED_OFF,
  LED_OFF_SOFT,
  LED_BLINK,
  LED_BLINK_SOFT,
  ACTIVATE,
};

enum state {      // device status
  IDLE_STATE = -1,
  MOVING,
  BOUNCED,
  STOPPED,
  IS_OFF,
  IS_ON,
  IS_SWITCHING_ON,
  IS_SWITCHING_OFF,
};


void next_step();


class Switch {
  uint8_t pin;
  bool was_pressed;
public:
  void init(uint8_t);
  bool pressed();
};


class LED {
  uint8_t pin;
  uint8_t mode;
  int8_t  cycle_cntr;
  unsigned long msec_on;
  unsigned long msec_off;
  unsigned long phase_time;
  unsigned long start_time;
  float soft_step;
  bool soft_switching;
  bool on_is_high;
public:
  void init(uint8_t, bool);
  void turn_off(bool, int);
  void turn_on(bool, int);
  void blink(bool, unsigned long, unsigned long, int);
  void update_status();
  int  get_status();
};


class Intermission {
  unsigned long delay_ms = 0;
  unsigned long start_time = 0;
public:
  void set_delay(unsigned long);
  int  check_elapsed_time();
};


class Motion {              // basic Motion class
  uint8_t servo_position;   // current servo position
  uint8_t stop_at;          // stop at position
  int8_t move_direction;    // (current) move direction (IDLE_STATE, LOW, HIGH)
  uint8_t lower_bound;      // stop/change movement direction when servo is at this lower_bound position
  uint8_t upper_bound;      // stop/change movement direction when servo is at this upper_bound position
  uint8_t cycle_cntr;       // periodic movement cycle counter
  uint8_t max_cycles;       // 1: single move, 0: unlimited periodic movemnt, > 1: specified number of cycles
  float location;           // current location (float) 
  float step_delta;         // abs value of position increment per step (controls speed of the movement)
  Servo *servo;             // servo pointer  

  void elementary_move(uint8_t bounce_pos, uint8_t stop_pos, float velocity);

public:
  void init(int, Servo*, int);
  void move_to(int, float);
  void cycle_to(int, float, int);
  int  update_position();
  void complete_cycle_and_stop();
  void stop_moving();
  int  get_position();
  void synchronize();
};


class Arm_Motion {
  Motion l_servo;       // left servo Motion object
  Motion r_servo;       // right servo Motion object
  uint8_t master;
public:
  void init(int, Servo*, int, int, Servo*, int);
  void move_to(int, int, float);
  void cycle_to(int, int, float, int);
  int  update_position();
  void complete_cycle_and_stop();
};


struct Action {
  uint8_t step_nbr;
  uint8_t device;
  uint8_t action_type;
  int pos_1;
  int pos_2;
  int delta;
  int cycles;
};


class Program {
  uint8_t row = 0;
  uint8_t step_nbr = 0;
  uint8_t loop_to_row = 0;
  uint8_t loop_to_step = 0;
  int     loop_cntr = 0;
  bool    waiting = false;
public:
  Action* scenario;   // TODO
  void init(int);
  void wait(bool);
  bool is_waiting();
  Action* get_action();
};


class PhotoSensor {
  uint8_t pin;
  uint8_t servo_pos;
  int max_level;
  bool active;
  Motion* servo;
public:
  void init(int, Motion*);
  void find_max_pos();
  void measure();
  uint8_t get_max_pos();
};

#endif
