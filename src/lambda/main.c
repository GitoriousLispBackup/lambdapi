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

extern scm_obj_t __next_task;
extern scm_obj_t __current_task;

mutex_t serial_mutex = 0L;

void test_1() {
  mutex_acquire(&serial_mutex);
  pl011_write('(');
	pl011_write(0xcebb); // UTF-8 λ
  yield();
	pl011_write(0xcf80); // UTF-8 π
	pl011_write(')');
	pl011_write(' ');
  mutex_release(&serial_mutex);
}
void test_2() {
  mutex_acquire(&serial_mutex);
  pl011_write('H');
	pl011_write('e'); // UTF-8 λ
  yield();
	pl011_write('l'); // UTF-8 π
	pl011_write('l');
	pl011_write('0');
	pl011_write(' ');
	pl011_write('w');
	pl011_write('0');
	pl011_write('r');
	pl011_write('l');
	pl011_write('d');
	pl011_write('!');
	pl011_write(' ');
  mutex_release(&serial_mutex);
}
// Startup code, to be done on system startup.
// Executes in SVC mode with interrupts off
void c_entry(void) {
  platform_startup();
  init_syscalls();
  pl011_init();
  
  // Create idle loop as first task.
  make_task((scm_obj_t)&sys_sleep, make_fixnum(256), make_fixnum(31), make_fixnum(TASK_RUNNABLE));
  make_task((scm_obj_t)&test_1, make_fixnum(256), make_fixnum(0), make_fixnum(TASK_RUNNABLE));
  make_task((scm_obj_t)&test_2, make_fixnum(256), make_fixnum(0), make_fixnum(TASK_RUNNABLE));
  
}



