/*	  Copyright (c) 20011, Simon Stapleton (simon.stapleton@gmail.com)	  */
/*										  */
/*				All rights reserved.				  */
/*										  */
/* Redistribution  and	use   in  source  and  binary  forms,	with  or  without */
/* modification, are permitted provided that the following conditions are met:	  */
/*										  */
/* Redistributions of  source code must	 retain the above copyright  notice, this */
/* list of conditions and the following disclaimer.				  */
/*										  */
/* Redistributions in binary form must reproduce the above copyright notice, this */
/* list of conditions and the following disclaimer in the documentation and/or	  */
/* other materials provided with the distribution.				  */
/*										  */
/* Neither the name of	the developer nor the names of	other contributors may be */
/* used	 to  endorse or	 promote  products  derived  from this	software  without */
/* specific prior written permission.						  */
/*										  */
/* THIS SOFTWARE  IS PROVIDED BY THE  COPYRIGHT HOLDERS AND CONTRIBUTORS  "AS IS" */
/* AND ANY  EXPRESS OR	IMPLIED WARRANTIES,  INCLUDING, BUT  NOT LIMITED  TO, THE */
/* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE */
/* DISCLAIMED. IN NO  EVENT SHALL THE COPYRIGHT HOLDER OR  CONTRIBUTORS BE LIABLE */
/* FOR	ANY DIRECT,  INDIRECT, INCIDENTAL,  SPECIAL, EXEMPLARY,	 OR CONSEQUENTIAL */
/* DAMAGES (INCLUDING,	BUT NOT	 LIMITED TO, PROCUREMENT  OF SUBSTITUTE	 GOODS OR */
/* SERVICES; LOSS  OF USE,  DATA, OR PROFITS;  OR BUSINESS  INTERRUPTION) HOWEVER */
/* CAUSED AND ON ANY THEORY OF	LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING	IN ANY WAY OUT OF THE USE */
/* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.		  */

.include "sysequates.inc"



/********************************************************/
/* Identify and acknowledge interrupt			*/
/* In 	: nada 						*/
/* Out	: r0 - address of interrupt handler or null 	*/
/* Flags: zero if no handler				*/
/* Clob	: r1-r6						*/
/********************************************************/
FUNC	identify_and_clear_irq
	ldr	r4, =.Lirq_base
	ldr	r4, [r4]
	
	/* read the vector address to indicate we're handling the interrupt */
	ldr	r0, [r4, #IRQ_HANDLER]
	
	/* which IRQs are asserted? */
	ldr	r0, [r4, #IRQ_STATUS]
	ldr	r5, =__irq_handlers

	clz	r6, r0				/* which IRQ was asserted? */
	rsb	r6, r6, #31			/* in terms of bit number */

	mov	r1, #1				/* make a mask */
	bic	r0, r0, r1, lsl r6		/* clear flag */
	
	str	r0, [r4, #IRQ_ACK]		/* Now acknowledge the interrupt */
	str	r0, [r4, #IRQ_SOFTCLEAR]	/* and make sure we clear software irqs too */
		
	ldr	r0, [r5, r6, lsl #2]		/* load handler address */
.Lret:	bx	lr				/* exit */
	
.Lirq_base:
	.word	IRQ_BASE
	

.bss
.global __irq_handlers
__irq_handlers:	.skip	32 * 4
