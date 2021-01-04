#include <Servo.h> //Biblioteka odpowiedzialna za serwa

#define STOPPED -1
#define MOVING   0
#define BOUNCED  1


bool master_stopped = false;


class Motion {              // basic Motion class
  int   servo_position;     // current servo position
  int   stop_at;            // stop at position
  int   move_direction;     // (current) move direction (STOPPED, LOW, HIGH)
  float location;           // current location (float) 
  float step_delta;         // abs value of position increment per step (controls speed of the movement)
  int   lower_bound;        // stop/change movement direction when servo is at this lower_bound position
  int   upper_bound;        // stop/change movement direction when servo is at this upper_bound position
  bool  is_master;          // movement program manager flag 
  bool  changed_position;   // used for properly counting movement cycles
  int   cycle_cntr;         // periodic movement cycle counter
  int   max_cycles;         // 1: single move, 0: unlimited periodic movemnt, > 1: limited number of cycles
  Servo *servo;             // servo pointer  

public:
  void init(int pos, Servo *srv, int servo_pin);
  void move_to(int stop_at, float velocity);
  void cycle_to(int bound_at, float velocity, bool as_master, int cycles);
  int  update_position();
  void complete_cycle_and_stop();
  void stop_moving();
  int  get_position();
  void synchronize();
};

void Motion::synchronize() {
  servo_position = (abs(upper_bound - servo_position) < abs(servo_position - lower_bound))? upper_bound : lower_bound;
  location = (float)servo_position;
  servo->write(servo_position);
}

int Motion::get_position() {
  return(servo_position);
}

void Motion::complete_cycle_and_stop() {
  if (move_direction != STOPPED) {
    cycle_cntr = max_cycles - 1;
  }
}

void Motion::stop_moving() {
  move_direction = STOPPED;
}

void Motion::init(int pos, Servo *srv, int pin) {
  servo_position = pos;
  lower_bound = 0;
  upper_bound = 179;
  move_direction = STOPPED;
  location = float(pos);
  is_master = true;
  servo = srv;
  servo->attach(pin);
  servo->write(servo_position);
}

void Motion::move_to(int stop_pos, float velocity) {
  int dir = (stop_pos > servo_position)? HIGH : LOW;
  move_direction = dir;
  stop_at = stop_pos;
  step_delta = velocity;
  cycle_cntr = 0;
  max_cycles = 1;
  is_master = true;
}

void Motion::cycle_to(int bound_at, float velocity, bool as_master = false, int cycles = 0) {
  location = (float)servo_position;
  move_direction = (bound_at > servo_position)? HIGH : LOW;
  lower_bound = (move_direction == HIGH)? servo_position: bound_at;
  upper_bound = (move_direction == HIGH)? bound_at: servo_position;
  stop_at = servo_position;
  step_delta = velocity;
  cycle_cntr = 0;
  max_cycles = cycles;
  is_master = as_master;
}

int Motion::update_position() {
  int current_position = servo_position;
  int ret_value = STOPPED;
  
  if (move_direction != STOPPED) {
    ret_value = MOVING;
    if (move_direction == LOW) {
      location -= step_delta;
      servo_position = (int)round(location);
      if (servo_position == lower_bound) {
        move_direction = HIGH;
        ret_value = BOUNCED;
      }
    }
    else {
      location += step_delta;
      servo_position = (int)round(location);
      if (servo_position == upper_bound) {
        move_direction = LOW;
        ret_value = BOUNCED;
      }
    }

    if (servo_position != current_position) {
      servo->write(servo_position);
      if (servo_position == stop_at) {
        cycle_cntr += 1;
        if (cycle_cntr == max_cycles) {
          move_direction = STOPPED;
          ret_value = STOPPED;
          if (is_master) {
            master_stopped = true;
            digitalWrite(LED_BUILTIN, LOW);
          }
        }
      }
    }
  }
  return(ret_value);
}


class Arm_Motion {
  Motion l_servo;       // left servo Motion object
  Motion r_servo;       // right servo Motion object
public:
  void init(int pos_left, Servo *srv_left, int pin_left, int pos_right, Servo *srv_right, int pin_right);
  void move_to(int dir, int stop_at);
  void cycle_to(int end_pos_left, int end_pos_right, float velocity, int cycles);
  void update_position();
  
};


void Arm_Motion::init(int pos_left, Servo *srv_left, int pin_left, int pos_right, Servo *srv_right, int pin_right) {
   l_servo.init(pos_left, srv_left, pin_left);
   r_servo.init(pos_right, srv_right, pin_right);
}

void Arm_Motion::update_position() {
  int master_status;
  int slave_status;
  bool sync = false;
  
  master_status = l_servo.update_position();
  slave_status = r_servo.update_position();
  if (master_status == STOPPED and slave_status != STOPPED) {
    r_servo.stop_moving();
    sync = true;    
  }
  if (master_status == BOUNCED or sync) {
    l_servo.synchronize();
    r_servo.synchronize();
  }
  // return(master_status);  TODO
}

void Arm_Motion::cycle_to(int end_pos_left, int end_pos_right, float velocity, int cycles) {
  // TODO - decide on-fly on master and slave servos
  int master_delta = (end_pos_left > l_servo.get_position())? end_pos_left - l_servo.get_position() : l_servo.get_position() - end_pos_left;
  int slave_delta = (end_pos_right > r_servo.get_position())? end_pos_right - r_servo.get_position() : r_servo.get_position() - end_pos_right;
  float slave_velocity = velocity * (float)slave_delta / (float)master_delta;
  
  l_servo.cycle_to(end_pos_left, velocity, true, cycles);
  r_servo.cycle_to(end_pos_right, slave_velocity);
  
}


bool switch_pressed = false;

Servo servo_rotation;
Servo servo_grabbler;
Servo servo_left;
Servo servo_right;
   
Motion rotation;
Motion grabbler;
Arm_Motion arm;


void setup() 
{ 
  pinMode(7, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  rotation.init(50, &servo_rotation, 9);
  grabbler.init(68, &servo_grabbler, 6);
  arm.init(155, &servo_left, 10, 95, &servo_right, 11);
} 

 
void loop() 
{ 

  if (digitalRead(7) == LOW) {
    switch_pressed = true;
  }
  else {
    if (switch_pressed) {
      switch_pressed = false;
      rotation.cycle_to(140, 0.05, true, 100);
      grabbler.cycle_to(35, 0.08);
      arm.cycle_to(80, 150, 0.03, 5);
      digitalWrite(LED_BUILTIN, HIGH);
    }
  }
  
  rotation.update_position();
  grabbler.update_position();
  arm.update_position();

  if (master_stopped) {
    rotation.complete_cycle_and_stop();
    grabbler.complete_cycle_and_stop();
    // arm.complete_cycle_and_stop();   TODO
    master_stopped = false;
  }
  
  delay(1);
}
