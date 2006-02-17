/******************************************************************************
LICENSE NOTICE
--------------

IT IS  A BREACH OF  THE LICENSE AGREEMENT  TO REMOVE THIS  NOTICE FROM
THIS  FILE  OR SOFTWARE  OR  ANY MODIFIED  VERSIONS  OF  THIS FILE  OR
SOFTWARE.

Copyright notices/Licensor(s) Identification
--------------------------------------------
Each of  the entity(ies) whose name properly  appear immediately below
in connection with a copyright notice is a Licensor(s) under the terms
that follow.

Copyright  1998  New  York  University.  All rights  reserved  by  the
foregoing, respectively.

Copyright 2001 Center for  Research on Embedded Systems and Technology
at the  Georgia Institute  of Technology. All  rights reserved  by the
foregoing, respectively.

License agreement
-----------------

The  code contained  in this  file  including both  binary and  source
(hereafter,  Software)  is subject  to  copyright  by Licensor(s)  and
ownership remains with Licensor(s).

Licensor(s)  grants you  (hereafter, Licensee)  a license  to  use the
Software for  academic, research and internal  business purposes only,
without  a  fee.  "Internal  business  use"  means  that Licensee  may
install, use and execute the Software for the purpose of designing and
evaluating products.   Licensee may also disclose  results obtained by
executing  the  Software,  as  well as  algorithms  embodied  therein.
Licensee may  distribute the Software  to third parties  provided that
the copyright notice and this statement appears on all copies and that
no  charge  is  associated  with  such copies.   No  patent  or  other
intellectual property license is granted or implied by this Agreement,
and this  Agreement does not  license any acts except  those expressly
recited.

Licensee may  make derivative works,  which shall also be  governed by
the  terms of  this  License Agreement.  If  Licensee distributes  any
derivative work based  on or derived from the  Software, then Licensee
will abide by the following terms.  Both Licensee and Licensor(s) will
be  considered joint  owners of  such derivative  work  and considered
Licensor(s) for  the purpose of distribution of  such derivative work.
Licensee shall  not modify this  agreement except that  Licensee shall
clearly  indicate  that  this  is  a  derivative  work  by  adding  an
additional copyright notice in  the form "Copyright <year> <Owner>" to
other copyright notices above, before the line "All rights reserved by
the foregoing, respectively".   A party who is not  an original author
of such derivative works within  the meaning of US Copyright Law shall
not modify or add his name to the copyright notices above.

Any Licensee  wishing to  make commercial use  of the  Software should
contact each and every Licensor(s) to negotiate an appropriate license
for  such  commercial  use;  permission  of all  Licensor(s)  will  be
required for such a  license.  Commercial use includes (1) integration
of all or part  of the source code into a product  for sale or license
by or on  behalf of Licensee to third parties,  or (2) distribution of
the Software  to third  parties that need  it to utilize  a commercial
product sold or licensed by or on behalf of Licensee.

LICENSOR (S)  MAKES NO REPRESENTATIONS  ABOUT THE SUITABILITY  OF THIS
SOFTWARE FOR ANY  PURPOSE.  IT IS PROVIDED "AS  IS" WITHOUT EXPRESS OR
IMPLIED WARRANTY.   LICENSOR (S) SHALL  NOT BE LIABLE FOR  ANY DAMAGES
SUFFERED BY THE USERS OF THIS SOFTWARE.

IN NO EVENT UNLESS REQUIRED BY  APPLICABLE LAW OR AGREED TO IN WRITING
WILL ANY  COPYRIGHT HOLDER, OR ANY  OTHER PARTY WHO  MAY MODIFY AND/OR
REDISTRIBUTE THE  PROGRAM AS PERMITTED  ABOVE, BE LIABLE  FOR DAMAGES,
INCLUDING  ANY GENERAL, SPECIAL,  INCIDENTAL OR  CONSEQUENTIAL DAMAGES
ARISING OUT OF THE USE OR  INABILITY TO USE THE PROGRAM (INCLUDING BUT
NOT  LIMITED TO  LOSS OF  DATA OR  DATA BEING  RENDERED  INACCURATE OR
LOSSES SUSTAINED BY  YOU OR THIRD PARTIES OR A  FAILURE OF THE PROGRAM
TO  OPERATE WITH ANY  OTHER PROGRAMS),  EVEN IF  SUCH HOLDER  OR OTHER
PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

By using  or copying  this Software, Licensee  agrees to abide  by the
copyright law and all other applicable laws of the U.S., and the terms
of  this license  agreement. Any  individual Licensor  shall  have the
right  to terminate this  license immediately  by written  notice upon
Licensee's   breach   of,  or   non-compliance   with,   any  of   its
terms.  Licensee may  be held  legally responsible  for  any copyright
infringement  that is caused  or encouraged  by Licensee's  failure to
abide     by    the     terms    of     this     license    agreement.
******************************************************************************/

