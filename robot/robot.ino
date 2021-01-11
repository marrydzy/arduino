#include <Servo.h> //Biblioteka odpowiedzialna za serwa

#include </home/marek/Projects/arduino/robot/program.h>

void next_step(int, int);
int  prog_A(int);
int  prog_B(int);


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

void Motion::synchronize() {
  servo_position = (abs(upper_bound - servo_position) < abs(servo_position - lower_bound))? upper_bound : lower_bound;
  location = (float)servo_position;
  servo->write(servo_position);
}

int Motion::get_position() {
  return(servo_position);
}

// force stopping after completing the current cycle of the movement
void Motion::complete_cycle_and_stop() {
  if (move_direction != IDLE_STATE) {
    cycle_cntr = max_cycles - 1;
  }
}

// force immediate stop of the motion servo
void Motion::stop_moving() {
  move_direction = IDLE_STATE;
}

// initialize the motion object
void Motion::init(int pos, Servo *srv, int pin) {
  servo_position = pos;
  lower_bound = 0;
  upper_bound = 179;
  move_direction = IDLE_STATE;
  location = float(pos);
  servo = srv;
  servo->attach(pin);
  servo->write(servo_position);
}

// elementary move from the current position to stop_pos with/without bouncing at bounce_pos
void Motion::elementary_move(int bounce_pos, int stop_pos, float velocity) {
  location = (float)servo_position;
  move_direction = (bounce_pos > servo_position)? HIGH : LOW;
  lower_bound = (move_direction == HIGH)? servo_position : bounce_pos;
  upper_bound = (move_direction == HIGH)? bounce_pos : servo_position;
  stop_at = stop_pos;
  step_delta = velocity;
}


// move from the current position to the destination one (and stop there)
void Motion::move_to(int stop_pos, float velocity) {
  elementary_move(stop_pos, stop_pos, velocity);
  cycle_cntr = 0;
  max_cycles = 1;
}

// perform cyclic movement from the current position to 'bound-at' posint and back; repeat it 'cycles' times
void Motion::cycle_to(int bound_at, float velocity, int cycles = 0) {
  elementary_move(bound_at, servo_position, velocity);
  cycle_cntr = 0;
  max_cycles = cycles;
}

// elementary step of the movement
int Motion::update_position() {
  int current_position = servo_position;
  int ret_value = IDLE_STATE;
  
  if (move_direction != IDLE_STATE) {
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
          move_direction = IDLE_STATE;
          ret_value = STOPPED;
        }
      }
    }
  }
  return(ret_value);
}


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


void Arm_Motion::init(int pos_left, Servo *srv_left, int pin_left, int pos_right, Servo *srv_right, int pin_right) {
   l_servo.init(pos_left, srv_left, pin_left);
   r_servo.init(pos_right, srv_right, pin_right);
}

void Arm_Motion::complete_cycle_and_stop() {
  if (master == LEFT) {
    l_servo.complete_cycle_and_stop();
  }
  else {
    r_servo.complete_cycle_and_stop();
  }
}

// elemeentary step of arm movement
int Arm_Motion::update_position() {
  Motion *ptr_master;
  Motion *ptr_slave;
  int master_status;
  
  if (master == LEFT) {
    ptr_master = &l_servo;
    ptr_slave  = &r_servo;
  }
  else {
    ptr_master = &r_servo;
    ptr_slave  = &l_servo;
  }
  
  master_status = ptr_master->update_position();
  ptr_slave->update_position();
  if (master_status == BOUNCED or master_status == STOPPED) {
    ptr_master->synchronize();
    ptr_slave->synchronize();
    if (master_status == STOPPED) {
      ptr_slave->stop_moving(); 
    }
  }
  return(master_status);
}

