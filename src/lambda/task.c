/*        Copyright (c) 20011, Simon Stapleton (simon.stapleton@gmail.com)        */
/*                                                                                */
/*                              All rights reserved.                              */
/*                                                                                */
/* Redistribution  and  use   in  source  and  binary  forms,   with  or  without */
/* modification, are permitted provided that the following conditions are met:    */
/*                                                                                */
/* Redistributions of  source code must  retain the above copyright  notice, this */
/* list of conditions and the following disclaimer.                               */
/*                                                                                */
/* Redistributions in binary form must reproduce the above copyright notice, this */
/* list of conditions and the following disclaimer in the documentation and/or    */
/* other materials provided with the distribution.                                */
/*                                                                                */
/* Neither the name of  the developer nor the names of  other contributors may be */
/* used  to  endorse or  promote  products  derived  from this  software  without */
/* specific prior written permission.                                             */
/*                                                                                */
/* THIS SOFTWARE  IS PROVIDED BY THE  COPYRIGHT HOLDERS AND CONTRIBUTORS  "AS IS" */
/* AND ANY  EXPRESS OR  IMPLIED WARRANTIES,  INCLUDING, BUT  NOT LIMITED  TO, THE */
/* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE */
/* DISCLAIMED. IN NO  EVENT SHALL THE COPYRIGHT HOLDER OR  CONTRIBUTORS BE LIABLE */
/* FOR  ANY DIRECT,  INDIRECT, INCIDENTAL,  SPECIAL, EXEMPLARY,  OR CONSEQUENTIAL */
/* DAMAGES (INCLUDING,  BUT NOT  LIMITED TO, PROCUREMENT  OF SUBSTITUTE  GOODS OR */
/* SERVICES; LOSS  OF USE,  DATA, OR PROFITS;  OR BUSINESS  INTERRUPTION) HOWEVER */
/* CAUSED AND ON ANY THEORY OF  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING  IN ANY WAY OUT OF THE USE */
/* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.           */

#include "task.h"

scm_obj_t __current_task = 0L;
scm_obj_t __next_task = 0L;

uint32_t __runnable_priorities = 0;
scm_obj_t __priority_queues[32] = {scm_nil, scm_nil, scm_nil, scm_nil,
                                   scm_nil, scm_nil, scm_nil, scm_nil,
                                   scm_nil, scm_nil, scm_nil, scm_nil,
                                   scm_nil, scm_nil, scm_nil, scm_nil,
                                   scm_nil, scm_nil, scm_nil, scm_nil,
                                   scm_nil, scm_nil, scm_nil, scm_nil,
                                   scm_nil, scm_nil, scm_nil, scm_nil,
                                   scm_nil, scm_nil, scm_nil, scm_nil};

// For internal use only
void terminate_current_task(void);


scm_obj_t make_task(scm_obj_t entry_point, scm_fixnum_t stack_size, scm_fixnum_t priority, scm_fixnum_t state) {
  // Create the object
  scm_task_t object = alloc_cells(4);
  HDR(object) = ((uint32_t)state & 0xfffffff0) | scm_hdr_task;
  TASK_PRIORITY(object) = priority;
  TASK_STACK(object) = alloc_cells(FIXNUM(stack_size));
  
  // And prefill the stack
  // Specific format to work with our particular context switcher
  uint32_t * sp = TASK_STACK(object) + FIXNUM(stack_size);
  *(--sp) = 0x00000010;             // CPSR (user mode with interrupts enabled)
  *(--sp) = (uint32_t)entry_point;  // 'return' address (i.e. where we come in)
  *(--sp) = 0x0c0c0c0c;             // r12
  *(--sp) = 0x0b0b0b0b;             // r11
  *(--sp) = 0x0a0a0a0a;             // r10
  *(--sp) = 0x09090909;             // r9
  *(--sp) = 0x08080808;             // r8
  *(--sp) = 0x07070707;             // r7
  *(--sp) = 0x06060606;             // r6
  *(--sp) = 0x05050505;             // r5
  *(--sp) = 0x04040404;             // r4
  *(--sp) = 0x03030303;             // r3
  *(--sp) = 0x02020202;             // r2
  *(--sp) = 0x01010101;             // r1
  *(--sp) = 0x00000000;             // r0

  if ((uint32_t)sp & 0x07) {
    *(--sp) = 0xdeadc0de;           // Stack filler
    *(--sp) = (uint32_t)terminate_current_task;  // lr, where we go on exit
    *(--sp) = 0x00000004;           // Stack Adjust
  } else {
    *(--sp) = (uint32_t)terminate_current_task;  // lr, where we go on exit
    *(--sp) = 0x00000000;           // Stack Adjust
  }
  
  TASK_SP(object) = sp;
    
  add_task_to_queue(object, &(__priority_queues[FIXNUM(priority)]));
  __runnable_priorities |= (1 << FIXNUM(priority));
  
  return object;
}

void yield() {
//  syscall0_nr(SYS_YIELD);
}

void terminate_current_task(void) {
  uint32_t priority = FIXNUM(TASK_PRIORITY(__current_task));
  scm_obj_t * queue = &(__priority_queues[priority]);
  remove_task_from_queue(__current_task, queue);
  __current_task = 0;
  yield();
}

// Find next runnable task.
scm_obj_t find_next_runnable_task() {
  // Find first set bit in the list of runnable priorities, to find which queue
  // we want to take.
  uint32_t priority;
  scm_obj_t task = scm_nil;
  // Go through the set priorities, from 0 to 31
  while((priority = first_set_bit(__runnable_priorities) - 1) >= 0) {
    scm_obj_t tail = __priority_queues[priority];
    // Go through the queue for that priority
    while (tail != scm_nil) {
      // If we find a runnable task, we're good to go
      if (FIXNUM(HDR(CAR(tail))) == TASK_RUNNABLE) {
        return CAR(tail);
      }
      tail = CDR(tail);
    }
    // If we get here, the priority only has waiting tasks, so reset its runnable status 
    __runnable_priorities ^= (1 << priority);
  }
  // If we get here, we have no runnable tasks at all.  Bad coder!
  return scm_nil;
}

scm_obj_t remove_task_from_queue(scm_obj_t task, scm_obj_t * queue) {
  if (CAR(*queue) == task) {
    *queue = CDR(*queue);
  } else {
    scm_obj_t q = *queue;
    while (CDR(q) != scm_nil && CADR(q) != task) {
      q = CDR(q);
    }
    if (CDR(q) != scm_nil) {
      SET_CDR(q, CDDR(q));
    }
  }
  return task;
}

scm_obj_t add_task_to_queue(scm_obj_t task, scm_obj_t * queue) {
  nconc(queue, make_pair(task, scm_nil));
  return task;
}

scm_obj_t set_task_state(scm_obj_t task, scm_obj_t state) {
  HDR(task) = ((uint32_t)state & 0xfffffff0) | scm_hdr_task;
  return task;
}

scm_obj_t set_task_priority(scm_obj_t task, scm_obj_t priority) {
  if (TASK_PRIORITY(task) != priority) {
    remove_task_from_queue(task, &(__priority_queues[FIXNUM(TASK_PRIORITY(task))]));
    TASK_PRIORITY(task) = priority;
    add_task_to_queue(task, &(__priority_queues[FIXNUM(TASK_PRIORITY(task))]));
    __runnable_priorities |= (1 << FIXNUM(priority)); 
  }
  return task;
}

// On every timer tick (1ms)
// Executes in System mode
void tick (void) {
  __next_task = find_next_runnable_task();
}

