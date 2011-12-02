#include "lambda.h"

extern scm_obj_t __next_task;
extern scm_obj_t __current_task;

void idle(void);

// Startup code, to be done on system startup.
// Executes in SVC mode
void c_entry(void) {
  platform_startup();
  // Create idle loop as first task.
  __current_task = make_task((scm_obj_t)&idle, make_fixnum(256), make_fixnum(255), make_fixnum(0));
  __next_task = __current_task;
  
  INTERRUPTS_ON_PRIV;
  
  for (;;);
}

// On every timer tick (1ms)
// Executes in System mode
void tick (void) {
  
}

void go_low_power () { 
}

// Idle loop
// Executes in User mode
void idle() {
  for (;;) {
    go_low_power();
  }
}

