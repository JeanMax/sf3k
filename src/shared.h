#ifndef _SHARED_H
#define _SHARED_H

//TODO: move
typedef enum state e_state;
enum state {
    WAIT = 0,
    COOL,
    HEAT,
};


extern volatile float shared__current_temp;
extern volatile int shared__goal_temp;
extern volatile e_state shared__state;


#endif
