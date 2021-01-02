#include <Servo.h> //Biblioteka odpowiedzialna za serwa

int  global_counter = 0;


class Motion {              // basic Motion class
  int   initial_position;   // initial servo position
  int   current_position;   // current servo position
  int   move_direction;     // (current) move direction (-1: not moving, LOW, HIGH)
  int   initial_direction;  // movement starting direction
  int   stop_at;            // stop at position
  int   lower_bound;        // stop/change movement direction when servo is at this lower_bound position
  int   upper_bound;        // stop/change movement direction when servo is at this upper_bound position
  int   delay_time;         // update servo position every 'delay_time' ms
  int   cycle_cntr;         // periodic movement cycle counter
  int   max_cycles;         // 1: single move, 0: unlimited periodic movemnt, > 1: limited number of cycles
  Servo *servo;             // servo pointer

public:
  void init(int pos, int l_stop, int h_stop, int del, Servo *srv);
  void single_move(int dir, int stop_at);
  void cyclic_move(int start_dir, int cycles);
  void update_position();
};

void Motion::init(int pos, int low_limit, int high_limit, int wait_time, Servo *srv) {
  initial_position = pos;
  current_position = pos;
  move_direction = -1;
  lower_bound = low_limit;
  upper_bound = high_limit;
  delay_time = wait_time;
  servo = srv;
  servo->write(current_position);
}

void Motion::single_move(int dir, int stop_pos) {
  move_direction = dir;
  stop_at = stop_pos;
  initial_direction = dir;
  cycle_cntr = 1;
  max_cycles = 1;
}

void Motion::cyclic_move(int start_dir, int cycles) {
  move_direction = start_dir;
  stop_at = current_position;
  initial_direction = start_dir;
  cycle_cntr = 1;
  max_cycles = cycles;
}

void Motion::update_position() {
  if ((move_direction == LOW or move_direction == HIGH) and global_counter % delay_time == 0) {
    if (move_direction == LOW) {
      current_position -= 1;
      if (current_position <= lower_bound) {
        move_direction = HIGH;
      }
    }
    else {
      current_position += 1;
      if (current_position >= upper_bound) {
        move_direction = LOW;
      }
    }
    
    servo->write(current_position);

    if (current_position == stop_at and move_direction == initial_direction) {
      if (cycle_cntr == max_cycles) {
        move_direction = -1;
      }
      else {
        cycle_cntr += 1;
      }
    }
  }
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
  int left_pos = 160;
  int right_pos = 95;
  
  servo_rotation.attach(9);
  rotation.init(93, 0, 179, 10, &servo_rotation);

  servo_grabbler.attach(6);
  grabbler.init(68, 35, 68, 10, &servo_grabbler);

  servo_left.attach(10);
  cable_left.init(155, 85, 179, 35, &servo_left);
  
  servo_right.attach(11);
  cable_right.init(95, 55, 179, 25, &servo_right);
  
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
      // rotation.cyclic_move(LOW, 2);
      grabbler.cyclic_move(LOW, 20);
      cable_left.cyclic_move(LOW, 2);
      // cable_right.cyclic_move(LOW, 3);
    }
  }
  
  rotation.update_position();
  grabbler.update_position();
  cable_left.update_position();
  cable_right.update_position();
  
  global_counter += 1;
  delay(1);
}
