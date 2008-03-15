/******************************************************************************

                    SOFTWARE LICENSE AGREEMENT NOTICE
                   -----------------------------------

IT IS A BREACH OF THIS LICENSE AGREEMENT TO REMOVE THIS NOTICE FROM THE FILE
OR SOFTWARE, OR ANY MODIFIED VERSIONS OF THIS FILE OR SOFTWARE OR DERIVATIVE
WORKS.
___________________________________________________

Copyright Notices/Identification of Licensor(s) of Original Software
in the File


All rights reserved by the foregoing, respectively.
___________________________________________________

Copyright Notices/Identification of Subsequent Licensor(s)/Contributors of
Derivative Works

Copyright 2007 University of Michigan

All rights reserved by the foregoing, respectively.
___________________________________________________

The code contained in this file, including both binary and source [if released
by the owner(s)] (hereafter, Software) is subject to copyright by the
respective Licensor(s) and ownership remains with such Licensor(s).  The
Licensor(s) of the original Software remain free to license their respective
proprietary Software for other purposes that are independent and separate from
this file, without obligation to any party.

Licensor(s) grant(s) you (hereafter, Licensee) a license to use the Software
for academic, research and internal business purposes only, without a fee.
"Internal business purposes" means that Licensee may install, use and execute
the Software for the purpose of designing and evaluating products.  Licensee
may submit proposals for research support, and receive funding from private
and Government sponsors for continued development, support and maintenance of
the Software for the purposes permitted herein.

Licensee may also disclose results obtained by executing the Software, as well
as algorithms embodied therein.  Licensee may redistribute the Software to
third parties provided that the copyright notices and this License Agreement
Notice statement are reproduced on all copies and that no charge is associated
with such copies. No patent or other intellectual property license is granted
or implied by this Agreement, and this Agreement does not license any acts
except those expressly recited.

Licensee may modify the Software to make derivative works (as defined in
Section 101 of Title 17, U.S. Code) (hereafter, Derivative Works), as
necessary for its own academic, research and internal business purposes.
Title to copyrights and other proprietary rights in Derivative Works created
by Licensee shall be owned by Licensee subject, however, to the underlying
ownership interest(s) of the Licensor(s) in the copyrights and other
proprietary rights in the original Software.  All the same rights and licenses
granted herein and all other terms and conditions contained in this Agreement
pertaining to the Software shall continue to apply to any parts of the
Software included in Derivative Works.  Licensee's Derivative Work should
clearly notify users that it is a modified version and not the original
Software distributed by the Licensor(s).

If Licensee wants to make its Derivative Works available to other parties,
such distribution will be governed by the terms and conditions of this License
Agreement.  Licensee shall not modify this License Agreement, except that
Licensee shall clearly identify the contribution of its Derivative Work to
this file by adding an additional copyright notice to the other copyright
notices listed above, to be added below the line "Copyright
Notices/Identification of Subsequent Licensor(s)/Contributors of Derivative
Works."  A party who is not an owner of such Derivative Work within the
meaning of U.S. Copyright Law (i.e., the original author, or the employer of
the author if "work of hire") shall not modify this License Agreement or add
such party's name to the copyright notices above.

Each party who contributes Software or makes a Derivative Work to this file
(hereafter, Contributed Code) represents to each Licensor and to other
Licensees for its own Contributed Code that:

(a)  Such Contributed Code does not violate (or cause the Software to
violate) the laws of the United States, including the export control laws of
the United States, or the laws of any other jurisdiction.

(b)  The contributing party has all legal right and authority to make such
Contributed Code available and to grant the rights and licenses contained in
this License Agreement without violation or conflict with any law.

(c)  To the best of the contributing party's knowledge and belief, the
Contributed Code does not infringe upon any proprietary rights or intellectual
property rights of any third party.

LICENSOR(S) MAKE(S) NO REPRESENTATIONS ABOUT THE SUITABILITY OF THE SOFTWARE
OR DERIVATIVE WORKS FOR ANY PURPOSE.  IT IS PROVIDED "AS IS"    WITHOUT
EXPRESS OR IMPLIED WARRANTY, INCLUDING BUT NOT LIMITED TO THE MERCHANTABILITY,
USE OR FITNESS FOR ANY PARTICULAR PURPOSE AND ANY WARRANTY AGAINST
INFRINGEMENT OF ANY INTELLECTUAL PROPERTY RIGHTS.  LICENSOR(S) SHALL NOT BE
LIABLE FOR ANY DAMAGES SUFFERED BY THE USERS OF THE SOFTWARE OR DERIVATIVE
WORKS.

Any Licensee wishing to make commercial use of the Software or Derivative
Works should contact each and every Licensor to negotiate an appropriate
license for such commercial use, and written permission of all Licensors will
be required for such a commercial license.  Commercial use includes (1)
integration of all or part of the source code into a product for sale by or on
behalf of Licensee to third parties, or (2) distribution of the Software or
Derivative Works to third parties that need it to utilize a commercial product
sold or licensed by or on behalf of Licensee.

By using or copying this Contributed Code, Licensee agrees to abide by the
copyright law and all other applicable laws of the U.S., and the terms of this
License Agreement.  Any individual Licensor shall have the right to terminate
this license immediately by written notice upon Licensee's breach of, or
non-compliance with, any of its terms.  Licensee may be held legally
responsible for any copyright infringement that is caused or encouraged by
Licensee's failure to abide by the terms of this License Agreement.

******************************************************************************/

