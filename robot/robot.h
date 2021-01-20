
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
  IS_BLINKING,
};


void next_step();


class Switch {
  int  pin;
  bool was_pressed;
public:
  void init(int);
  bool pressed();
};


class LED {
  int   pin;
  int   mode;
  int   msec_on;
  int   msec_off;
  int   on_off_cntr;
  int   cycle_cntr;
  int   soft_cntr;
  float soft_step;
  bool  soft_switching;
  bool  on_is_high;
public:
  void init(int, bool);
  void turn_off(bool, int);
  void turn_on(bool, int);
  void blink(bool, int, int, int);
  void update_status();
  int  get_status();
};


class Intermission {
  int counter = 0;
public:
  void set_delay(int);
  int  check_elapsed_time();
};


class Motion {              // basic Motion class
  int   servo_position;     // current servo position
  int   stop_at;            // stop at position
  int   move_direction;     // (current) move direction (IDLE_STATE, LOW, HIGH)
  float location;           // current location (float) 
  float step_delta;         // abs value of position increment per step (controls speed of the movement)
  int   lower_bound;        // stop/change movement direction when servo is at this lower_bound position
  int   upper_bound;        // stop/change movement direction when servo is at this upper_bound position
  bool  changed_position;   // used for properly counting movement cycles
  int   cycle_cntr;         // periodic movement cycle counter
  int   max_cycles;         // 1: single move, 0: unlimited periodic movemnt, > 1: specified number of cycles
  Servo *servo;             // servo pointer  

  void elementary_move(int bounce_pos, int stop_pos, float velocity);

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
  int master;
public:
  void init(int, Servo*, int, int, Servo*, int);
  void move_to(int, int, float);
  void cycle_to(int, int, float, int);
  int  update_position();
  void complete_cycle_and_stop();
};


struct Action {
  int step_nbr;
  int device;
  int action_type;
  int pos_1;
  int pos_2;
  int delta;
  int cycles;
};


class Program {
  int row = 0;
  int step_nbr = 0;
  int loop_cntr = 0;
  int loop_to_row = 0;
  int loop_to_step = 0;
  bool waiting = false;
  // Action* scenario;     TODO
public:
  Action* scenario;
  void init(int);
  void wait(bool);
  bool is_waiting();
  Action* get_action();
};



#endif
