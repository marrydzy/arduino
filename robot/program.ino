#include </home/marek/Projects/arduino/robot/program.h>


/* General robot movements program structure:
 *  int* steps[tot_steps]
 *  for each step:
 *  int* = {active_devices, [device_1, data_11[, data_12, ...][, device_2, data_21[, data22, ...]] ... ]}
 */


int scenario[][7] = {
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


void Program::init() {
  row = 0;
  step_nbr = 0;
  l1_cntr = 0;
  l2_cntr = 0;

};


int* Program::get_action() {
  if (scenario[row][0] == step_nbr) {
    if (scenario[row][1] == PROGRAM) {
      if (scenario[row][2] == STOP) {
        reset_leds();
        return(NULL);
      }
    }
    else {
      return(scenario[row++]);
    }
  }
  else {
    step_nbr++;
    return(NULL);
  }
};