#include "PD_m5e.h"
#include "PD_error.h"
#include "PD_kernel.h"
#include "PD_reg.h"
#include "PD_stats.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "m5elements/cache.hh"
#include "m5elements/mem_types.hh"

#define NUM_CLUSTERS 4   //HACK //TODO: get this from somewhere

enum __PD_stall_reason {
  __PD_STALL_INSTRUCTION_LOAD,
  __PD_STALL_ON_LOAD_USE,
  __PD_STALL_REQ_QUEUE_FULL,
  __PD_STALL_EPILOGUE
};

typedef enum __PD_stall_reason __PD_stall_reason;

static const char *__PD_STALL_INSTRUCTION_LOAD_STR = "STALL_INSTRUCTION_LOAD";
static const char *__PD_STALL_ON_LOAD_USE_STR = "STALL_ON_LOAD_USE";
static const char *__PD_STALL_REQ_QUEUE_FULL_STR = "STALL_REQ_QUEUE_FULL";
static const char *__PD_STALL_EPILOGUE_STR = "STALL_EPILOGUE";
static const char *__PD_STALL_REASON_UNKNOWN_STR = "STALL_REASON_UNKNOWN";



static long __PD_m5e_max_req_id;

static inline void advance_cache_state_one_cycle(bool advance_simu_clock);
static inline void stall_one_cycle(const __PD_OP*, const struct __PD_port *p, const int stall_cycles,
    __PD_stall_reason);
static inline const char *__PD_stall_reason_to_str(__PD_stall_reason);
static void update_ready_requests();
static void wait_on_operand(const __PD_OP* op, const struct __PD_port* p);

extern void
__PD_m5e_initialize()
{
  M5E_init_system(NUM_CLUSTERS);
  __PD_m5e_max_req_id = 0;
  atexit(M5E_destruct_system);
}