// move from the current position to the destination one (and stop there)
void Arm_Motion::cycle_to(int end_pos_left, int end_pos_right, float velocity, int cycles) {
  int delta_left = (end_pos_left > l_servo.get_position())? end_pos_left - l_servo.get_position() : l_servo.get_position() - end_pos_left;
  int delta_right = (end_pos_right > r_servo.get_position())? end_pos_right - r_servo.get_position() : r_servo.get_position() - end_pos_right;

  if (delta_left >= delta_right) {
    master = LEFT;
    l_servo.cycle_to(end_pos_left, velocity, cycles);
    r_servo.cycle_to(end_pos_right, velocity * (float)delta_right/(float)delta_left);
  }
  else {
    master = RIGHT;
    l_servo.cycle_to(end_pos_left, velocity * (float)delta_left/(float)delta_right);
    r_servo.cycle_to(end_pos_right, velocity, cycles);
  }
}

void Arm_Motion::move_to(int end_pos_left, int end_pos_right, float velocity) {
  int delta_left = (end_pos_left > l_servo.get_position())? end_pos_left - l_servo.get_position() : l_servo.get_position() - end_pos_left;
  int delta_right = (end_pos_right > r_servo.get_position())? end_pos_right - r_servo.get_position() : r_servo.get_position() - end_pos_right;

  if (delta_left >= delta_right) {
    master = LEFT;
    l_servo.move_to(end_pos_left, velocity);
    r_servo.move_to(end_pos_right, velocity * (float)delta_right/(float)delta_left);
  }
  else {
    master = RIGHT;
    l_servo.move_to(end_pos_left, velocity * (float)delta_left/(float)delta_right);
    r_servo.move_to(end_pos_right, velocity);
  }
}


bool switch_pressed = false;

Servo servo_rotation;
Servo servo_grabbler;
Servo servo_left;
Servo servo_right;
   
Motion rotation;
Motion grabbler;
Arm_Motion arm;

Program program;

void setup() 
{ 
  pinMode(7, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_BLUE, HIGH);
  
  rotation.init(93, &servo_rotation, 9);
  grabbler.init(65, &servo_grabbler, 6);
  arm.init(160, &servo_left, 10, 97, &servo_right, 11);
  Serial.begin(9600);
} 

 
void loop() { 
  int program_nbr;
  if (digitalRead(7) == LOW) {
    switch_pressed = true;
  }
  else {
    if (switch_pressed) {
      switch_pressed = false;
      program_nbr = 2;            // TODO: here select program to executed
      next_step(program_nbr, 0);
    }
  }

  static bool waiting_to_stop = false;
  int rotation_status = rotation.update_position();
  int grabbler_status = grabbler.update_position();
  int arm_status = arm.update_position();

  if (rotation_status == STOPPED or grabbler_status == STOPPED or arm_status == STOPPED) {
    rotation.complete_cycle_and_stop();
    grabbler.complete_cycle_and_stop();
    arm.complete_cycle_and_stop();
    waiting_to_stop = true;
  }

  if (waiting_to_stop and rotation_status == IDLE_STATE and grabbler_status == IDLE_STATE and arm_status == IDLE_STATE) {
    next_step(program_nbr, 1);
    waiting_to_stop = false;
  }
  
  delay(1);
}


void next_step(int program, int start_from=1) {
  static int current_step = 0;

  if (start_from == 0) {
    current_step = 1;
  }

  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_BLUE, HIGH);

  if (program == 1) {
    current_step = prog_A(current_step);
  }
  else {
    current_step = prog_B(current_step);
  }
}

int prog_A(int current_step) {
  switch(current_step) {
    case 1:
      digitalWrite(LED_BUILTIN, HIGH);
      arm.cycle_to(179, 55, 0.05, 1);
      break;
    case 2:
      arm.cycle_to(179, 150, 0.05, 1);
      break;
    case 3:
      arm.cycle_to(150, 179, 0.05, 1);
      break;
    case 4:
      arm.cycle_to(55, 179, 0.05, 1);
      break;
    case 5:
      arm.cycle_to(55, 120, 0.05, 1);
      break;
    case 6:
      arm.cycle_to(120, 55, 0.05, 1);
      break;
    case 7:
      arm.move_to(179, 55, 0.05);
      break;
    case 8:
      arm.move_to(160, 97, 0.05);
      break;
    default:
      digitalWrite(LED_BUILTIN, LOW);
      break;
  } 
  return(current_step); 
}

