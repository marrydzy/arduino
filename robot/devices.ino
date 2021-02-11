#include <Servo.h> //Biblioteka odpowiedzialna za serwa
#include </home/marek/Projects/arduino/robot/robot.h>


void PhotoSensor::init(int pin_nbr, Motion* srv) {
  pin = pin_nbr;
  servo = srv;
  servo_pos = 0;
  max_level = 0;
  active = false;
}

void PhotoSensor::find_max_pos() {
  servo_pos = 0;
  max_level = 0;
  active = true;
}

void PhotoSensor::measure() {
  if (active) {
    int level = analogRead(pin);
    if (level > max_level) {
      max_level = level;
      servo_pos = servo->get_position();
    }
  }
}

uint8_t PhotoSensor::get_max_pos() {
  active = false;
  return servo_pos;
}


void Intermission::set_delay(unsigned long del) {
  delay_ms = del;
  start_time = millis();
}

int Intermission::check_elapsed_time() {
  if (delay_ms == 0) {
    return IDLE_STATE;
  }
  if (millis() - start_time >= delay_ms) {
    delay_ms = 0;
    return STOPPED;
  }
  else {
    return MOVING;
  }
}


void Switch::init(uint8_t pin_nbr) {
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
      return true;
    }
  }
  return false;
}


void LED::init(uint8_t pin_nbr, bool high_on) {
  pin = pin_nbr;
  pinMode(pin_nbr, OUTPUT);
  on_is_high = high_on;
  cycle_cntr = 0;
  msec_on = 0UL;
  msec_off = 0UL;
  phase_time = 0UL;
  start_time = 0UL;
  turn_off(false, NONE); 
}

void LED::turn_on(bool softly, int switching_time) {
  soft_switching = softly;
  if (soft_switching) {
    mode = IS_SWITCHING_ON;
    start_time = millis();
    soft_step = 255.0/float(switching_time);
  }
  else {
    digitalWrite(pin, (on_is_high)? HIGH : LOW);
    mode = IS_ON;
  }
}

void LED::turn_off(bool softly, int switching_time) {
  cycle_cntr = 0;
  soft_switching = softly;
  if (soft_switching) {
    mode = IS_SWITCHING_OFF;
    start_time = millis();
    soft_step = 255.0/float(switching_time);
  }
  else {
    digitalWrite(pin, (on_is_high)? LOW : HIGH);
    mode = IS_OFF;
  }
}

void LED::blink(bool softly, unsigned long on_time, unsigned long off_time, int cycles) {
  soft_switching = softly;
  msec_on = on_time;
  msec_off = off_time;
  cycle_cntr = cycles;
  phase_time = msec_on;
  start_time = millis();
  turn_on(softly, msec_on);
}

void LED::update_status() {
  if (mode == IS_SWITCHING_ON) {
    int soft_level = round(float(millis()-start_time) * soft_step);
    if (on_is_high) {
      soft_level = min(soft_level, 255);
      if (soft_level == 255) {
        mode = IS_ON;
      }
    }
    else {
      soft_level = max(255-soft_level, 0);
      if (soft_level == 0) {
        mode = IS_ON;
      }
    }
    analogWrite(pin, soft_level);
  }
  else if (mode == IS_SWITCHING_OFF) {
    int soft_level = 255 - round(float(millis()-start_time) * soft_step);
    if (on_is_high) {
      soft_level = max(soft_level, 0);
      if (soft_level == 0) {
        mode = IS_OFF;
      }
    } 
    else {
      soft_level = min(255-soft_level, 255);
      if (soft_level == 255) {
        mode = IS_OFF;
      }
    }
    analogWrite(pin, soft_level);
  }

  if (cycle_cntr != 0 and millis() - start_time >= phase_time ) {
    if (mode == IS_ON) {
      phase_time = msec_off;
      start_time = millis();
      int tmp = cycle_cntr;
      turn_off(soft_switching, msec_off);
      cycle_cntr = tmp;
    }
    else {
      if (cycle_cntr > 0 and --cycle_cntr == 0) {
        turn_off(false, NONE);
      } 
      else {
        phase_time = msec_on;
        start_time = millis();
        turn_on(soft_switching, msec_on);
      }
    }
  } 
}


void Motion::synchronize() {
  servo_position = (abs(upper_bound - servo_position) < abs(servo_position - lower_bound))? upper_bound : lower_bound;
  location = (float)servo_position;
  servo->write(servo_position);
}

int Motion::get_position() {
  return servo_position;
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
void Motion::elementary_move(uint8_t bounce_pos, uint8_t stop_pos, float velocity) {
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
  return ret_value;
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
  return master_status;
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