extern bool
__PD_m5e_data_load(const __PD_OP* op, __PD_width w, __PD_addressing_type t)
{
  M5E_mem_req req;
  int cid = op->cid;
  int stall_cycles = 0;

  if((op->flags & __PD_VLD_OP) == __PD_VLD_OP)
    __PD_punt("M5Elements cache simuation does not support vector loads. Op: %d", 
              op->rebel_id);
  __PD_assert(cid >= 0 && cid < NUM_CLUSTERS);

  req.type = M5E_LOAD;
  req.data = 0;            /* our use of M5 only simulates timing, not data */
  req.size = 4;
  req.is_valid = 1;
  req.id = __PD_m5e_max_req_id++;
  req.register_id = (int) (&SREG(OP_DEST0(op))); 

  /*
   * Associate the defining Op's PD_opstats with the destination register
   * The request send to the cache sub-system contains info
   * about the destination register which inturn points to the
   * defining Op's stats.
   */
  __PD_reg *reg = (__PD_reg *) (&SREG(OP_DEST0(op)));
  reg->user_handler = (__PD_opstats *) &__PD_statistics->ops[OP_BBIDX(op)];

  if (t == __PD_I)
    req.addr    = SREG(OP_SRC0(op)).reg.gpr;
  else if (t == __PD_L)
    req.addr    = LIT(OP_SRC0(op));
  else if (t == __PD_II)
    req.addr    = SREG(OP_SRC0(op)).reg.gpr + SREG(OP_SRC1(op)).reg.gpr;
  else if (t == __PD_IL)
    req.addr    = SREG(OP_SRC0(op)).reg.gpr + LIT(OP_SRC1(op));
  else if (t == __PD_LL)
    req.addr    = LIT(OP_SRC0(op))+ LIT(OP_SRC1(op));
  else
    assert(0);

#ifdef __PD_DEBUG
  fprintf(stderr, "DEBUG: issue load: op %d, cid %d, clock %d\n",
      op->rebel_id, op->cid, __PD_global_clock);
  fflush(stderr);
#endif /* __PD_DEBUG */

  /* if request queue is full, stall now. */
  while (M5E_is_dcache_request_queue_full(cid)) {
    stall_one_cycle(op, NULL, ++stall_cycles, __PD_STALL_REQ_QUEUE_FULL);
  }

  M5E_send_dcache_request(req, cid);

  SREG(OP_DEST0(op)).pending = __PD_REG_BUSY;

#ifdef __PD_TRACK_OP_STATS_
  //TODO: maybe keep track of L1 misses per op as dinero iface does?
#endif
  return __PD_false; /* assume miss this cycle, loads take at least 1 cycle */
}

extern void
__PD_m5e_data_store(const __PD_OP* op, __PD_width w, __PD_addressing_type t)
{
  M5E_mem_req req;
  int cid = op->cid;
  int stall_cycles = 0;

  /* M5Elements does not currently support vector stores */
  if((op->flags & __PD_VST_OP) == __PD_VST_OP)
    __PD_punt("M5Elements cache simuation does not support vector stores. Op: %d", 
              op->rebel_id);
  __PD_assert(cid >= 0 && cid < NUM_CLUSTERS);

  req.type = M5E_STORE;
  req.data = 0;            /* our use of M5 only simulates timing, not data */
  req.size = 4;
  req.is_valid = 1;
  req.id = __PD_m5e_max_req_id++;

  if (t == __PD_I)
    req.addr    = SREG(OP_SRC0(op)).reg.gpr;
  else if (t == __PD_L)
    req.addr    = LIT(OP_SRC0(op));
  else if (t == __PD_II)
    req.addr    = SREG(OP_SRC0(op)).reg.gpr + SREG(OP_SRC1(op)).reg.gpr;
  else if (t == __PD_IL)
    req.addr    = SREG(OP_SRC0(op)).reg.gpr + LIT(OP_SRC1(op));
  else if (t == __PD_LL)
    req.addr    = LIT(OP_SRC0(op))+ LIT(OP_SRC1(op));
  else
    assert(0);
  
  /* if request queue is full, stall now. */
  while (M5E_is_dcache_request_queue_full(cid)) {
    stall_one_cycle(op, NULL, ++stall_cycles, __PD_STALL_REQ_QUEUE_FULL);
  }

  M5E_send_dcache_request(req, cid);
}

extern void
__PD_m5e_inst_load(const __PD_OP* op)
{
  /* Not functional yet -- need to handle ICMOVE, which has no cid */
#if 0
  M5E_mem_req req;
  int cid = op->cid;
  int stall_cycles = 0;
  bool hit;

  if (cid < 0 || cid >= NUM_CLUSTERS)
    __PD_punt("op %d had invalid cluster id: %d", op->rebel_id, op->cid);

  req.type = M5E_LOAD;
  req.data = 0;            /* our use of M5 only simulates timing, not data */
  req.size = __PD_parameters->bytes_per_op;
  req.is_valid = 1;
  req.id = __PD_m5e_max_req_id++;
  req.register_id = 0;    /* not used for I-cache */
  req.addr = OP_ADDR(op);


#ifdef __PD_DEBUG
  fprintf(stderr, "DEBUG: instruction load: op %d, cid %d, clock %d\n",
      op->rebel_id, op->cid, __PD_global_clock);
  fflush(stderr);
#endif /* __PD_DEBUG */

  hit = M5E_is_present_in_icache(req, cid);

  /* Send the request either way, this ensures the cache gets a request  *
   * and updates its replacement policy, etc.                            */
  M5E_send_icache_request(req, cid);

  /* See if we got a hit. */
  while (!hit) {
    stall_one_cycle(op, ++stall_cycles, __PD_STALL_INSTRUCTION_LOAD);
    hit = M5E_is_present_in_icache(req, cid);
  }
#endif
}

