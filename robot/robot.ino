#include <Servo.h> //Biblioteka odpowiedzialna za serwa
#include </home/marek/Projects/arduino/robot/robot.h>


bool switch_pressed = false;

Servo servo_rotation;
Servo servo_grabbler;
Servo servo_left;
Servo servo_right;
   
Motion rotation;
Motion grabbler;
Arm_Motion arm;

Switch switch_1;

LED led_red;
LED led_green;
LED led_blue;


Program program;

void setup() 
{ 
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  /*
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_BLUE, HIGH);
  */
  
  rotation.init(93, &servo_rotation, 9);
  grabbler.init(65, &servo_grabbler, 6);
  arm.init(160, &servo_left, 10, 97, &servo_right, 11);
  switch_1.init(7);
  led_red.init(3, false);
  led_green.init(4, false);
  led_blue.init(2, false);
  // Serial.begin(9600);
} 

 
void loop() { 
  if (switch_1.pressed()) {
    program.init(2);
    next_step();
  }

  static bool waiting_to_stop = false;
  int rotation_status = rotation.update_position();
  int grabbler_status = grabbler.update_position();
  int arm_status = arm.update_position();
  
  led_red.update_status();
  led_green.update_status();
  led_blue.update_status();

  if (rotation_status == STOPPED or grabbler_status == STOPPED or arm_status == STOPPED) {
    rotation.complete_cycle_and_stop();
    grabbler.complete_cycle_and_stop();
    arm.complete_cycle_and_stop();
    waiting_to_stop = true;
  }

  if (waiting_to_stop and rotation_status == IDLE_STATE and grabbler_status == IDLE_STATE and arm_status == IDLE_STATE) {
    next_step();
    waiting_to_stop = false;
  }
  
  delay(1);
}


void next_step() {
  Action* action = program.get_action();
  LED* led = NULL;
  
  while(action) {
    int   device = action->device;
    int   action_type = action->action_type;
    int   pos_1 = action->pos_1;
    int   pos_2 = action->pos_2;
    float velocity = float(action->delta)/1000.0;
    int   cycles = action->cycles;

    switch(device) {
      
      case ROTATION:
        if (action_type == MOVE_TO) {
          rotation.move_to(pos_1, velocity);
        }
        else {
          rotation.cycle_to(pos_1, velocity, cycles);
        }
        break;
      case GRABBLER:
        if (action_type == MOVE_TO) {
          grabbler.move_to(pos_1, velocity);
        }
        else {
          grabbler.cycle_to(pos_1, velocity, cycles);
        }
        break;
      case ARM:
        if (action_type == MOVE_TO) {
          arm.move_to(pos_1, pos_2, velocity);
        }
        else {
          arm.cycle_to(pos_1, pos_2, velocity, cycles);
        }
        break;
      case LED_RED:
        led = &led_red;
        break;
      case LED_GREEN:
        led = &led_green;
        break;
      case LED_BLUE:
        led = &led_blue;
        break;
      default:
        break;
    }
 
    if (led) {
      switch(action_type) {
        case LED_OFF:
          led->turn_off();
          break;
        case LED_ON:
          led->turn_on();
          break;
        case LED_BLINK:
          led->blink(pos_1, pos_2, cycles);
          break;
      }
      led = NULL;
    }    
    action = program.get_action();
  }
}
