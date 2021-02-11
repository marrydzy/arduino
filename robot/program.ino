#include </home/marek/Projects/arduino/robot/robot.h>


/* 
Action script_0[] = {
  {0,  PROGRAM,   SET_LOOP,     NONE, NONE, NONE,    2},
  {1,  LED_RED,   LED_ON_SOFT,  3000, NONE, NONE, NONE},
  {1,  STOPPAGE,  PAUSE,        3000, NONE, NONE, NONE},
  {2,  LED_RED,   LED_OFF_SOFT, 3000, NONE, NONE, NONE},
  {2,  STOPPAGE,  PAUSE,        5000, NONE, NONE, NONE},
  {3,  PROGRAM,   LOOP,         NONE, NONE, NONE, NONE},
  {4,  LED_RED,   LED_BLINK_SOFT, 1000, 1000, NONE, -1},
  {4,  STOPPAGE,  PAUSE,       10000, NONE, NONE, NONE},
  {5,  LED_RED,   LED_OFF,      NONE, NONE, NONE, NONE},
  {5,  PROGRAM,   STOP,         NONE, NONE, NONE, NONE},
};

Action script_1[] = { 
  {0,  LED_BLUE,      LED_BLINK,       100,  400, NONE,   -1},
  {0,  ARM,           MOVE_TO,         60,   140,   70, NONE},
  {0,  ROTATION,      MOVE_TO,         163, NONE,  100, NONE},
  {1,  PHOTO_SENSOR,  ACTIVATE,       NONE, NONE, NONE, NONE},
  {1,  ROTATION,      MOVE_TO,          23, NONE,  100, NONE},
  {2,  PHOTO_SENSOR,  STOP,           NONE, NONE, NONE, NONE},
  {2,  ROTATION,      MOVE_TO,          93, NONE,  100, NONE},
  {3,  LED_BLUE,      LED_OFF,        NONE, NONE, NONE, NONE},
  {3,  PROGRAM,       SET_LOOP,       NONE, NONE, NONE,    2},
  {4,  LED_GREEN,     LED_ON,         NONE, NONE, NONE, NONE},
  {4,  GRABBLER,      CYCLE_TO,         67, NONE,  200,   20},
  {5,  LED_GREEN,     LED_OFF,        NONE, NONE, NONE, NONE}, 
  {5,  LED_RED,       LED_BLINK,       300,  150, NONE,   -1},     
  {5,  ARM,           MOVE_TO,         120,  120,   70, NONE},
  {6,  ARM,           CYCLE_TO,        150,  179,   70,    2}, 
  {7,  LED_RED,       LED_OFF,        NONE, NONE, NONE, NONE},
  {7,  ARM,           MOVE_TO,         160,   97,   40, NONE},
  {8,  PROGRAM,       LOOP,           NONE, NONE, NONE, NONE},  
  {9,  LED_RED,       LED_BLINK_SOFT,  100,  100, NONE,   -1},
  {9,  STOPPAGE,      PAUSE,           400, NONE, NONE, NONE},
  {10, LED_RED,       LED_OFF,        NONE, NONE, NONE, NONE},
  {10, PROGRAM,       STOP,           NONE, NONE, NONE, NONE},
};
*/

Action script_0[] = {
  {0,  PROGRAM,   SET_LOOP,         NONE, NONE, NONE,    5},
  {1,  LED_RED,   LED_ON_SOFT,      1000, NONE, NONE, NONE},
  {1,  STOPPAGE,  PAUSE,            1000, NONE, NONE, NONE},
  {2,  LED_RED,   LED_OFF_SOFT,     1000, NONE, NONE, NONE},
  {2,  STOPPAGE,  PAUSE,            1000, NONE, NONE, NONE},
  {3,  PROGRAM,   LOOP,             NONE, NONE, NONE, NONE},
  {4,  LED_RED,   LED_BLINK_SOFT,    200, 1800, NONE,   -1},
  {4,  STOPPAGE,  PAUSE,           20000, NONE, NONE, NONE},
  {5,  LED_RED,   LED_OFF,          NONE, NONE, NONE, NONE},
  {5,  PROGRAM,   JUMP_TO,             2, NONE, NONE, NONE},
};


Action script_1[] = { 
  {0,  LED_BLUE,      LED_BLINK,       100,  400, NONE,   -1},
  {0,  ARM,           MOVE_TO,         60,   140,   70, NONE},
  {0,  ROTATION,      MOVE_TO,         163, NONE,  100, NONE},
  {1,  PHOTO_SENSOR,  ACTIVATE,       NONE, NONE, NONE, NONE},
  {1,  ROTATION,      MOVE_TO,          23, NONE,   50, NONE},
  {2,  PHOTO_SENSOR,  STOP,           NONE, NONE, NONE, NONE},
  {2,  ROTATION,      MOVE_TO,           0, NONE,  100, NONE},
  {3,  LED_BLUE,      LED_OFF,        NONE, NONE, NONE, NONE},
  {3,  LED_GREEN,     LED_ON,         NONE, NONE, NONE, NONE},
  {3,  GRABBLER,      CYCLE_TO,         67, NONE,  200,   20},
  {4,  LED_GREEN,     LED_OFF,        NONE, NONE, NONE, NONE}, 
  {4,  LED_RED,       LED_BLINK,       300,  150, NONE,   -1},     
  {4,  ARM,           MOVE_TO,         160,   97,   40, NONE},
  {4,  ROTATION,      MOVE_TO,          98, NONE,   50, NONE},
  {5,  LED_RED,       LED_OFF,        NONE, NONE, NONE, NONE},
  {5,  PROGRAM,       STOP,           NONE, NONE, NONE, NONE},
};


Action* scripts[] = {
  script_0,
  script_1,
};

void Program::init(int prog_nbr) {
  row = 0;
  step_nbr = 0;
  loop_cntr = 0;
  loop_to_row = 0;
  loop_to_step = 0;
  scenario = scripts[prog_nbr-1];
};

void Program::wait(bool waiting_state) {
  waiting = waiting_state;
}

bool Program::is_waiting() {
  return waiting;
}


Action* Program::get_action() {
  if (scenario[row].step_nbr == step_nbr) {
    if (scenario[row].device == PROGRAM) {
      switch(scenario[row].action_type) {
        case STOP:
          break;
        case SET_LOOP:
          loop_cntr = scenario[row].cycles;
          loop_to_row = ++row;
          loop_to_step = ++step_nbr;
          waiting = true;
          break;
        case LOOP:
          if (--loop_cntr > 0) {
            row = loop_to_row;
            step_nbr = loop_to_step;
          }
          else {
            row++;
            step_nbr++;
          }
          waiting = true;
          break;
        case JUMP_TO:
          init(scenario[row].pos_1);      
          waiting = true;
          break;
        case GET_INPUT:
          Serial.print("Enter number of cycles: ");
          while (Serial.available() == 0) {}
          String odebraneDane = Serial.readStringUntil('\n');
          int cycles = atoi(odebraneDane.c_str());
          Serial.println(cycles);
          script_0[2].cycles = cycles;
          script_0[6].cycles = cycles;
          row++;
          step_nbr++;
          waiting = true;
          break;
      }
    return NULL; 
    }
    else {
      return &scenario[row++];
    }
  }
  else {
    step_nbr++;
    return NULL;
  }
};
