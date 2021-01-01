#include <Servo.h> //Biblioteka odpowiedzialna za serwa

int  global_counter = 0;


class Movement {            // basic Movement class
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

void Movement::init(int pos, int low_limit, int high_limit, int wait_time, Servo *srv) {
  initial_position = pos;
  current_position = pos;
  move_direction = -1;
  lower_bound = low_limit;
  upper_bound = high_limit;
  delay_time = wait_time;
  servo = srv;
  servo->write(current_position);
}

void Movement::single_move(int dir, int stop_pos) {
  move_direction = dir;
  stop_at = stop_pos;
  initial_direction = dir;
  cycle_cntr = 1;
  max_cycles = 1;
}

void Movement::cyclic_move(int start_dir, int cycles) {
  move_direction = start_dir;
  stop_at = current_position;
  initial_direction = start_dir;
  cycle_cntr = 1;
  max_cycles = cycles;
}

void Movement::update_position() {
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
   
Movement rotation;
Movement grabbler;


void setup() 
{ 
  servo_rotation.attach(9);
  rotation.init(93, 0, 179, 20, &servo_rotation);

  servo_grabbler.attach(6);
  grabbler.init(69, 45, 70, 5, &servo_grabbler);
  
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
      grabbler.cyclic_move(LOW, 30);
    }
  }
  
  rotation.update_position();
  grabbler.update_position();
  
  global_counter += 1;
  delay(1);
}
