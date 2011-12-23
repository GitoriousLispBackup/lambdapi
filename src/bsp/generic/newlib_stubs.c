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

#include <sys/stat.h>
#include <errno.h>
#include "pl011.h"

// Allocate or free memory
caddr_t _sbrk(int incr) {
  extern char * __heap_top;
  extern char * __memtop;
  extern char * __heap_start;
  char * prev_heap_top = __heap_top;
  
  if (__heap_top + incr > __memtop) {
    return (caddr_t)0;
  }
  
  if (__heap_top + incr < __heap_start) {
    return (caddr_t)0;
  }
  
  __heap_top += incr;
  return (caddr_t) prev_heap_top;
}

int _close(int file) { return -1; }

int _fstat(int file, struct stat *st) {
 st->st_mode = S_IFCHR;
 return 0;
}
int _stat(const char *file, struct stat *st) {
 st->st_mode = S_IFCHR;
 return 0;
}

void _exit(int arg) {
  while (1);
}

int _getpid() {
      return 1;
    }
int _kill(int pid, int sig){
      errno=EINVAL;
      return(-1);
    }

int _isatty(int file) { return 1; }

int _lseek(int file, int ptr, int dir) { return 0; }

int _open(const char *name, int flags, int mode) { return -1; }

int _read(int file, char *ptr, int len) {
  return pl011_gets(ptr, len);
}

int _write(int file, char *ptr, int len) {
  pl011_puts(ptr, len);
 // int todo;
 // 
 // for (todo = 0; todo < len; todo++) {
 //  UART_DR(UART0_ADDR) = *ptr++;
 // }
  return len;
}