extern void
__PD_m5e_check_src_regs(const __PD_OP* op)
{
  /* check  the pending  bit on  a register;  since this  is  a normal
   * operation and only the right most bit should to be set if data is
   * outstanding
   */
  if (IS_REG(OP_PRED(op)) && SREG(OP_PRED(op)).pending) {
    __PD_assert((SREG(OP_PRED(op)).pending ^ __PD_REG_BUSY) == 0);
    wait_on_operand(op, &OP_PRED(op));
  }
  if (IS_REG(OP_SRC0(op)) && SREG(OP_SRC0(op)).pending) {
    __PD_assert((SREG(OP_SRC0(op)).pending ^ __PD_REG_BUSY) == 0);
    wait_on_operand(op, &OP_SRC0(op));
  }
  if (IS_REG(OP_SRC1(op)) && SREG(OP_SRC1(op)).pending) {
    __PD_assert((SREG(OP_SRC1(op)).pending ^ __PD_REG_BUSY) == 0);
    wait_on_operand(op, &OP_SRC1(op));
  }
  if (IS_REG(OP_SRC2(op)) && SREG(OP_SRC2(op)).pending) {
    __PD_assert((SREG(OP_SRC2(op)).pending ^ __PD_REG_BUSY) == 0);
    wait_on_operand(op, &OP_SRC2(op));
  }
  if (IS_REG(OP_SRC3(op)) && SREG(OP_SRC3(op)).pending) {
    __PD_assert((SREG(OP_SRC3(op)).pending ^ __PD_REG_BUSY) == 0);
    wait_on_operand(op, &OP_SRC3(op));
  }
#ifdef __PD_TRACK_OP_STATS_
  //TODO
#endif
}

/** Do any necessary cache operations at the end of a function. */
extern void
__PD_m5e_epilogue(const __PD_OP* op)
{
  int stall_cycles = 0;

  /* Memory requests are given pointers to registers, and they modify those
   * registers' pending bits. If we are using virtual registers, the registers
   * will be deallocated upon function exit (epilogue). 
   *
   * This can be solved in one of two ways:
   *     (1) wait for all outstanding memory requests upon function exit
   *     (2) keep track of which function instance a memory request was made 
   *         within, and ignore the response if it comes back after that 
   *         particular function instance has exited
   *
   * We go with option (1) here.
   *
   * By the way, this happens because a load might have been hoisted above
   * a predicated RTS, so we never encounter the load use (and stall on it).
   *
   *                                          <lieberm 08/16/2006>
   */

   
  while (M5E_get_outstanding_requests() != 0) {
    stall_one_cycle(op, NULL, ++stall_cycles, __PD_STALL_EPILOGUE);
#if 0
    fprintf(stderr, 
	"DEBUG: stalled: op %d, cid %d, for %d cycles, clock %d reason: %s\n",
	op->rebel_id, op->cid, stall_cycles, __PD_global_clock,
	__PD_stall_reason_to_str(__PD_STALL_EPILOGUE));
    fflush(stderr);
#endif
  }
}

extern void
__PD_m5e_advance_state_one_step()
{
  advance_cache_state_one_cycle(__PD_false);
}

extern void
__PD_m5e_write_stats()
{
  /* do nothing -- stats will be written out by M5E_destruct_system(), which
   * was passed to atexit()
   */
}

