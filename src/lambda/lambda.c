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

#include "lambda.h"

#define LAMBDA_PI_REPL_PRIORITY 0

sexp ctx __attribute__ (( aligned (4) ));
sexp __current_task __attribute__ (( aligned (4) ));
sexp __next_task __attribute__ (( aligned (4) ));
sexp __task_priority_vector[33] __attribute__ (( aligned (4) ));

void c_entry() { 
  platform_startup();
  init_syscalls();
  pl011_init();
   
  sexp_init();
    
  __ctx = sexp_make_eval_context(NULL, NULL, NULL, 0, 0);
  
  sexp_load_standard_ports(__ctx, NULL, stdin, stdout, stderr, 0);
  

  // Set up list of tasks
  for (int i = 0; i < 31; i++) {
    sexp obj = sexp_cons(__ctx, SEXP_SKIP_TASK, SEXP_NULL);
    sexp_preserve_object(__ctx, obj);
    __task_priority_vector[i] = obj;
  }
      
  // We can now create tasks.  
  // The default system sleeper
  __current_task = sexp_make_task(__ctx, &lambda_sleep, &terminate_current_task, sexp_context_env(__ctx), 31, 256);
  sexp_preserve_object(__ctx, __current_task);
  __task_priority_vector[31] = sexp_cons(__ctx, __current_task, SEXP_NULL);
  sexp_preserve_object(__ctx, __task_priority_vector[31]);
  sexp_start_task(__ctx, __current_task);
  // And a repl
  __current_task = sexp_make_task(__ctx, &repl, &terminate_current_task, sexp_context_env(__ctx), 0, 8192);
  sexp_preserve_object(__ctx, __current_task);
  __task_priority_vector[0] = sexp_cons(__ctx, __current_task, __task_priority_vector[0]);
  sexp_preserve_object(__ctx, __task_priority_vector[0]);
  sexp_start_task(__ctx, __current_task);
  
  __current_task = (sexp)0L;
  __next_task = (sexp)0L;
}


void lambda_sleep(sexp env) {
  sys_sleep();
}

void tick() {
  __next_task = find_next_runnable_task();
}

// Terminate the current task.
void terminate_current_task() {
  unsigned int priority = sexp_task_priority(__current_task);
  sexp queue = __task_priority_vector[priority];
  __task_priority_vector[priority] = sexp_list_destructive_remove(queue, __current_task);
  __current_task = (sexp)0L;
  __next_task = find_next_runnable_task();
    
  yield();
}


static sexp sexp_param_ref (sexp ctx, sexp env, sexp name) {
  sexp res=sexp_env_ref(env, name, SEXP_FALSE);
  return sexp_opcodep(res) ? sexp_parameter_ref(ctx, res) : SEXP_VOID;
}

void repl (sexp env) {
  sexp in, out, err;
  sexp_gc_var3(obj, tmp, res);
  sexp_gc_preserve3(__ctx, obj, tmp, res);
  sexp_context_tracep(__ctx) = 1;
  in  = sexp_param_ref(__ctx, env, sexp_global(__ctx, SEXP_G_CUR_IN_SYMBOL));
  out = sexp_param_ref(__ctx, env, sexp_global(__ctx, SEXP_G_CUR_OUT_SYMBOL));
  err = sexp_param_ref(__ctx, env, sexp_global(__ctx, SEXP_G_CUR_ERR_SYMBOL));
  sexp_port_sourcep(in) = 1;
  while (1) {
    sexp_newline(__ctx, out);
    sexp_write_string(__ctx, "(λπ) ", out);
    sexp_flush(__ctx, out);
    sexp_maybe_block_port(__ctx, in, 1);
    obj = sexp_read(__ctx, in);
    sexp_maybe_unblock_port(__ctx, in);
    
    sexp_display(__ctx, obj, out);
    sexp_newline(__ctx, out);
    sexp_write(__ctx, obj, out);
    sexp_newline(__ctx, out);
    sexp_flush(__ctx, out);
    
    if (obj == SEXP_EOF)
      break;
    if (sexp_exceptionp(obj)) {
      sexp_print_exception(__ctx, obj, err);
    } else {
      tmp = sexp_env_bindings(env);
      sexp_context_top(__ctx) = 0;
      if (!(sexp_idp(obj)||sexp_pairp(obj)))
        obj = sexp_make_lit(__ctx, obj);
      res = sexp_eval(__ctx, obj, env);
      if (sexp_exceptionp(res)) {
        sexp_print_exception(__ctx, res, err);
        sexp_stack_trace(__ctx, err);
      } else {
#if SEXP_USE_WARN_UNDEFS
        sexp_warn_undefs(__ctx, sexp_env_bindings(env), tmp);
#endif
        if (res != SEXP_VOID) {
          sexp_write(__ctx, res, out);
          sexp_write_char(__ctx, '\n', out);
        }
      }
    }
  }
  sexp_gc_release3(__ctx);
}


sexp find_next_runnable_task() {
  for (uint32_t priority = 0; priority < 32; priority++) {
    sexp_gc_var3(cell, head, tail);
    sexp_gc_preserve3(__ctx, cell, head, tail);
    cell = __task_priority_vector[priority];
    head = sexp_cdr(cell);
    tail = cell;
    
    // If head is not SEXP_NULL, then we have a list with more than one task      
    if (sexp_pairp(head)) {
      for (; sexp_cdr(tail) != SEXP_NULL; tail = sexp_cdr(tail)) {
        // do nothing, get the tail of the list
      }
    
      do {
        // graft the cell onto the end of the list
        sexp_cdr(cell) = SEXP_NULL;
        sexp_cdr(tail) = cell;
        tail = cell;
        cell = head;
        head = sexp_cdr(cell);
        
      } while ((sexp_taskp(sexp_car(tail))) && (sexp_task_state(sexp_car(tail)) != SEXP_TASK_RUN));
      __task_priority_vector[priority] = cell;
    }
    
    // When we get to here, sexp_car(tail) is either a runnable task, or SEXP_SKIP_TASK 
    if (sexp_taskp(sexp_car(tail))) {
      // if it's a task, we return it, otherwise drop out and go round again for the next priority
      sexp_gc_release3(__ctx);
      return sexp_car(tail);
    }
    sexp_gc_release3(__ctx);
  }
  // If we get to here, something is catastrophically wrong
  return (sexp)0L;
}
