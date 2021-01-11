#include </home/marek/Projects/arduino/robot/program.h>


/* General robot movements program structure:
 *  int* steps[tot_steps]
 *  for each step:
 *  int* = {active_devices, [device_1, data_11[, data_12, ...][, device_2, data_21[, data22, ...]] ... ]}
 */


int scenario[][7] = {
  {1, ROTATION, MOVE_TO,   65, NONE, 100, 0},
  {2, ROTATION, CYCLE_TO, 125, -1, 100,  1},
  {2, GRABBLER, CYCLE_TO,  85, -1, 150,  0},
  {3, ROTATION, MOVE_TO,   93, -1, 100, -1},
  {4, PROGRAM,  STOP,     NAN, -1,   0, -1},
};


void Program::init() {
  row = 0;
  step_nbr = 0;
  l1_cntr = 0;
  l2_cntr = 0;;
};


int* Program::get_action() {
  if (scenario[row][0] == step_nbr) {
    if (scenario[row][1] == PROGRAM) {
      if (scenario[row][2] == STOP) {
        init();
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
