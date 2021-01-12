#include <Servo.h> //Biblioteka odpowiedzialna za serwa
#include </home/marek/Projects/arduino/robot/robot.h>



void Switch::init(int pin_nbr) {
  pin = pin_nbr;
  pinMode(pin, INPUT_PULLUP);
  was_pressed = false;
}

bool Switch::pressed() {
  if (digitalRead(7) == LOW) {
    was_pressed = true;
  }
  else {
    if (was_pressed) {
      was_pressed = false;
      return(true);
    }
  }
  return(false);
}


void LED::init(int pin_nbr, bool high_on) {
  pin = pin_nbr;
  on_is_high = high_on;
  pinMode(pin_nbr, OUTPUT);
  turn_off(); 
}

void LED::turn_off() {
  digitalWrite(pin, (on_is_high)? LOW : HIGH);
  mode = IS_OFF;
  is_on = false;
}

void LED::turn_on() {
  digitalWrite(pin, (on_is_high)? HIGH : LOW);
  mode = IS_ON;
  is_on = true;
}

void LED::blink(int on_time, int off_time, int cycles) {
  digitalWrite(pin, (on_is_high)? HIGH : LOW);
  mode = IS_BLINKING;
  msec_on = on_time;
  msec_off = off_time;
  is_on = true;
  on_off_cntr = msec_on;
  cycle_cntr = cycles;
}

void LED::update_status() {

  if (mode == IS_BLINKING and --on_off_cntr == 0) {
    if (is_on) {
      if (--cycle_cntr == 0) {
        turn_off();
      } else {
        digitalWrite(pin, (on_is_high)? LOW : HIGH);
        on_off_cntr = msec_off;
        is_on = false;
      }
    }
    else {
      digitalWrite(pin, (on_is_high)? HIGH : LOW);
      on_off_cntr = msec_on;
      is_on = true;
    }
  }
}


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