/******************************************************************************
 * File:    PD_kernel.c
 * Authors: Amit Nene, Igor Pechtchanski, Rodric M. Rabbah
 *
 * Description: simulator kernel
 ******************************************************************************/


#include "PD_kernel.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <setjmp.h>
#include "PD_error.h"
#include "PD_parms.h"
#include "PD_stats.h"
#include "PD_trace.h"
#include "PD_reg.h"
#include "PD_queue.h"
#include "PD_signals.h"
#include "PD_sl.h"
#ifdef __PD_DINERO_
#include "PD_dinero.h"
#endif /* __PD_DINERO_ */

#ifdef __PD_DINERO_
    int *mem_layout=NULL;
    int mem_size=0,mem_used=0;
    int mem_addr;
    int old_pc=-1;
    bool jump;

#endif /* __PD_DINERO_ */

/* setjmp/longjmp buffer for recovering from an exception */
extern jmp_buf __PD_jump_buffer;


/* simulator clock and overflow counter */
u64bitint __PD_global_clock = 0;
u64bitint __PD_global_clock_overflow = 0;


/* set when RTS is encounteres and terminates simulation of current function */
bool __PD_done = __PD_false;


#ifdef __PD_FULL_SPECULATION_SUPPORT_
/* check to see  if any of the speculative tags  of the source registers
 * are true, and if so, returns true, otherwise returns false
 */
#define __PD_has_speculated_sources(op) \
  ((LAT(OP_SRC0(op)) && REG(OP_SRC0(op)).nat) | \
   (LAT(OP_SRC1(op)) && REG(OP_SRC1(op)).nat) | \
   (LAT(OP_SRC2(op)) && REG(OP_SRC2(op)).nat) | \
   (LAT(OP_SRC3(op)) && REG(OP_SRC3(op)).nat))

/* propagate the exception raised to destination registers */
inline void __PD_set_op_speculative_tag(__PD_OP* op, bool val)
{
  bool bit = val;

  if ((OP_FLAGS(op) & __PD_BROPMASK) == __PD_BROPMASK) return;

  /* is it worth checking if the register tag is already set to val? */
  if (LAT(OP_DEST0(op))) {
    __PD_delay(&bit, &REG(OP_DEST0(op)).nat, LAT(OP_DEST0(op)));
  }
  if (LAT(OP_DEST1(op))) {
    __PD_delay(&bit, &REG(OP_DEST1(op)).nat, LAT(OP_DEST1(op)));
  }
  if (LAT(OP_DEST2(op))) {
    __PD_delay(&bit, &REG(OP_DEST2(op)).nat, LAT(OP_DEST2(op)));
  }
  if (LAT(OP_DEST3(op))) {
    __PD_delay(&bit, &REG(OP_DEST3(op)).nat, LAT(OP_DEST3(op)));
  }
}
#endif /* __PD_FULL_SPECULATION_SUPPORT_ */

/******************************************************************/
/* make_mem_layout 
 * added by M. Palesi and D. Patti
 */
#ifdef __PD_DINERO_
#define IUSSUE_WIDTH 8
#define BASE_ADDR 0