void step_actions() {
  int* action = program.get_action();
  if (action) {
    while (action) {
      Serial.print("step:   "); Serial.println(*action++);
      Serial.print("device: "); Serial.println(*action++);
      Serial.print("action: "); Serial.println(*action++);
      Serial.print("pos_1:  "); Serial.println(*action++);
      Serial.print("pos_2:  "); 
      int value = *action++;
      if (value == NONE) {
        Serial.println("NONE");
      }
      else {
        Serial.println(value);
      }
      Serial.print("speed:  "); Serial.println(float(*action++)/1000.0);
      Serial.print("cycles: "); Serial.println(*action);
      Serial.println();
      action = program.get_action();
    }
  }
  else {
    Serial.println("Program stopped");
  }
}

int prog_B(int current_step) {
  static int inner_counter = 1;
  int   ret_value = current_step + 1;
  int   grabbler_amp = 85;
  int   grabbler_speed = 0.2;
  float rot_speed = 0.15;

  switch(current_step) {
    case 1:
      digitalWrite(LED_BUILTIN, HIGH);
      digitalWrite(LED_GREEN, LOW);
      arm.move_to(140, 97, 0.05);
      step_actions();
      break;
    case 2:
      digitalWrite(LED_GREEN, LOW);
      arm.move_to(55, 179, 0.05);
      step_actions();
      break;
    case 3:   // loop start
      digitalWrite(LED_BLUE, LOW);
      arm.cycle_to(100, 179, 0.05, 2);
      step_actions();
      break;
    case 4:
      digitalWrite(LED_BLUE, LOW);
      rotation.move_to(65, rot_speed);
      step_actions();
      break;
    case 5:
      digitalWrite(LED_BLUE, LOW);
      arm.cycle_to(100, 179, 0.05, 1);
      step_actions();
      break;
    case 6:
      digitalWrite(LED_BLUE, LOW);
      rotation.move_to(93, rot_speed);
      step_actions();
      break;
    case 7:
      digitalWrite(LED_BLUE, LOW);
      arm.cycle_to(100, 179, 0.05, 2);
      break;
    case 8:
      digitalWrite(LED_BLUE, LOW);
      rotation.move_to(125, rot_speed);
      break;
    case 9:
      digitalWrite(LED_BLUE, LOW);
      arm.cycle_to(100, 179, 0.05, 2);
      break;
    case 10:
      digitalWrite(LED_BLUE, LOW);
      rotation.move_to(93, rot_speed);
      if (inner_counter++ == 2) {
        inner_counter = 1;
      }
      else {
        ret_value = 3;
      }
      break;
    case 11:   // loop 
      digitalWrite(LED_RED, LOW);
      arm.move_to(150, 179, 0.05);
      grabbler.cycle_to(grabbler_amp, 0.2);
      break;
    case 12:
      digitalWrite(LED_RED, LOW);
      arm.move_to(179, 150, 0.05);
      grabbler.cycle_to(grabbler_amp, 0.2);
      break;
    case 13:
      digitalWrite(LED_RED, LOW);
      arm.move_to(179, 97, 0.05);
      grabbler.cycle_to(grabbler_amp, 0.2);
      break;
    case 14:
      digitalWrite(LED_RED, LOW);
      arm.move_to(179, 150, 0.05);
      grabbler.cycle_to(grabbler_amp, 0.2);
      break;
    case 15:
      digitalWrite(LED_RED, LOW);
      arm.move_to(150, 179, 0.05);
      grabbler.cycle_to(grabbler_amp, 0.2);
      break;    
    case 16:
      digitalWrite(LED_RED, LOW);
      arm.move_to(55, 179, 0.05);
      grabbler.cycle_to(grabbler_amp, 0.2);
      if (inner_counter++ == 2) {
        inner_counter = 1;
      }
      else {
        ret_value = 11;
      }
      break;
    case 17:
      digitalWrite(LED_GREEN, LOW);
      arm.move_to(160, 97, 0.05);
      break;
    default:
      digitalWrite(LED_BUILTIN, LOW);
      break;
  }
  return(ret_value);
}
