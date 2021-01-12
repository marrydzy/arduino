
#ifndef Program_h
#define Program_h

#define IDLE_STATE -1
#define MOVING      0
#define BOUNCED     1
#define STOPPED     2

#define LEFT        0
#define RIGHT       1

#define LED_BLUE    2
#define LED_GREEN   4
#define LED_RED     3

#define PROGRAM     1
#define ROTATION    2
#define GRABBLER    3
#define ARM         4

#define STOP        1
#define MOVE_TO     2
#define CYCLE_TO    3

#define NONE       -1

void next_step();
void reset_leds();

struct Action {
  int step_nbr;
  int device;
  int action_type;
  int pos_1;
  int pos_2;
  int delta;
  int cycles;
};

class Program {
  int row = 0;
  int step_nbr = 0;
  int l1_cntr = 0;
  int l2_cntr = 0;
  Action* scenario;
public:
  void init(int);
  Action* get_action();
};



#endif
