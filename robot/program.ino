#include </home/marek/Projects/arduino/robot/robot.h>


/* General robot movements program structure:
 *  int* steps[tot_steps]
 *  for each step:
 *  int* = {active_devices, [device_1, data_11[, data_12, ...][, device_2, data_21[, data22, ...]] ... ]}
 */


Action script_0[] = {
  {10, PROGRAM,  STOP,    NONE, NONE, NONE, NONE},
};

Action script_1[] = {
  {0,  ROTATION, MOVE_TO,   45, NONE,  100, NONE},
  {1,  ROTATION, MOVE_TO,  145, NONE,  100, NONE},
  {2,  ROTATION, MOVE_TO,   93, NONE,  100, NONE},
  {2,  GRABBLER, MOVE_TO,   85, NONE,  100, NONE},
  {3,  GRABBLER, MOVE_TO,   65, NONE,  100, NONE},
  {4,  ARM,      CYCLE_TO, 179,   55,   50,    2},
  {5,  ARM,      CYCLE_TO, 179,  150,   50,    2},
  {6,  ARM,      CYCLE_TO, 150,  179,   50,    2},
  {7,  ARM,      CYCLE_TO,  55,  179,   50,    2},
  {8,  ARM,      CYCLE_TO,  55,  120,   50,    2},
  {9,  ARM,      CYCLE_TO, 120,   55,   50,    2},
  {10, PROGRAM,  STOP,    NONE, NONE, NONE, NONE},
};

Action script_2[] = {
  {0,  ARM,       MOVE_TO,    179,   55,   50, NONE},
  {0,  LED_RED,   LED_BLINK,  500,  500, NONE,    0},
  {1,  ARM,       MOVE_TO,    179,  150,   50, NONE},
  {2,  ARM,       MOVE_TO,    150,  179,   50, NONE},
  {3,  ARM,       MOVE_TO,     55,  179,   50, NONE},
  {3,  LED_RED,   LED_OFF,   NONE, NONE, NONE, NONE},
  {3,  LED_BLUE,  LED_BLINK,  300,  300, NONE,    0},
  {4,  ARM,       MOVE_TO,     55,  120,   50, NONE},
  {5,  ARM,       MOVE_TO,    120,   55,   50, NONE},
  {6,  ARM,       MOVE_TO,    179,   55,   50, NONE},
  {7,  ARM,       MOVE_TO,    160,   97,   50, NONE},
  {7,  LED_BLUE,  LED_OFF,   NONE, NONE, NONE, NONE},
  {7,  LED_GREEN, LED_BLINK,  300,  300, NONE,    0},
  {8,  ARM,       MOVE_TO,    160,  170,   50, NONE},
  {9,  ARM,       MOVE_TO,    160,   55,   50, NONE},
  {10, ARM,       MOVE_TO,    160,   97,   50, NONE},
  {11, LED_GREEN, LED_OFF,   NONE, NONE, NONE, NONE}, 
  {11, PROGRAM,   STOP,      NONE, NONE, NONE, NONE},
};


void Program::init(int prog_nbr) {
  row = 0;
  step_nbr = 0;
  l1_cntr = 0;
  l2_cntr = 0;
  switch(prog_nbr) {
    case 1:
      scenario = script_1;
      break;
    case 2:
      scenario = script_2;
      break;
    default:
      scenario = script_0;
      break;
  }
};


Action* Program::get_action() {
  if (scenario[row].step_nbr == step_nbr) {
    if (scenario[row].device == PROGRAM) {
      if (scenario[row].action_type == STOP) {
        // reset_leds();
        return(NULL);
      }
    }
    else {
      return(&scenario[row++]);
    }
  }
  else {
    step_nbr++;
    return(NULL);
  }
};
