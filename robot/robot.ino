#include <Servo.h> //Biblioteka odpowiedzialna za serwa


class Motion {              // basic Motion class
  int   initial_position;   // initial servo position
  int   servo_position;     // current servo position
  double location;           // current location (double) 
  double step_delta;         // abs value of position increment per step (controls speed of the movement)
  int   move_direction;     // (current) move direction (-1: not moving, LOW, HIGH)
  int   initial_direction;  // movement starting direction
  int   stop_at;            // stop at position
  int   lower_bound;        // stop/change movement direction when servo is at this lower_bound position
  int   upper_bound;        // stop/change movement direction when servo is at this upper_bound position
  bool  is_master;          // movement program manager flag 
  bool  changed_position;   // used for properly counting movement cycles
  int   cycle_cntr;         // periodic movement cycle counter
  int   max_cycles;         // 1: single move, 0: unlimited periodic movemnt, > 1: limited number of cycles
  Servo *servo;             // servo pointer  

public:
  void init(int pos, bool master, Servo *srv, int servo_pin);
  void move_to(int stop_at, double velocity);
  void cycle_to(int end_pos, double velocity, int cycles);
  int  update_position();
  void stop_moving();
  int  get_position();
};

int Motion::get_position() {
  return(initial_position);
}

void Motion::stop_moving() {
  move_direction = -1;  
  Serial.println(servo_position);
  Serial.println();
}

void Motion::init(int pos, bool master, Servo *srv, int pin) {
  initial_position = pos;
  servo_position = pos;
  lower_bound = 55;
  upper_bound = 179;
  move_direction = -1;
  location = double(pos);
  is_master = master;
  servo = srv;
  servo->attach(pin);
  servo->write(servo_position);
}

void Motion::move_to(int stop_pos, double velocity) {
  int dir = (stop_pos > servo_position)? HIGH : LOW;
  move_direction = dir;
  stop_at = stop_pos;
  initial_direction = dir;
  step_delta = velocity;
  cycle_cntr = 1;
  max_cycles = 1;
}

void Motion::cycle_to(int end_position, double velocity, int cycles) {
  location = (double)servo_position;
  move_direction = (end_position > servo_position)? HIGH : LOW;
  lower_bound = (move_direction == HIGH)? servo_position: end_position;
  upper_bound = (move_direction == HIGH)? end_position: servo_position;
  digitalWrite(LED_BUILTIN, move_direction);
  stop_at = servo_position;
  initial_direction = move_direction;
  step_delta = velocity;
  cycle_cntr = 1;
  max_cycles = cycles;
}

int Motion::update_position() {
  int current_position = servo_position;
  if (move_direction > -1) {
    if (move_direction == LOW) {
      location -= step_delta;
      servo_position = (int)round(location);
      if (servo_position == lower_bound) {
        move_direction = HIGH;
        digitalWrite(LED_BUILTIN, HIGH);
      }
    }
    else {
      location += step_delta;
      servo_position = (int)round(location);
      if (servo_position == upper_bound) {
        move_direction = LOW;
        digitalWrite(LED_BUILTIN, LOW);
      }
    }

    if (servo_position != current_position) {
      servo->write(servo_position);
      changed_position = true;
    }
    
    if (is_master and changed_position) {
      if (servo_position == stop_at) {
        if (cycle_cntr == max_cycles) {
          move_direction = -1;
          digitalWrite(LED_BUILTIN, LOW);
          Serial.println(servo_position);
        }
        else if (changed_position == true) {
          cycle_cntr += 1;
          changed_position = false;
        }
      }
    }
  }
  return(move_direction);
}


class Arm_Motion {
  Motion l_servo;       // left servo Motion object
  Motion r_servo;       // right servo Motion object
public:
  void init(int pos_left, Servo *srv_left, int pin_left, int pos_right, Servo *srv_right, int pin_right);
  void move_to(int dir, int stop_at);
  void cycle_to(int end_pos_left, int end_pos_right, double velocity, int cycles);
  void update_position();
};


void Arm_Motion::init(int pos_left, Servo *srv_left, int pin_left, int pos_right, Servo *srv_right, int pin_right) {
   l_servo.init(pos_left, true, srv_left, pin_left);
   r_servo.init(pos_right, false, srv_right, pin_right);    // TODO
}

void Arm_Motion::update_position() {
  int is_moving;
  int tmp;
  is_moving = l_servo.update_position();
  tmp = r_servo.update_position();
  if (tmp > -1 and is_moving == -1) {
    r_servo.stop_moving();
  }
}

void Arm_Motion::cycle_to(int end_pos_left, int end_pos_right, double velocity, int cycles) {

  int master_delta = (end_pos_left > l_servo.get_position())? end_pos_left - l_servo.get_position() : l_servo.get_position() - end_pos_left;
  int slave_delta = (end_pos_right > r_servo.get_position())? end_pos_right - r_servo.get_position() : r_servo.get_position() - end_pos_right;
  double slave_velocity = velocity * (double)slave_delta / (double)master_delta;
  Serial.print("master velocity: "); Serial.println(velocity);
  Serial.print("slave velocity:  "); Serial.println(slave_velocity);
  
  slave_velocity = velocity * double(170 - 95) / double(155 - 65);
  l_servo.cycle_to(end_pos_left, velocity, cycles);
  r_servo.cycle_to(end_pos_right, slave_velocity, 1);
  
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
  rotation.init(93, true, &servo_rotation, 9);
  grabbler.init(68, true, &servo_grabbler, 6);
  arm.init(155, &servo_left, 10, 95, &servo_right, 11);
  
  pinMode(7, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  Serial.println("setup() done!");

} 

 
void loop() 
{ 
  if (digitalRead(7) == LOW) {
    switch_pressed = true;
  }
  else {
    if (switch_pressed == true) {
      switch_pressed = false;
      arm.cycle_to(65, 170, 0.05, 10);
    }
  }
  
  rotation.update_position();
  grabbler.update_position();
  arm.update_position();

  
  delay(1);
}