void make_mem_layout(__PD_OP* op_list,int pc)
{
    int pos;

    if (pc>=mem_used)
    {
	if (pc >= mem_size) /* we need to alloc more bytes */
	{
	    mem_size = (pc+1)*2;
    
	    mem_layout=(int*)realloc(mem_layout,mem_size*sizeof(int));

	    /*
	    fprintf(stderr,"\n pc: %d, mem_size: %d, used: %d ",pc,mem_size,mem_used);
	    fflush(stderr);
	    */

	    if (mem_layout==NULL)
	    {
		fprintf(stderr, "realloc failed (mem_size=%d Bytes)\n",(mem_size)*sizeof(int));
		exit(-1);
	    }
	}

	for (pos=mem_used;pos<=pc;pos++)
	{
	    /*
	    fprintf(stderr,"\n pc: %d, mem_size: %d, used: %d, pos=%d",pc,mem_size,*used,pos);
	    fflush(stderr);
	    */

	    if (pos==0)
		mem_layout[0]=BASE_ADDR;
	    else
		if (op_list[pos-1].flags == __PD_NOOPMASK)
		    mem_layout[pos] = mem_layout[pos-1];
	    	else
		    mem_layout[pos] = mem_layout[pos-1]+__PD_DINERO_SYLLABLE_BYTES;
	}
	mem_used = pc+1;
    }
}

bool pc2addr(int pc)
{
    bool jump = __PD_false;
    
    mem_addr = mem_layout[pc];
    if (pc != old_pc+1) 
    {
	jump = __PD_true;
	while (mem_addr%(__PD_DINERO_SYLLABLE_BYTES*IUSSUE_WIDTH)!=0) mem_addr-=__PD_DINERO_SYLLABLE_BYTES;
    }
    
    old_pc =pc;
    return jump;
}
/*************************************************************/
#endif /* __PD_DINERO_ */


