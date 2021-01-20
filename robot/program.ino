#include </home/marek/Projects/arduino/robot/robot.h>


Action script_0[] = {
  {0,  PROGRAM,   SET_LOOP,     NONE, NONE, NONE,   10},
  {1,  LED_RED,   LED_ON_SOFT,  3000, NONE, NONE, NONE},
  {1,  STOPPAGE,  PAUSE,        3000, NONE, NONE, NONE},
  {2,  LED_RED,   LED_OFF_SOFT, 3000, NONE, NONE, NONE},
  {2,  STOPPAGE,  PAUSE,        3000, NONE, NONE, NONE},
  {3,  PROGRAM,   LOOP,         NONE, NONE, NONE, NONE},
  {4,  PROGRAM,   STOP,         NONE, NONE, NONE, NONE},
};

Action script_1[] = {
  {0,  LED_RED,   LED_BLINK_SOFT, 2000, 2000, NONE,   10},
  {0,  STOPPAGE,  PAUSE,         28000, NONE, NONE, NONE},
  {4,  PROGRAM,   STOP,           NONE, NONE, NONE, NONE},
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
  return(waiting);
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
    return(NULL); 
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
