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

#include "syscalls.h"

extern uint32_t __syscall_table;

extern void * switch_context_do;

// Define a syscall.
// Although this is a void function, it can return up to 4 values by cheating
// and setting reg[0]-reg[3]
// It's opaque, but it works.  See __alloc_cells() in lisp_memory.c for an
// example 
void define_syscall(syscall_linkage_t linkage, uint32_t number, void * func) {
	(&__syscall_table)[number & MAX_SYSCALLS] = (uint32_t)func | linkage;
}

void init_syscalls() {
  define_syscall(NAKED, SYS_YIELD, &switch_context_do);
}

void yield() {
  syscall0_nr(SYS_YIELD);
}