bool __PD_simulate(__PD_OP* op_list)
{
    
#ifndef __PD_REMAP_OP_LAYOUT_
  __PD_opnode* op;
#else
  __PD_table*  op = op_list;
#endif /* __PD_REMAP_OP_LAYOUT_ */
  volatile ulong  pc;
  bool speculated_op, notrapping_op;

  for (pc = 0; !__PD_done; pc++) {
    /* advance the program counter */
    __PD_PC_REG = pc;

#ifndef __PD_REMAP_OP_LAYOUT_
    op = &(op_list[pc]);
#endif /* __PD_REMAP_OP_LAYOUT_ */

/*
 * fprintf(stderr, "\n id %d, flag %d at time = %llu  (pc = %ld)", OP_ID(op),OP_FLAGS(op), __PD_global_clock,pc);
*/

#ifdef __PD_DINERO_
    make_mem_layout(op_list,pc);
    jump = pc2addr(pc);
		
    
    if (OP_FLAGS(op) != __PD_NOOPMASK || jump) 
    {
	/* __PD_dinero_inst_load(op);
	*/
	//fprintf(stderr,"\n ADDR : %d",mem_addr);
	if (mem_addr%(__PD_DINERO_SYLLABLE_BYTES*IUSSUE_WIDTH)==0)
	    __PD_dinero_inst_load((__PD_OP*)mem_addr); 
    }

#endif /* __PD_DINERO_ */

#ifdef __PD_DEBUG
    fprintf(stderr, "issuing instruction id %d at time = %llu\n",
		OP_ID(op), __PD_global_clock);
#endif /* __PD_DEBUG */

    /* flag this operation as speculated if necessary */
    speculated_op = OP_FLAGS(op) & __PD_SPOPMASK;
    notrapping_op = OP_FLAGS(op) & __PD_NTOPMASK;

#ifdef __PD_FULL_SPECULATION_SUPPORT_
    /* if one of the source operands was the  result of a speculated 
     * instruction that caused an exception, raise the exception now
     * for normal operations 
     */
    if (__PD_has_speculated_sources(op)) {
	if (!speculated_op) {
	  __PD_raise_exception(OP_ID(op), 0);
      }
	/* this operation is scheduled speculatively and need not execute
	 * since an exception occured while computing at least one of its 
	 * operands 
	 */
	else {
	  /* __PD_set_op_speculative_tag(op, __PD_true); */
	  /* update statistics - no reason why this should be a pseudo operation */
	  assert(OP_FLAGS(op) != __PD_NOOPMASK);
	  __PD_update_stats(op);
	  __PD_statistics->spec_squash++;
	  continue;
	}
    }
#endif /* __PD_FULL_SPECULATION_SUPPORT_ */

    /* for non-speculated operations, an exception raised while simulating it
     * must be raised  immediately - the flag  is set to (current op  id + 1)
     * since  pseudo-operations  have a  rebel_id  of  0  by convention,  for
     * speculated operations, the exception is masked 
     */
    if (notrapping_op) {
	/* count the number of non-trapping ops */
	__PD_statistics->notrapping++;

	/* will mask the exception if it occurs */
	__PD_do_not_trap_raised_exception = __PD_false;

	/* if an exception is encountered, simulation resumes here */
	if (setjmp(__PD_jump_buffer) == __PD_delayed_exception) {
	  /* sanity check - no reason why this should be a pseudo operation */
	  assert(__PD_exception_raised & (OP_FLAGS(op) != __PD_NOOPMASK));

	  /* reset exception flag */
	  __PD_exception_raised = __PD_false;

#ifdef __PD_FULL_SPECULATION_SUPPORT_
	  /* set the speculative tag on the destination registers */
	  __PD_set_op_speculative_tag(op, __PD_true);
#endif /* __PD_FULL_SPECULATION_SUPPORT_ */

	  /* update statistics */
	  __PD_update_stats(op);
	  __PD_statistics->spec_except++;
	  continue;
	}
    }
    else {
	/* store the current op id to be used in case of an exception */
	__PD_do_not_trap_raised_exception = OP_ID(op) + 1;
    }

#ifdef __PD_TRACE_ALL_OPS_
    if (__PD_parameters->flags & __PD_CFGTMASK) {
	__PD_trace_op(op);
    }
#endif /* __PD_TRACE_ALL_OPS_ */

    /* invoke the function that implements the current operation */
    (*OP_IMPL(op))(op);

#ifdef __PD_FULL_SPECULATION_SUPPORT_
    /* this operation did not cause an exception and does not have any
     * operands that resulted from operations that caused an exception
     * and  so the exception  tag on the destination operands is reset
     * note that when full speculation support  is not enabled this is
     * not alway true  but it is assumed that  such instances are safe
     */
    __PD_set_op_speculative_tag(op, __PD_false);
#endif /* __PD_FULL_SPECULATION_SUPPORT_ */

    /* when a longjmp is executed,  the interpreter must force the necessary
     * number of function returns, so that the actual C program stack points
     * to the  proper function  a side effect of this  approach is  that any
     * memory allocated to preserve speculative tags may  not be deallocated
     */
    if (__PD_longjmp_executed) {
	if (__PD_longjmp_depth) {
	  __PD_longjmp_depth--;
	  __PD_decrement_longjump_depth_count();
	  return __PD_false;
	}
	/* the actual program stack frame now should point to the "C" longjmp
	 * target function
	 */
	__PD_longjmp_executed = __PD_false;
    }

    if (__PD_parameters->flags & __PD_DYNSMASK) {
	/* ignore pseudo ops - note that predicated ops are counted as 
	 * issued but marked as nullified
       */
	if (OP_FLAGS(op) != __PD_NOOPMASK) {
	  /* update the count of speculated ops */
	  __PD_statistics->speculated += (speculated_op != __PD_false);
	  __PD_update_stats(op);
	}
    }

    /* in case of a branch operation, the local program counter should be
     * adjusted as per the global program counter
     */
    pc = __PD_PC_REG;

  }

  __PD_done = __PD_false;
  return __PD_true;
}
