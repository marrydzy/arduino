#include <Servo.h> //Biblioteka odpowiedzialna za serwa


class Motion {              // basic Motion class
  int   initial_position;   // initial servo position
  int   servo_position;     // current servo position
  float location;           // current location (float) 
  float step_delta;         // abs value of position increment per step (controls speed of the movement)
  int   move_direction;     // (current) move direction (-1: not moving, LOW, HIGH)
  int   initial_direction;  // movement starting direction
  int   stop_at;            // stop at position
  int   lower_bound;        // stop/change movement direction when servo is at this lower_bound position
  int   upper_bound;        // stop/change movement direction when servo is at this upper_bound position
  bool  is_master;          // movement program manager flag 
  bool  direction_changed;  // changed movement direction flag (used for counting cycles)
  int   cycle_cntr;         // periodic movement cycle counter
  int   max_cycles;         // 1: single move, 0: unlimited periodic movemnt, > 1: limited number of cycles
  Servo *servo;             // servo pointer  

public:
  void init(int pos, bool master, Servo *srv);
  void single_move(int dir, int stop_at, float velocity);
  void cyclic_move(int start_dir, int low_limit, int high_limit, float velocity, int cycles);
  void update_position();
};

void Motion::init(int pos, bool master, Servo *srv) {
  initial_position = pos;
  servo_position = pos;
  lower_bound = 55;
  upper_bound = 179;
  move_direction = -1;
  location = float(pos);
  is_master = master;
  servo = srv;
  servo->write(servo_position);
}

void Motion::single_move(int dir, int stop_pos, float velocity) {
  move_direction = dir;
  stop_at = stop_pos;
  initial_direction = dir;
  step_delta = velocity;
  cycle_cntr = 1;
  max_cycles = 1;
}

void Motion::cyclic_move(int start_dir, int low_limit, int high_limit, float velocity, int cycles) {
  move_direction = start_dir;
  if (move_direction == HIGH) {
    digitalWrite(LED_BUILTIN, HIGH);
  }
  direction_changed = true;
  lower_bound = low_limit;
  upper_bound = high_limit;
  stop_at = servo_position;
  initial_direction = start_dir;
  step_delta = velocity;
  cycle_cntr = 1;
  max_cycles = cycles;
}

void Motion::update_position() {
  if (move_direction > -1) {
    if (move_direction == LOW) {
      location -= step_delta;
      servo_position = (int)location;
      if (servo_position <= lower_bound) {
        move_direction = HIGH;
        digitalWrite(LED_BUILTIN, HIGH);
        direction_changed = true;
      }
    }
    else {
      location += step_delta;
      servo_position = (int)location;
      if (servo_position >= upper_bound) {
        move_direction = LOW;
        digitalWrite(LED_BUILTIN, LOW);
        direction_changed = true;
      }
    }
    
    servo->write(servo_position);
  
    if (is_master) {
      if (servo_position == stop_at and move_direction == initial_direction) {
        if (cycle_cntr == max_cycles) {
          move_direction = -1;
          digitalWrite(LED_BUILTIN, LOW);
        }
        else if (direction_changed == true) {
          cycle_cntr += 1;
          direction_changed = false;
        }
      }
    }
  }
}


class Arm_Motion {
  Motion l_servo;       // left servo Motion object
  Motion r_servo;       // right servo Motion object
public:
  void init(int pos_left, int pos_right, int l_stop_left, int l_stop_right, int h_stop_left, int h_stop_right,
            int wait_time, Servo *srv_l, Servo *serv_r);
  void single_move(int dir, int stop_at);
  void cyclic_move(int start_dir, int cycles);
  void update_position();
};

void Arm_Motion::update_position() {
  l_servo.update_position();
  r_servo.update_position();
}


bool switch_pressed = false;

Servo servo_rotation;
Servo servo_grabbler;
Servo servo_left;
Servo servo_right;
   
Motion rotation;
Motion grabbler;
Motion cable_left;
Motion cable_right;


void setup() 
{ 
  servo_rotation.attach(9);
  rotation.init(93, true, &servo_rotation);

  servo_grabbler.attach(6);
  grabbler.init(68, true, &servo_grabbler);

  servo_left.attach(10);
  cable_left.init(155, true, &servo_left);
  
  servo_right.attach(11);
  cable_right.init(95, true, &servo_right);
  
  pinMode(7, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);

} 

 
void loop() 
{ 
  if (digitalRead(7) == LOW) {
    switch_pressed = true;
  }
  else {
    if (switch_pressed == true) {
      switch_pressed = false;
      // cable_right.cyclic_move(HIGH, 55, 170, 0.1, 5); 
      cable_left.cyclic_move(LOW, 80, 155, 0.05, 5);
    }
  }
  
  rotation.update_position();
  grabbler.update_position();
  cable_left.update_position();
  cable_right.update_position();
  
  delay(1);
}