static inline const char*
__PD_stall_reason_to_str(__PD_stall_reason reason)
{
  switch (reason) {
    case __PD_STALL_INSTRUCTION_LOAD:
      return __PD_STALL_INSTRUCTION_LOAD_STR;
    case __PD_STALL_ON_LOAD_USE:
      return __PD_STALL_ON_LOAD_USE_STR;
    case __PD_STALL_REQ_QUEUE_FULL:
      return __PD_STALL_REQ_QUEUE_FULL_STR;
    case __PD_STALL_EPILOGUE:
      return __PD_STALL_EPILOGUE_STR;
    default:
      return __PD_STALL_REASON_UNKNOWN_STR;
  }
}

static inline void
advance_cache_state_one_cycle(bool advance_simu_clock)
{
  /* increment clock on stalls */
  if (advance_simu_clock)
    __PD_global_clock++;

  M5E_advance_clock();
  update_ready_requests();
}

/**
 * Stalls the entire machine for one cycle on the specified op, advancing
 * the simu clock, the M5 clock, and increasing the stall statistics.
 */
static inline void
stall_one_cycle(const __PD_OP* op, const struct __PD_port *p, const int stall_cycles,
    __PD_stall_reason reason)
{

#ifdef __PD_TRACK_OP_STATS_
  /* advance_cache_state deallocates the current event pointer */
#endif /* __PD_TRACK_OP_STATS_ */

#ifdef __PD_DEBUG 
  fprintf(stderr, 
      "DEBUG: stalled: op %lu, cid %d, for %d cycles, clock %llu reason: %s\n",
      op->rebel_id, op->cid, stall_cycles, __PD_global_clock,
      __PD_stall_reason_to_str(reason));
  fflush(stderr);
#endif /* __PD_DEBUG */

  if (stall_cycles > 500000) 
    __PD_punt("stalled on op %d for too long! stall cycles: %d reason: %s\n",
	op->rebel_id, stall_cycles, __PD_stall_reason_to_str(reason));

  if (__PD_parameters->flags & __PD_DYNSTATS) {
    __PD_assert(!__PD_statistics->is_procedure);
    __PD_statistics->stall_cycles++;
    switch (reason) {

      case __PD_STALL_INSTRUCTION_LOAD:
	__PD_statistics->icache_stalls++;
	break;

      case __PD_STALL_ON_LOAD_USE: {
        /* charge the defining op */
        __PD_reg reg = PREG(p);
        __PD_opstats *def_op_stats = (__PD_opstats *) reg.user_handler;
        __PD_assert(def_op_stats);
#ifdef __PD_DEBUG 
        __PD_OP *def_op = def_op_stats->op_node;
        fprintf(stderr, 
                "DEBUG: defining op %lu, cid %d, for %d cycles, clock %llu reason: %s\n",
                def_op->rebel_id, def_op->cid, stall_cycles, __PD_global_clock,
                __PD_stall_reason_to_str(reason));
        fflush(stderr);
#endif /* __PD_DEBUG */
        def_op_stats->caused_use_stalls++;
	__PD_statistics->use_stalls++;
        break;
      }
      case __PD_STALL_REQ_QUEUE_FULL:
      case __PD_STALL_EPILOGUE:
	__PD_statistics->use_stalls++;
	break;
    }
#ifdef __PD_TRACK_OP_STATS_
    __PD_statistics->ops[OP_BBIDX(op)].taken_use_stalls++;
#endif /* __PD_TRACK_OP_STATS_ */
  }

  advance_cache_state_one_cycle(__PD_true);
}

static void
update_ready_requests()
{
  int* ready_reqs = M5E_ready_loads();
  int i = 0;

  while (ready_reqs[i] != -1) {
    if (ready_reqs[i] == 0) {
      /* do nothing; I-cache requests don't have register ids */
    } else {
      __PD_reg* reg = (__PD_reg*) ready_reqs[i];
      reg->pending = __PD_REG_READY;
    }
    i++;
  }
}

static void
wait_on_operand(const __PD_OP* op, const struct __PD_port* p)
{
  int stall_cycles = 0;

  __PD_assert(PREG(p).pending);
  __PD_assert((PREG(p).pending ^ __PD_REG_BUSY) == 0);

  while (PREG(p).pending) {
    stall_one_cycle(op, p, ++stall_cycles, __PD_STALL_ON_LOAD_USE);
  }

  __PD_assert(!PREG(p).pending);
}

#undef __PD_DEBUG

