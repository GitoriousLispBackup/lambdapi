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
scm_obj_t __waiting_tasks = scm_nil;

scm_obj_t make_task(scm_obj_t entry_point, scm_fixnum_t stack_size, scm_fixnum_t priority, scm_fixnum_t state) {
  // Create the object
  scm_task_t object = (scm_task_t)alloc_cells(4);
  HDR(object) = ((uint32_t)state & 0xfffffff0) | scm_hdr_task;
  TASK_PRIORITY(object) = (uint32_t)priority;
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
  
  scm_obj_t pair = make_pair(object, scm_nil);
  
  if (FIXNUM(state) == TASK_RUNNABLE) {
    nconc(&(__priority_queues[FIXNUM(priority)]), pair);
    __runnable_priorities |= (1 << FIXNUM(priority));
  } else {
    nconc(&__waiting_tasks, pair);
  }
  
  return object;
}

void yield() {
  for(;;);
}

void terminate_current_task(void) {
  uint32_t priority = FIXNUM(TASK_PRIORITY(__current_task));
  remove_task_from_queue(__current_task, priority);
  __current_task = 0;
  yield();
}

// Find next runnable task.
scm_obj_t find_next_runnable_task() {
  // Find first set bit in the list of runnable priorities, to find which queue
  // we want to take.
  uint32_t priority = first_set_bit(__runnable_priorities) - 1;
  
  if (priority >= 0) {
    scm_obj_t queue = __priority_queues[priority];
    // Get the task
    scm_obj_t task = CAR(queue);
    // Remove it from the queue
    __priority_queues[priority] = CDR(queue);
    // And graft it back on the end again
    nconc(&(__priority_queues[priority]), make_pair(task, scm_nil));
    return task;
  }
  return scm_nil;
}

void remove_task_from_queue(scm_obj_t task, uint32_t priority) {
  scm_obj_t * queue = &(__priority_queues[priority]);
  if (CAR(*queue) == task) {
    *queue = CDR(*queue);
    if (*queue == scm_nil) {
      __runnable_priorities ^= (1 << priority);
    }
  } else {
    scm_obj_t q = *queue;
    while (CDR(q) != scm_nil && CADR(q) != task) {
      q = CDR(q);
    }
    if (CDR(q) != scm_nil) {
      SET_CDR(q, CDDR(q));
    }
  }
}

