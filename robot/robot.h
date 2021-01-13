
#ifndef Program_h
#define Program_h

#define IDLE_STATE -1
#define MOVING      0
#define BOUNCED     1
#define STOPPED     2

#define LEFT        0
#define RIGHT       1

// led status:  
#define IS_OFF      0
#define IS_ON       1
#define IS_BLINKING 2

// scenario devices:
#define PROGRAM     1
#define ROTATION    2
#define GRABBLER    3
#define ARM         4
#define LED_RED     5
#define LED_GREEN   6
#define LED_BLUE    7
#define STOPPAGE    8

// scenario actions:
#define STOP        1
#define MOVE_TO     2
#define CYCLE_TO    3
#define LED_OFF     4
#define LED_ON      5
#define LED_BLINK   6
#define SET_LOOP    7
#define LOOP        8
#define PAUSE       9

#define NONE       -1

void next_step();

class Switch {
  int  pin;
  bool was_pressed;
public:
  void init(int);
  bool pressed();
};


class LED {
  int  pin;
  int  mode;
  int  msec_on;
  int  msec_off;
  int  on_off_cntr;
  int  cycle_cntr;
  bool is_on;
  bool on_is_high;
public:
  void init(int, bool);
  void turn_off();
  void turn_on();
  void blink(int, int, int);
  void update_status();
  int  get_status();
};

class Delay {
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
  int   max_cycles;         // 1: single move, 0: unlimited periodic movemnt, > 1: limited number of cycles
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
  Action* scenario;
public:
  void init(int);
  void wait(bool);
  bool is_waiting();
  Action* get_action();
};



#endif
