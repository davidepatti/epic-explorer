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

Copyright  2003  Massachusetts  Institute  of Technology.  All  rights
reserved by the foregoing, respectively.

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
 * File:    PD_dinero.c
 * Authors: Rodric M. Rabbah, Weng Fai Wong
 *
 * Description: interface to the Dinero-4 cache simulator
 *****************************************************************************/


/******************************************************************************
 * Weng Fai Wong implemented the following functionality:
 *   - exact timing for load and store operations
 *   - write (store) buffers
 *   - branch prediction
 *   - TLBs
 *   - stall on use
 *
 * 1. Exact timing and write buffers
 * =================================
 * Two queues, the  cache event queue and the  write buffer queue, are
 * implemented.  Because Dinero  commits  changes instantaneously,  we
 * have to queue events till it  is time to execute the event. This is
 * what the cache event queue does.
 *
 * For the write buffer queue, the queue is merely to correctly record
 * occupancy of  the write buffer.  In order to simulate  write buffer
 * cross referencing during loads,  data writes are actually committed
 * immediately  IF  there is  at  least one  free  slot  in the  write
 * buffer. Otherwise, a global processor stall is requested.
 *
 * The  queues are scanned  during PD_aclock()  execution for  as many
 * times as there are outstanding processor stalls. (See the code.)
 *
 * 2. Store on use
 * ===============
 * Each register data structure has a pending bit. In the main loop of
 * the  simulator,  before  any   op  is  executed,  its  sources  are
 * scanned. If any  of its register source has the  pending bit set, a
 * processor  stall if  requested.  This request  is entertained  only
 * during the next PD_aclock() when the queues are scanned.
 *
 * 3. Branch prediction
 * ====================
 * The gshare branch prediction scheme is implemented. (Ask google.) A
 * call  to PD_handle_branch  had  to be  inserted  for every  branch,
 * including predicated ones. PD_ops.list and gen/gen_functions.c were
 * modified to  do this.   If a branch  is predicted correctly,  it is
 * rewarded with  a "free-of-charge" access  to the icache;  I'm still
 * not quite sure  if this is completely correct but  it should err on
 * the generous side. If it fails, the processor is stall for a number
 * of misprediction cycles.
 *
 * 4. TLBs
 * =======
 * The instruction and data TLBs  are two Dinero caches rooted at main
 * memory. They are fully associative. There are accessed concurrently
 * with the  icache and dcache,  respectively. The stall cycle  is the
 * max of the two.
 *
 * IMPORTANT NOTES:
 * ================
 * - The  stall cycle is  already added  to __PD_global_clock.  Not an
 *   absolute necessity.
 * - All cache  parameters are hard-wired  in dinero_default_memh().
 * - You  must make the  whole stuff  using the  __PD_DINERO_ compiler
 *   flag.
 *
 * Weng-Fai
 * Oct 24, 2003
 *
 *****************************************************************************/

/*************************************************************************
   Modified for EpicExplorer (epic-explorer.sf.net)
    - added __EE_load_cache_config() function 
    - added code for computing bus transitions 

 TODO: make __EE_load_cache_config() use trimaran hmdes memory files

 Davide Patti (dpatti@diit.unict.it)
 May 2005
 *************************************************************************/

/* Use this line to enable/disable Epic Explorer code */
#define EPIC_EXPLORER


/*******************************************************/
#ifdef __PD_SSDINERO_
#  include "SSdinero/PD_dinero.c" /* use SimpleScalar version */
#else

#include <string.h>
#include "PD_dinero.h"
#include "PD_kernel.h"
#include "PD_queue.h"
#include "PD_parms.h"
#include "PD_error.h"
#include "PD_stats.h"
#include <dinero.h>


/******************************************************************************
 * Memory Units; lazy to read the Mdes file... FIX ME
 *****************************************************************************/
#define NUM_MEM_UNITS       2
#define WRITE_BUFFER_SLOTS  10
#define WRITE_LATENCY       1

#define PERFECT_ICACHE      0
#define PERFECT_DCACHE      0
#define PERFECT_MEMH        0
#define PERFECT_TLB         1
#define PERFECT_BTB         1
#define PERFECT_BRANCH_PRED 0



/******************************************************************************
 * Memory Hierarchy
 *****************************************************************************/

#define DINERO_UCACHE   0
#define DINERO_ICACHE   1
#define DINERO_DCACHE   2
#define DINERO_CACHETY  3
#define DINERO_MAX_LEV  5


#define ONE_STEP        1

#ifdef EPIC_EXPLORER
/******************************************************************************
 * Added by Davide Patti for Epic-Explorer
 ******************************************************************************/

unsigned long long total_L1I_refs;
unsigned long long total_L1D_refs;
unsigned long long total_L2U_refs;

unsigned int previous_L1I_ref;
unsigned int previous_L1D_ref;
unsigned int previous_L2U_ref;

unsigned int current_L1I_ref;
unsigned int current_L1D_ref;
unsigned int current_L2U_ref;

unsigned long long total_L1I_hamming;
unsigned long long total_L1D_hamming;
unsigned long long total_L2U_hamming;

double average_L1I_hamming;
double average_L1D_hamming;
double average_L2U_hamming;

void __EE_load_cache_config(d4cache* UC,d4cache* IC,d4cache* DC);
#define CACHE_FILE "/trimaran-workspace/cache.cfg"
#endif
/*****************************************************************************/

struct PD_PORT_NODE {
  __PD_reg* reg;
  struct PD_PORT_NODE* next;
};

struct CACHE_EVENT {
  d4cache*  cache;
  d4memref  memref;
  u64bitint timestamp;
  uint      tick;

  struct PD_PORT_NODE* port_list;
  __PD_opstats*        def_op;
  struct CACHE_EVENT*  prev;
  struct CACHE_EVENT*  next;
}; 

struct QUE {
  struct CACHE_EVENT* head;
  struct CACHE_EVENT* tail;
};

static struct QUE cache_event_q;
static struct QUE write_buffer;

static int      dinero_maxlevel  = 0;
static d4cache* dinero_memh[DINERO_CACHETY][DINERO_MAX_LEV] = {{NULL}};

static d4cache* dinero_L1_ICACHE = NULL;
static d4cache* dinero_L1_DCACHE = NULL;
static d4cache* dinero_UCACHE    = NULL;
static d4cache* dinero_ITLB      = NULL;
static d4cache* dinero_DTLB      = NULL;
static d4cache* dinero_BTB       = NULL;

static uint      available_write_buffers = 0;

static u64bitint dinero_branch_stalls    = 0;
static u64bitint dinero_icache_stalls    = 0;
static u64bitint dinero_wb_stalls        = 0;
static u64bitint dinero_use_stalls       = 0;
static u64bitint dinero_block_misaligned = 0;



/******************************************************************************
 * Branch Predictor
 *****************************************************************************/

#define SIZE_OF_BRANCH_PREDICTOR_TBL 512
#define BRANCH_MISPREDICTION_PENALTY 4

static char gshare_branch_predictor_tbl[SIZE_OF_BRANCH_PREDICTOR_TBL];
static char bimode_branch_predictor_tbl[SIZE_OF_BRANCH_PREDICTOR_TBL];
static char which_branch_predictor_tbl[SIZE_OF_BRANCH_PREDICTOR_TBL];
static int  GR = 0;
static int  cur_gpred, cur_bpred;

static u64bitint dinero_branches = 0;
static u64bitint dinero_branch_mispredictions = 0;


/******************************************************************************
 * Statics
 *****************************************************************************/

static void dinero_default_memh(d4cache* memh[DINERO_CACHETY][DINERO_MAX_LEV], int* maxlevel);
static struct CACHE_EVENT* enque_cache_event(struct QUE* q, d4cache*, d4memref, int tick, const struct __PD_port*, __PD_opstats*);
static void check_operand(const __PD_OP* op, const struct __PD_port *p);
static void advance_cache_state_one_step(bool stall);
static int  gshare_predict(uint pc);
static void gshare_br_confirm(uint pc, int outcome);
static int  bimode_predict(uint pc);
static void bimode_br_confirm(uint pc, int outcome);
static int  branch_predict(uint pc);
static void branch_confirm(uint pc, int outcome);

/******************************************************************************
 * Custom Memory Allocation
 *****************************************************************************/

#define POOL_SIZE 2000

static struct PD_PORT_NODE  PORT_NODE_pool[POOL_SIZE];
static struct PD_PORT_NODE* PORT_NODE_pool_head;
static struct PD_PORT_NODE* PD_PORT_NODE_malloc();
static void                 PD_PORT_NODE_free(struct PD_PORT_NODE*);

static struct CACHE_EVENT  CACHE_EVENT_pool[POOL_SIZE];
static struct CACHE_EVENT* CACHE_EVENT_pool_head;
static struct CACHE_EVENT* CACHE_EVENT_malloc();
static void                CACHE_EVENT_free(struct CACHE_EVENT*);

static void init_custom_memory_pools();

inline static int width_to_numbytes(__PD_width w);
inline static int clog2(unsigned int c);


/******************************************************************************
 * initializer and statistic writer
 *****************************************************************************/


/* create and configure caches */
void __PD_dinero_initialize()
{
  int i;

  /* initialize custom memory allocation pool */
  init_custom_memory_pools();
  
  /* initialize branch history */
  for (i = 0; i < SIZE_OF_BRANCH_PREDICTOR_TBL; i++) {
    gshare_branch_predictor_tbl[i] = 2;
    bimode_branch_predictor_tbl[i] = 0;
    which_branch_predictor_tbl[i]  = i % 2; /* pseudo-randomly pick one */
  }

  /* create default memory hiearchy */
  dinero_default_memh(dinero_memh, &dinero_maxlevel);

  cache_event_q.head = cache_event_q.tail = NULL;
  write_buffer.head  = write_buffer.tail  = NULL;

  if (dinero_memh[DINERO_UCACHE][0]) {
    dinero_L1_ICACHE = dinero_memh[DINERO_UCACHE][0];
    dinero_L1_DCACHE = dinero_memh[DINERO_UCACHE][0];
  }
  else {
    __PD_assert(dinero_memh[DINERO_ICACHE][0]);
    dinero_L1_ICACHE = dinero_memh[DINERO_ICACHE][0];

    __PD_assert(dinero_memh[DINERO_DCACHE][0]);
    dinero_L1_DCACHE = dinero_memh[DINERO_DCACHE][0];

    __PD_assert(dinero_memh[DINERO_UCACHE][1]);
    dinero_UCACHE = dinero_memh[DINERO_UCACHE][1];
 }

  __PD_assert(dinero_L1_ICACHE);
  __PD_assert(dinero_L1_DCACHE);
  __PD_assert(dinero_UCACHE);

  if (d4setup()) {
    __PD_punt("Could not complete memory hierarchy setup");
  }
#ifdef EPIC_EXPLORER

total_L1I_refs = 0;
total_L1D_refs = 0;
total_L2U_refs = 0;

previous_L1I_ref = 0;
previous_L1D_ref = 0;
previous_L2U_ref = 0;

current_L1I_ref = 0;
current_L1D_ref = 0;
current_L2U_ref = 0;

total_L1I_hamming = 0;
total_L1D_hamming = 0;
total_L2U_hamming = 0;

average_L1I_hamming = 0;
average_L1D_hamming = 0;
average_L2U_hamming = 0;

#endif
}


/* print cache statistics */
void __PD_dinero_write_stats()
{
  int lev;
  u64bitint total_stalls;

  for (lev = 0; lev < dinero_maxlevel; lev++) {
    if ((dinero_memh[DINERO_ICACHE][lev] != NULL) & !PERFECT_ICACHE) {
	dinero_print_cache_stats(__PD_stats_file, 
					 dinero_memh[DINERO_ICACHE][lev]);
    }
    if (dinero_memh[DINERO_DCACHE][lev] != NULL) {
	dinero_print_cache_stats(__PD_stats_file, 
					 dinero_memh[DINERO_DCACHE][lev]);
    }
    if (dinero_memh[DINERO_UCACHE][lev] != NULL) {
	dinero_print_cache_stats(__PD_stats_file, 
					 dinero_memh[DINERO_UCACHE][lev]);
    }
  }
  
  if (dinero_ITLB && !PERFECT_TLB) {
    dinero_print_cache_stats(__PD_stats_file, dinero_ITLB);
  }
  if (dinero_DTLB && !PERFECT_TLB) {
    dinero_print_cache_stats(__PD_stats_file, dinero_DTLB);
  }
  if (dinero_BTB && !PERFECT_BTB && !PERFECT_BRANCH_PRED) {
    dinero_print_cache_stats(__PD_stats_file, dinero_BTB);
  }
  
  total_stalls = dinero_branch_stalls + dinero_icache_stalls + dinero_wb_stalls + dinero_use_stalls;

  fprintf(__PD_stats_file, "branches..............................%-10llu\n", 
	    dinero_branches);

  fprintf(__PD_stats_file, "block misaligned loads................%-10llu\n", 
	    dinero_block_misaligned);

  fprintf(__PD_stats_file, "mispredicted_branches.................%-10llu (%6.2f %%)\n\n", 
	    dinero_branch_mispredictions, 100 * dinero_branch_mispredictions / (double) dinero_branches);

  fprintf(__PD_stats_file, "total_branch_stalls...................%-10llu (%6.2f %%)\n",
	    dinero_branch_stalls, 100 * dinero_branch_stalls / (double) __PD_global_clock);

  fprintf(__PD_stats_file, "total_icache_stalls...................%-10llu (%6.2f %%)\n",
	    dinero_icache_stalls, 100 * dinero_icache_stalls / (double) __PD_global_clock);

  fprintf(__PD_stats_file, "total_wb_stalls.......................%-10llu (%6.2f %%)\n",
	    dinero_wb_stalls, 100 * dinero_wb_stalls / (double) __PD_global_clock);

  fprintf(__PD_stats_file, "total_use_stalls......................%-10llu (%6.2f %%)\n",
	    dinero_use_stalls, 100 * dinero_use_stalls / (double) __PD_global_clock);

  fprintf(__PD_stats_file, "total_stall_cycles....................%-10llu (%6.2f %%)\n",
	    total_stalls, 100 * total_stalls / (double) __PD_global_clock);

  fprintf(__PD_stats_file, "total_execution_cycles................%-10llu\n", __PD_global_clock);

#ifdef EPIC_EXPLORER
// Write a temporary file with transition count. It will be read by
// Epic Explorer during the exploration 
  FILE * fp;
  char filename[100];
  
    strcpy(filename,getenv("HOME"));
    strcat(filename,"/trimaran-workspace/epic-explorer/tmp_transition");
    
    if ((fp=fopen(filename,"w"))==NULL)
    {
	printf("\n ERROR: can't' write tmp_transition file !");
	exit(1);
    }

  average_L1I_hamming = ((double)total_L1I_hamming/total_L1I_refs)/32;
  average_L1D_hamming = ((double)total_L1D_hamming/total_L1D_refs)/32;

  fprintf(fp,"%g %g",average_L1I_hamming,average_L1D_hamming);
  fclose(fp);
  //printf("\n hamming finale L1I = %g, L1D = %g",average_L1I_hamming,average_L1D_hamming);
#endif
}

#ifdef EPIC_EXPLORER
inline int hamming_distance(unsigned int a,unsigned int b)
{
    int h=0;
    unsigned int c = a ^ b;

    while (c!=0)
    {
	h+= c & 1;
	c >>= 1;
    }
    return h;
}
#endif


/******************************************************************************
 * main API handles branches, instruction and data reads, data writes,
 * operand availability checks
 *****************************************************************************/


/* intiate branch prediction */
void __PD_handle_branch(__PD_OP* op, uint target_pc, int outcome)
{
  uint  i;
  uint  prediction = 0;
  uint  btb_miss   = 0;

  d4memref m;
  m.address    = OP_ADDR(op);
  m.accesstype = D4XINSTRN;
  m.size       = 4;

  /* update branch count */
  dinero_branches++;

  if (PERFECT_BRANCH_PRED) {
    return;
  }

#ifdef __PD_DEBUG
  fprintf(stderr, "B op %lu\taddress %llu\tsize %d (%s)\n", OP_ID(op), m.address, m.size,  __PD_function_name);
#endif /* __PD_DEBUG */
  
  /* the simulator assumes that all load requests are alignmed mod their size */
  __PD_assert((m.address % m.size) == 0); 

  /* first probe BTB */
  if (!PERFECT_BTB) {
    btb_miss = d4probe_cache(dinero_BTB, m);
  
    /* hit the BTB */
    if (!btb_miss) {
	d4ref(dinero_BTB, m);
    }
    else {
#ifdef __PD_DEBUG
	fprintf(stderr, "\tmissed BTB\n");
#endif /* __PD_DEBUG */
	
	/* missed the BTB */
	d4ref(dinero_BTB, m);
    }
  }
  
  /* check for a wrong prediction and penalize if necessary */
  prediction = branch_predict(m.address);
  
  if ((prediction != outcome) || btb_miss) {
    /* stall processor and advance memory hierarchy state */
#ifdef __PD_DEBUG
    fprintf(stderr, "\tbranch misprediction (latency %d)\n", BRANCH_MISPREDICTION_PENALTY);
#endif /* __PD_DEBUG */
    for (i = 0; i < BRANCH_MISPREDICTION_PENALTY; i += ONE_STEP) {
	advance_cache_state_one_step(__PD_true);
    }
    dinero_branch_stalls += BRANCH_MISPREDICTION_PENALTY;
    
    if (prediction != outcome) {
	dinero_branch_mispredictions++;
	if (__PD_parameters->flags & __PD_DYNSTATS) {
	  __PD_statistics->branch_mispredicts++;
#ifdef __PD_TRACK_OP_STATS_
	  __PD_statistics->ops[OP_BBIDX(op)].branch_mispredicts++;
#endif /* __PD_TRACK_OP_STATS_ */
	}
    }

    if (__PD_parameters->flags & __PD_DYNSTATS) {
	__PD_assert(!__PD_statistics->is_procedure);
	__PD_statistics->stall_cycles  += BRANCH_MISPREDICTION_PENALTY;
	__PD_statistics->branch_stalls += BRANCH_MISPREDICTION_PENALTY;
#ifdef __PD_TRACK_OP_STATS_
	__PD_statistics->ops[OP_BBIDX(op)].branch_stalls += BRANCH_MISPREDICTION_PENALTY;
#endif /* __PD_TRACK_OP_STATS_ */
    }
  }

  branch_confirm(m.address, outcome);
}


/* initiate instruction fetch
 *
 * - for itlb/icache  misses, perform the  d4ref and charge  the stall
 *   cycles immediately
 */
void __PD_dinero_inst_load(const __PD_OP* op)
{
  d4memref m;
  uint latency = 0;
  uint max_latency = 0;
  
  if (PERFECT_ICACHE) {
    return;
  }

  m.address    = OP_ADDR(op);
  m.accesstype = D4XINSTRN;
  m.size       = __PD_parameters->bytes_per_op;

  /* the simulator assumes that all load requests are alignmed mod their size */
  __PD_assert((m.address % m.size) == 0); 

#ifdef __PD_DEBUG
  fprintf(stderr, "I op %lu\taddress %llu\tsize %d (%s)\n", OP_ID(op), m.address, m.size,  __PD_function_name);
#endif /* __PD_DEBUG */

  /* first probe ITLB */
  if (!PERFECT_TLB) {
    max_latency = latency = d4probe_cache(dinero_ITLB, m);

    /* hit the TLB */
    if (!latency) {
	d4ref(dinero_ITLB, m);
    }
    /* missed the TLB */
    else {
#ifdef __PD_DEBUG
	fprintf(stderr, "\tmissed ITLB (latency %d)\n", latency);
	fprintf(stderr, "\t\tfilling ITLB\n");
#endif /* __PD_DEBUG */
	
	/* fill the TLB */
	d4ref(dinero_ITLB, m);
	
#ifdef __PD_TRACK_OP_STATS_
	if (__PD_parameters->flags & __PD_DYNSTATS) {
	  __PD_assert(!__PD_statistics->is_procedure);
	  __PD_statistics->ops[OP_BBIDX(op)].itlb_misses++;
	}
#endif /* __PD_TRACK_OP_STATS_ */
    }
  }

  /* probe L1 instruction cache */
  latency = d4probe_cache(dinero_L1_ICACHE, m);

#ifdef EPIC_EXPLORER
/* Instruction cache address bus is accessed */
  total_L1I_refs++;
  current_L1I_ref = m.address; 
  total_L1I_hamming += hamming_distance(previous_L1I_ref,current_L1I_ref);
  previous_L1I_ref = current_L1I_ref;
#endif
  /* hit the ICACHE */
  if (!latency) {
#ifdef __PD_DEBUG
    fprintf(stderr, "\thit ICACHE\n");
#endif /* __PD_DEBUG */
    d4ref(dinero_L1_ICACHE, m);
  }
  /* missed L1 icache */
  else {
#ifdef __PD_DEBUG
    fprintf(stderr, "\tmissed ICACHE (latency %d)\n", latency);
#endif /* __PD_DEBUG */

    if (latency > max_latency) max_latency = latency;

#ifdef __PD_TRACK_OP_STATS_
    if (__PD_parameters->flags & __PD_DYNSTATS) {
	__PD_assert(!__PD_statistics->is_procedure);
	__PD_statistics->ops[OP_BBIDX(op)].l1i_misses++;
    }
#endif /* __PD_TRACK_OP_STATS_ */

    /* probe L2 */
    latency = d4probe_cache(dinero_UCACHE, m);

    /* missed L2 cache */
    if (latency) { 
#ifdef __PD_DEBUG
    fprintf(stderr, "\tmissed UCACHE (latency %d)\n", latency);
#endif /* __PD_DEBUG */

	if (latency > max_latency) max_latency = latency;

#ifdef __PD_TRACK_OP_STATS_
	if (__PD_parameters->flags & __PD_DYNSTATS) {
	  __PD_assert(!__PD_statistics->is_procedure);
	  __PD_statistics->ops[OP_BBIDX(op)].l2i_misses++;
	}
#endif /* __PD_TRACK_OP_STATS_ */
    }

    /* fill L1 */
    d4ref(dinero_L1_ICACHE, m);
#ifdef __PD_DEBUG
    fprintf(stderr, "\t\tfilling ICACHE\n");
#endif /* __PD_DEBUG */
  }

  if (max_latency) {
    uint i;
#ifdef __PD_DEBUG
    fprintf(stderr, "\tstalling processor (latency %d)\n", max_latency);
#endif /* __PD_DEBUG */

    /* stall processor and advance memory hierarchy state */
    for (i = 0; i < max_latency; i += ONE_STEP) {
	advance_cache_state_one_step(__PD_true);
    }
    dinero_icache_stalls += max_latency;
    
    if (__PD_parameters->flags & __PD_DYNSTATS) {
	__PD_assert(!__PD_statistics->is_procedure);
	__PD_statistics->stall_cycles  += max_latency;
	__PD_statistics->icache_stalls += max_latency;
#ifdef __PD_TRACK_OP_STATS_
	__PD_statistics->ops[OP_BBIDX(op)].icache_stalls += max_latency;
#endif /* __PD_TRACK_OP_STATS_ */
    }
  }
}


/* initiate data write
 *
 * - if there  is a  free write buffer  available, perform  the store
 *   immediately by calling d4ref; this simulates write buffer checking
 *   during loads
 *
 * - in addition, enque the memory reference into the write buffer if
 *   there is a free write buffer, else enque in the cache event queue
 */
void __PD_dinero_data_store(const __PD_OP* op, __PD_width w)
{
  uint i;
  uint stalls  = 0;
  d4memref m;

  if (PERFECT_DCACHE) return;

  m.address    = REG(OP_SRC0(op)).reg.gpr;
  m.accesstype = D4XWRITE;
  m.size       = width_to_numbytes(w);

  /* RMR { support vector loads } */
  for (i = 0; i <= OP_SRC0(op).vec; i++) {
    /* if a vector store, treat it as multiple scalar stores from
     * consecutive addresses
     *
     * NOTE this assumes misalignment only happens with vector
     * stores; compiler otherwise asserts proper alignment
     */
    m.address = m.address + (i * m.size);

    /* the simulator assumes that all store requests are alignmed mod their size */
    __PD_assert((m.address % m.size) == 0); 

#ifdef __PD_DEBUG
    fprintf(stderr, "S op %lu\taddress %llu\tsize %d (%s)\n", OP_ID(op), m.address, m.size, __PD_function_name);
#endif /* __PD_DEBUG */

    /* write buffer is full; stall immediately and wait for a free slot */
    while (available_write_buffers == 0) {
#ifdef __PD_DEBUG
	fprintf(stderr, "\tWBUFF full\n");
	fprintf(stderr, "\tstalling processor (latency %d)\n", ONE_STEP);
#endif /* __PD_DEBUG */
	
	advance_cache_state_one_step(__PD_true);
	stalls += ONE_STEP;
    }

    dinero_wb_stalls += stalls;

    if (__PD_parameters->flags & __PD_DYNSTATS) {
	__PD_assert(!__PD_statistics->is_procedure);
	__PD_statistics->stall_cycles += stalls;
	__PD_statistics->wb_stalls    += stalls;
#ifdef __PD_TRACK_OP_STATS_
	__PD_statistics->ops[OP_BBIDX(op)].wb_stalls += stalls;
#endif /* __PD_TRACK_OP_STATS_ */
    }
    
    /* write buffer slot is availble */
#ifdef __PD_DEBUG
    fprintf(stderr, "\tWBUFF slot available\n");
    fprintf(stderr, "\t\treserve WBUFF slot (latency %d)\n", WRITE_LATENCY);
#endif /* __PD_DEBUG */
    enque_cache_event(&write_buffer, dinero_L1_DCACHE, m, WRITE_LATENCY, NULL, NULL);

    /* update the cache to simulate write buffer checking during loads */
#ifdef __PD_DEBUG
    fprintf(stderr, "\t\tupdate DCACHE (store)\n");
#endif /* __PD_DEBUG */
    d4ref(dinero_L1_DCACHE, m);
    
#ifdef EPIC_EXPLORER
//Data cache address bus is accessed *
  total_L1D_refs++;
  current_L1D_ref = m.address;
  total_L1D_hamming += hamming_distance(previous_L1D_ref,current_L1D_ref);
  previous_L1D_ref = current_L1D_ref;
#endif
    /* mark write buffer slot busy */
    available_write_buffers--;
  }
}


/* initiate data fetch (returns miss (false) or hit (true) */
bool __PD_dinero_data_load(const __PD_OP* op, __PD_width w)
{
  uint i;
  uint any_misses = 0;
  struct CACHE_EVENT *ce = NULL;

  d4memref m;
  m.address    = REG(OP_SRC0(op)).reg.gpr;
  m.accesstype = D4XREAD;
  m.size       = width_to_numbytes(w);

  /* RMR { count misaligned references that span two cache blocks */
  if (OP_SRC0(op).vec) {
    if (D4ADDR2BLOCK(dinero_L1_DCACHE, m.address) != 
	  D4ADDR2BLOCK(dinero_L1_DCACHE, (m.address + (m.size << OP_SRC0(op).vec)))) {
	dinero_block_misaligned++;
    }
  }
  /* } RMR */
  
  if (PERFECT_DCACHE) return __PD_false;

  /* RMR { support vector loads } */
  for (i = 0; i <= OP_DEST0(op).vec; i++) {
    uint tlb_latency = 0;
    uint l1_latency  = 0;
    uint l2_latency  = 0;

    struct CACHE_EVENT  *tlb_ce = NULL;
    struct CACHE_EVENT  *l1_ce  = NULL;
    struct CACHE_EVENT  *l2_ce  = NULL;
    struct CACHE_EVENT  *cep    = NULL;
    struct PD_PORT_NODE *port   = NULL;
    bool miss = __PD_false;
    bool overlapping_miss = __PD_false;

    /* if a vector load, treat it as multiple scalar loads from
     * consecutive addresses
     *
     * NOTE this assumes misalignment only happens with vector
     * loads; compiler otherwise asserts proper alignment
     */
    m.address = m.address + (i * m.size);

#ifdef __PD_DEBUG
    fprintf(stderr, "L op %lu\taddress %llu\tsize %d (%s)\n", OP_ID(op), m.address, m.size, __PD_function_name);
#endif /* __PD_DEBUG */

    /* the simulator assumes that all load requests are alignmed mod their size */
    __PD_assert((m.address % m.size) == 0); 

    /* probe DTLB */
    if (!PERFECT_TLB) {
	tlb_latency = d4probe_cache(dinero_DTLB, m);
	
	/* hit the TLB */
	if (!tlb_latency) {
	  d4ref(dinero_DTLB, m);
	}
	/* missed  DTLB; should  the use  be  dependent on  DTLB or  DCACHE?
	 * depends on the latency; decide later  after we check to see if it
	 * miss the dcache or not
	 */
	else {
	  miss = __PD_true;

#ifdef __PD_DEBUG
	  fprintf(stderr, "\tmissed DTLB (latency %d)\n", tlb_latency);
#endif /* __PD_DEBUG */
	  
#ifdef __PD_TRACK_OP_STATS_
	  if (__PD_parameters->flags & __PD_DYNSTATS) {
	    __PD_assert(!__PD_statistics->is_procedure);
	    __PD_statistics->ops[OP_BBIDX(op)].dtlb_misses++;
	  }
#endif /* __PD_TRACK_OP_STATS_ */
	  
	  /* check oustanding  cache events  for an overlapping  request; if
	   * there  exists such  an event,  use  its cache  event entry  for
	   * bookeeping
	   */
	  for (cep = cache_event_q.head; cep != NULL; cep = cep->next) {
	    if (cep->cache != dinero_DTLB) continue;
	    /* found an overlapping request */
	    if (d4same_blockaddr(dinero_DTLB, &m, &(cep->memref))) {
		/* any reason why an oustanding TLB miss for the
		 * same address will have a longer latency? 
		 */	
		__PD_assert(cep->tick <= tlb_latency);
		tlb_ce = cep;
		overlapping_miss = __PD_true;
#ifdef __PD_DEBUG
		fprintf(stderr, "\t\tfound overlapping miss (latency %d)\n", cep->tick);
#endif /* __PD_DEBUG */
		break;
	    }
	  }

	  /* no outstanding  request for the same address exists; queue request */
	  if (!tlb_ce) {
#ifdef __PD_DEBUG
	    fprintf(stderr, "\t\tenque DTLB request (latency %d)\n", tlb_latency);
#endif /* __PD_DEBUG */
	    tlb_ce = enque_cache_event(&cache_event_q, dinero_DTLB, m, tlb_latency, 
						 &(OP_DEST0(op)), &__PD_statistics->ops[OP_BBIDX(op)]);
	  }
	  if (!ce || (ce->tick < tlb_ce->tick)) ce = tlb_ce;
	}
    }
    
    /* probe L1 data cache */
    l1_latency = d4probe_cache(dinero_L1_DCACHE, m);
    
#ifdef EPIC_EXPLORER
//Data cache address bus is accessed 
  total_L1D_refs++;
  current_L1D_ref = m.address;
  total_L1D_hamming += hamming_distance(previous_L1D_ref,current_L1D_ref);
  previous_L1D_ref = current_L1D_ref;
#endif
    if (!l1_latency) {
#ifdef __PD_DEBUG
	fprintf(stderr, "\thit DCACHE\n");
#endif /* __PD_DEBUG */
	d4ref(dinero_L1_DCACHE, m);
    }
    /* missed L1 dcache (either aligned and missed or misaligned and
     * part/all missed
     */
    else {
	miss = __PD_true;
#ifdef __PD_DEBUG
	fprintf(stderr, "\tmissed DCACHE (latency %d)\n", l1_latency);
#endif /* __PD_DEBUG */

#ifdef __PD_TRACK_OP_STATS_
	if (__PD_parameters->flags & __PD_DYNSTATS) {
	  __PD_assert(!__PD_statistics->is_procedure);
	  __PD_statistics->ops[OP_BBIDX(op)].l1d_misses++;
	}
#endif /* __PD_TRACK_OP_STATS_ */
	
	/* check oustanding  cache events  for an overlapping  request; if
	 * there  exists such  an event,  use  its cache  event entry  for
	 * bookeeping
	 */
	for (cep = cache_event_q.head; cep != NULL; cep = cep->next) {
	  if (cep->cache != dinero_L1_DCACHE) continue;
	  /* found an overlapping request */
	  if (d4same_blockaddr(dinero_L1_DCACHE, &m, &(cep->memref))) {
	    l1_ce      = cep;
	    l1_latency = cep->tick;
	    overlapping_miss = __PD_true;
#ifdef __PD_DEBUG
	    fprintf(stderr, "\t\tfound overlapping miss (latency %d)\n", l1_latency);
#endif /* __PD_DEBUG */  
	    break;
	  }
	}

	/* an  outstanding  request  for  the same  address  exists;  not
	 * necessary to queue additional requests 
	 */
	if (l1_ce) {
	  /* if the latency to fill L1 is greater than the TLB fill lantecy
	   * then use  the L1 latency for marking  a destination registers
	   * unavailable
	   */
	  if (!ce || (ce->tick < l1_ce->tick)) ce = l1_ce;
	}
	else {
	  /* probe L2 */
	  l2_latency = d4probe_cache(dinero_UCACHE, m);
	  
	  /* missed L2 cache */
	  if (l2_latency) {
#ifdef __PD_DEBUG
	    fprintf(stderr, "\tmissed UCACHE (latency %d)\n", l2_latency);
#endif /* __PD_DEBUG */
	    
#ifdef __PD_TRACK_OP_STATS_
	    if (__PD_parameters->flags & __PD_DYNSTATS) {
		__PD_assert(!__PD_statistics->is_procedure);
		__PD_statistics->ops[OP_BBIDX(op)].l2d_misses++;
	    }
#endif /* __PD_TRACK_OP_STATS_ */

	    /* check oustanding  cache events  for an overlapping  request; if
	     * there  exists such  an event,  use  its cache  event entry  for
	     * bookeeping
	     */
	    for (cep = cache_event_q.head; cep != NULL; cep = cep->next) {
		if (cep->cache != dinero_L1_DCACHE) continue;
		/* found an overlapping request */
		if (d4same_blockaddr(dinero_UCACHE, &m, &(cep->memref))) {
		  l2_ce = cep;
		  overlapping_miss = __PD_true;
#ifdef __PD_DEBUG
		  fprintf(stderr, "\t\tfound overlapping miss (latency %d)\n", l2_ce->tick);
#endif /* __PD_DEBUG */ 
		}
	    }
	    
	    if (!l2_ce) {
		l2_ce = enque_cache_event(&cache_event_q, dinero_L1_DCACHE, m, l2_latency, 
						  &(OP_DEST0(op)), &__PD_statistics->ops[OP_BBIDX(op)]);
	    }
	    if (!ce || (ce->tick < l2_latency)) ce = l2_ce;
	  }
	  /* missed L1 but hit L2 */
	  else {
	    l1_ce = enque_cache_event(&cache_event_q, dinero_L1_DCACHE, m, l1_latency, 
						&(OP_DEST0(op)), &__PD_statistics->ops[OP_BBIDX(op)]);
	    
	    if (!ce || (ce->tick < l1_latency)) ce = l1_ce;
	  }
	}
    }
    
    /* wait on DTLB or DCACHE by marking destination register oustanding (pending) */
    if (miss) {
	__PD_assert(ce);
	
	if (overlapping_miss) {
	  port = PD_PORT_NODE_malloc();
	  port->next = ce->port_list;
	  ce->port_list = port;
	  // RMR // memcpy(&(port->reg), &(OP_DEST0(op)), sizeof(struct __PD_port));
	  port->reg = &REG(OP_DEST0(op));
	  __PD_assert(port->reg);
	}
	else {
	  bool found = __PD_false;
	  for (port = ce->port_list; port != NULL; port = port->next) {
	    __PD_assert(port->reg);
	    found |= (port->reg == &REG(OP_DEST0(op)));
	  }
	  __PD_assert(found);
	}
	
#ifdef __PD_DEBUG
	fprintf(stderr, "\top %lu has outsanding request on register %u (latency %d)\n", 
		  OP_ID(op), OP_DEST0(op).num, ce->tick);
#endif /* __PD_DEBUG */

	/* sanity check assumption that the cache even that handles this
	 * register will have a tick that is greater than the current event
	 * tagged to the register
	 */
	if (miss && any_misses) {
	  __PD_assert(REG(OP_DEST0(op)).pending);
	  __PD_assert(REG(OP_DEST0(op)).user_handler);
	  __PD_assert(((struct CACHE_EVENT*)(REG(OP_DEST0(op)).user_handler))->tick <= ce->tick);
	}

	/* for return value */
	any_misses++;

	REG(OP_DEST0(op)).pending = __PD_DINERO_PENDING;
	REG(OP_DEST0(op)).user_handler = (void*) ce;
    }
    else {
	REG(OP_DEST0(op)).pending = __PD_REG_AVAILABLE;
	REG(OP_DEST0(op)).user_handler = NULL;
    }
  } 

  return (any_misses > 0);
}


void __PD_dinero_check_src_regs(const __PD_OP* op)
{
  if (PERFECT_MEMH) return;

  /* check  the pending  bit on  a register;  since this  is  a normal
   * operation and only the right most bit should to be set if data is
   * outstanding
   */
  if (IS_REG(OP_PRED(op)) && REG(OP_PRED(op)).pending) {
    __PD_assert((REG(OP_PRED(op)).pending ^ __PD_DINERO_PENDING) == 0);
    check_operand(op, &OP_PRED(op));
  }
  if (IS_REG(OP_SRC0(op)) && REG(OP_SRC0(op)).pending) {
    __PD_assert((REG(OP_SRC0(op)).pending ^ __PD_DINERO_PENDING) == 0);
    check_operand(op, &OP_SRC0(op));
  }
  if (IS_REG(OP_SRC1(op)) && REG(OP_SRC1(op)).pending) {
    __PD_assert((REG(OP_SRC1(op)).pending ^ __PD_DINERO_PENDING) == 0);
    check_operand(op, &OP_SRC1(op));
  }
  if (IS_REG(OP_SRC2(op)) && REG(OP_SRC2(op)).pending) {
    __PD_assert((REG(OP_SRC2(op)).pending ^ __PD_DINERO_PENDING) == 0);
    check_operand(op, &OP_SRC2(op));
  }
  if (IS_REG(OP_SRC3(op)) && REG(OP_SRC3(op)).pending) {
    __PD_assert((REG(OP_SRC3(op)).pending ^ __PD_DINERO_PENDING) == 0);
    check_operand(op, &OP_SRC3(op));
  }
}


/* this is to be called from __PD_aclock just AFTER __PD_global_clock
 * is incremented; updating the cache state will:
 * - perform cache events that are due
 * - retire write buffers
 * - do not touch __PD_global_clock (should be updated in caller)
 */
void __PD_dinero_advance_state_one_step()
{
  if (!PERFECT_MEMH) {
    advance_cache_state_one_step(__PD_false);
  }
}


/******************************************************************************
 * memory hierarchy initializer
 *****************************************************************************/

#ifdef EPIC_EXPLORER
/********************************************************
  Added by Davide Patti for EpicExplorer
  Load a cache configuration from file 

NOTE: currently only exploration parameters are set here. The other
are set in __PD_dinero_default_memh(...)
 *********************************************************/
void __EE_load_cache_config(d4cache* UC,d4cache* IC, d4cache* DC)
{
    int a;
    FILE * fp;
    char s[256];
    char filename[256];

    strcpy(filename,getenv("HOME"));

    strcat(filename,CACHE_FILE);

    
    if ((fp=fopen(filename,"r"))==NULL)
    {
	printf("\n FATAL ERROR: Can't open cache config file ");
	exit(1);
    }

    while (fscanf(fp,"%s",s)!=EOF)
    {

	// L2 Unified cache
	if ( !strcmp("L2U_blocksize",s))
	{
	    fscanf(fp,"%d",&a);
	    UC->lg2blocksize    = clog2(a);
	}
	else if (!strcmp("L2U_subblocksize",s))
	{
	    fscanf(fp,"%d",&a);
	    UC->lg2subblocksize = clog2(a);
	}
	else if (!strcmp("L2U_size",s))
	{
	    fscanf(fp,"%d",&a);
	    UC->lg2size         = clog2(a);
	}
	else if (!strcmp("L2U_associativity",s))
	{
	    fscanf(fp,"%d",&a);
	    UC->assoc           = a;
	}


	// L1 Instruction cache 
	//
	else if (!strcmp("L1I_blocksize",s))
	{
	    fscanf(fp,"%d",&a);
	    IC->lg2blocksize    = clog2(a);
	}
	else if (!strcmp("L1I_subblocksize",s))
	{
	    fscanf(fp,"%d",&a);
	    IC->lg2subblocksize = clog2(a);
	}
	else if (!strcmp("L1I_size",s))
	{
	    fscanf(fp,"%d",&a);
	    IC->lg2size         = clog2(a);
	}
	else if (!strcmp("L1I_associativity",s))
	{
	    fscanf(fp,"%d",&a);
	    IC->assoc           = a;
	}
	// L1 Data cache 
	else if (!strcmp("L1D_blocksize",s))
	{
	    fscanf(fp,"%d",&a);
	    DC->lg2blocksize    = clog2(a);
	}
	else if (!strcmp("L1D_subblocksize",s))
	{
	    fscanf(fp,"%d",&a);
	    DC->lg2subblocksize = clog2(a);
	}
	else if (!strcmp("L1D_size",s))
	{
	    fscanf(fp,"%d",&a);
	    DC->lg2size         = clog2(a);
	}
	else if (!strcmp("L1D_associativity",s))
	{
	    fscanf(fp,"%d",&a);
	    DC->assoc           = a;
	}

    }
    fclose(fp);
}
#endif /*EPIC_EXPLORER*/

/* create and configure a default memory hierarchy */
static void dinero_default_memh(d4cache* memh[DINERO_CACHETY][DINERO_MAX_LEV], 
					  int* maxlevel)
{
  d4cache* mem  = d4new(NULL);
  d4cache* uc   = d4new(mem);
  d4cache* ic   = d4new(uc);
  d4cache* dc   = d4new(uc);
  d4cache* dtlb = d4new(mem);
  d4cache* itlb = d4new(mem);
  d4cache* btb  = d4new(mem);


  /* configure memory */
  mem->name              = strdup("memory");

  /* configure unified cache */
  uc->name               = strdup("l2-ucache");
  uc->flags             |= D4F_CCC;

#ifdef EPIC_EXPLORER
  __EE_load_cache_config(uc,ic,dc);
#endif
#ifndef EPIC_EXPLORER
  uc->lg2blocksize       = clog2(64);
  uc->lg2subblocksize    = clog2(64);
  uc->lg2size            = clog2(1 << 20);
  uc->assoc              = 8;
#endif
  uc->latency            = 100;

  uc->replacementf       = d4rep_lru; 
  uc->name_replacement   = "LRU";

  uc->prefetchf          = d4prefetch_none;
  uc->name_prefetch      = "demand only";

  uc->wallocf            = d4walloc_always;
  uc->name_walloc        = "always";

  uc->wbackf             = d4wback_always;
  uc->name_wback         = "always";

  /* configure instruction cache */
  ic->name               = strdup("l1-icache");
  ic->flags             |= D4F_CCC;
  ic->flags             |= D4F_RO;

#ifndef EPIC_EXPLORER
  ic->lg2blocksize       = clog2(64);
  ic->lg2subblocksize    = clog2(64);
  ic->lg2size            = clog2(32 << 10);
  ic->assoc              = 4;
#endif
  ic->latency            = 7;

  ic->replacementf       = d4rep_lru; 
  ic->name_replacement   = "LRU";

  ic->prefetchf          = d4prefetch_always;
  ic->name_prefetch      = "always";

  ic->wallocf            = d4walloc_impossible;
  ic->name_walloc        = "impossible";

  ic->wbackf             = d4wback_impossible;
  ic->name_wback         = "impossible";

  ic->prefetch_distance     = ic->lg2subblocksize;
  ic->prefetch_abortpercent = 50;

  /* configure data cache */
  dc->name               = strdup("l1-dcache");
  dc->flags             |= D4F_CCC;
  dc->latency            = 10;

#ifndef EPIC_EXPLORER
  dc->lg2blocksize       = clog2(32);
  dc->lg2subblocksize    = clog2(32);
  dc->lg2size            = clog2(32 << 10);
  dc->assoc              = 4;
#endif

  dc->replacementf       = d4rep_lru; 
  dc->name_replacement   = "LRU";

  dc->prefetchf          = d4prefetch_none;
  dc->name_prefetch      = "demand only";

  dc->wallocf            = d4walloc_never;
  dc->name_walloc        = "never";

  dc->wbackf             = d4wback_always;
  dc->name_wback         = "always";

  /* configure itlb and dtlb; both are fully associative */
  itlb->name             = strdup("itlb");
  itlb->flags           |= D4F_CCC;
  itlb->latency          = 8;

  itlb->lg2subblocksize  = clog2(32);
  itlb->lg2blocksize     = clog2(32);
  itlb->lg2size          = clog2(8 << 10);
  itlb->assoc            = 1 << (itlb->lg2size - itlb->lg2blocksize);

  itlb->replacementf     = d4rep_lru; 
  itlb->name_replacement = "LRU";

  itlb->prefetchf        = d4prefetch_none;
  itlb->name_prefetch    = "demand only";

  itlb->wallocf          = d4walloc_always;
  itlb->name_walloc      = "always";

  itlb->wbackf           = d4wback_always;
  itlb->name_wback       = "always";

  /* dtlb */
  dtlb->name             = strdup("dtlb");
  dtlb->flags           |= D4F_CCC;
  dtlb->latency          = 8;

  dtlb->lg2subblocksize  = clog2(32);
  dtlb->lg2blocksize     = clog2(32);
  dtlb->lg2size          = clog2(8 << 10);
  dtlb->assoc            = 1 << (dtlb->lg2size - dtlb->lg2blocksize);

  dtlb->replacementf     = d4rep_lru; 
  dtlb->name_replacement = "LRU";

  dtlb->prefetchf        = d4prefetch_none;
  dtlb->name_prefetch    = "demand only";

  dtlb->wallocf          = d4walloc_always;
  dtlb->name_walloc      = "always";

  dtlb->wbackf           = d4wback_always;
  dtlb->name_wback       = "always";

  /* configure BTB - 8K 4 way assoc */
  btb->name               = strdup("BTB");
  btb->flags             |= D4F_CCC;
  btb->latency            = 3;

  btb->lg2blocksize       = clog2(4);
  btb->lg2subblocksize    = clog2(4);
  btb->lg2size            = clog2(8 << 10);
  btb->assoc              = 4;

  btb->replacementf       = d4rep_lru; 
  btb->name_replacement   = "LRU";

  btb->prefetchf          = d4prefetch_none;
  btb->name_prefetch      = "demand only";

  btb->wallocf            = d4walloc_always;
  btb->name_walloc        = "always";

  btb->wbackf             = d4wback_always;
  btb->name_wback         = "always";

  /* complete memory hierarchy configuration */
  memh[DINERO_UCACHE][1] = uc;
  memh[DINERO_ICACHE][0] = ic;
  memh[DINERO_DCACHE][0] = dc;
  dinero_ITLB = itlb;
  dinero_DTLB = dtlb;
  dinero_BTB  = btb;
  *maxlevel   = 2;

  /* write buffers */
  available_write_buffers = WRITE_BUFFER_SLOTS;
}


/******************************************************************************
 * dinero simulation helpers
 *****************************************************************************/


static void check_operand(const __PD_OP* op, const struct __PD_port* pp)
{
  struct CACHE_EVENT* cp;
  struct __PD_port p = *pp;
  uint stalls = 0;
  
  /* find the corresponding cache event */
  cp = (struct CACHE_EVENT*) (REG(p).user_handler);

  __PD_assert(REG(p).pending);
  __PD_assert((REG(p).pending ^ __PD_DINERO_PENDING) == 0);
  __PD_assert(cp);
  __PD_assert(cp->tick >= 0);

#ifdef __PD_TRACK_OP_STATS_
  /* advance_cache_state deallocates the current event pointer */
  __PD_opstats* defining_op = cp->def_op;
#endif /* __PD_TRACK_OP_STATS_ */

#ifdef __PD_DEBUG
  int printed = 0;
#endif /* __PD_DEBUG */
  do {
    /* advance state will free the cache event; hence save the stall cycles */
    /* cannot guarantee the  register will  be free  after cp->tick
     * cycles because  the number of memory units  is finite; do one
     * cycle at a time */
#ifdef __PD_DEBUG
    if (!printed) {
	printed = 1;
	fprintf(stderr, "[op %lu use stall on register %u]\n", OP_ID(op), p.num);
    }
#endif /* __PD_DEBUG */
	
    advance_cache_state_one_step(__PD_true);
    stalls += ONE_STEP;
  } while (REG(p).pending);
  dinero_use_stalls += stalls;

#ifdef __PD_DEBUG
  fprintf(stderr, "\t\top %lu total use stall (latency %d) on register %u\n", 
	    OP_ID(op), stalls, p.num);
#endif /* __PD_DEBUG */

  if (__PD_parameters->flags & __PD_DYNSTATS) {
    __PD_assert(!__PD_statistics->is_procedure);
    __PD_statistics->stall_cycles  += stalls;
    __PD_statistics->use_stalls    += stalls;
#ifdef __PD_TRACK_OP_STATS_
    __PD_statistics->ops[OP_BBIDX(op)].taken_use_stalls += stalls;
    /* charge the defining op */
    __PD_assert(defining_op);
    defining_op->caused_use_stalls += stalls;
#endif /* __PD_TRACK_OP_STATS_ */
  }

  /* sanity check */
  __PD_assert(REG(p).pending == __PD_REG_AVAILABLE);
  __PD_assert(REG(p).user_handler == NULL);
}


static struct CACHE_EVENT* enque_cache_event(struct QUE* q, d4cache* cache, d4memref memref, int tick,
							   const struct __PD_port* reg, __PD_opstats* def_op)
{
  struct CACHE_EVENT* p;
  struct PD_PORT_NODE* port;

  p = CACHE_EVENT_malloc();
  p->cache     = cache;
  p->memref    = memref;
  p->timestamp = __PD_global_clock;
  p->tick      = tick;
  p->port_list = NULL;
  p->def_op    = NULL;
  p->next      = NULL;
  
  /* write buffer has no register port associated */
  if (reg) {
    __PD_assert(reg->file);

    port = PD_PORT_NODE_malloc();
    port->next = NULL;
    // RMR // memcpy(&(port->reg), reg, sizeof(struct __PD_port));
    port->reg = &(REG((*reg)));

    p->port_list = port;

    __PD_assert(def_op);
    p->def_op = def_op;
  }

  if (q->head == NULL) {
    q->tail = q->head = p;
    p->prev = NULL;
  }
  else {
    q->tail->next = p;
    p->prev = q->tail;
    q->tail = p;
  }
  
  return p;
}


static void advance_cache_state_one_step(bool stall)
{
  struct CACHE_EVENT  *cur_p, *prev_p, *fp;
  struct PD_PORT_NODE *port,  *fpp;
  int num_of_mem_unit_used = 0;
  
  /* incrememnt clock on stalls */
  __PD_assert(stall == 0 || stall == ONE_STEP); 
  if (stall) __PD_global_clock++;

  /* decrement tick in write buffer and retire all those that are
   * ready (tick == 0)
   */
  prev_p = NULL;

  for (cur_p = write_buffer.head; cur_p != NULL; ) {
    if (cur_p->tick == 0) {
	/* check if in  this cycle already used up  all available memory
	 * units; assume that the  memory units are pipelined, i.e., can
	 * complete NUM_MEM_UNITS operations per cycle
	 */
	if (num_of_mem_unit_used >= NUM_MEM_UNITS) {
	  cur_p = cur_p->next;
	  /* wait till next cycle */
	  continue; 
	} 
	else {
	  /* reserve memory unit */
	  num_of_mem_unit_used++;
	}

	/* dequeue event now that it has been serviced */
	prev_p = cur_p->prev;
	
	if (cur_p == write_buffer.tail) {
	  write_buffer.tail = prev_p;
	}
	
	if (cur_p == write_buffer.head) {
	  write_buffer.head = cur_p->next;
	} 
	
	if (prev_p) {
	  prev_p->next = cur_p->next;
	}
	
	if (cur_p->next) {
	  cur_p->next->prev = prev_p;
	}

	available_write_buffers++;
	fp = cur_p;
	cur_p = cur_p->next;

	CACHE_EVENT_free(fp);
    } 
    else {
	cur_p->tick--;
	cur_p = cur_p->next;
    }
  }

  /* FIXME: must check if there is an entry in the write buffer. 6/04.
   * HUH? WHY? 5/05.
   */

  /* decrement tick in cache event  queue and perform all those events
   * that hit zero 
   */
  for (cur_p = cache_event_q.head; cur_p != NULL; ) {
    if (cur_p->tick == 0) {
	/* check if in this cycle already used up all available units */
	if (num_of_mem_unit_used >= NUM_MEM_UNITS) {
	  cur_p = cur_p->next;
	  /* wait till next cycle */
	  continue;
	} 
	else {
	  /* reserve memory unit */
	  num_of_mem_unit_used++;
	}
	
	d4memref m;
	m.address = cur_p->memref.address;
	m.accesstype = cur_p->memref.accesstype;
	m.size = cur_p->memref.size;

	/* do it */
#ifdef __PD_DEBUG
	fprintf(stderr, "\t\tupdating %s for address %llu...", cur_p->cache->name, m.address);
#endif /* __PD_DEBUG */
	__PD_assert(d4probe_cache(cur_p->cache, m));
	d4ref(cur_p->cache, m);
	__PD_assert(d4probe_cache(cur_p->cache, m) == 0);
#ifdef __PD_DEBUG
	fprintf(stderr, "done\n");
#endif /* __PD_DEBUG */

	/* dequeue cache event */
	prev_p = cur_p->prev;
	
	if (cur_p == cache_event_q.tail) {
	  cache_event_q.tail = prev_p;
	}

	if (cur_p == cache_event_q.head) {
	  cache_event_q.head = cur_p->next;
	}
	
	if (prev_p) {
	  prev_p->next = cur_p->next;
	}

	if (cur_p->next) {
	  cur_p->next->prev = prev_p;
	}

	/* deallocate the ports */
	for (port = cur_p->port_list; port != NULL; ) {
	  if (port->reg->pending) {
	    /* sanity check */
	    if (!(port->reg->pending & __PD_DINERO_PENDING)) {
		/* immagine  this scenario: normal  program operation writes
		 * to a  register through a load, branch to  a spear op that
		 * is  canceled  and  has the  same  destination  register,
		 * eventually  the miss is  serviced; seems like  a register
		 * allocator  bug to me  (164.gzip has this  sceneario, will
		 * look again) 
		 */
		__PD_assert(port->reg->pending & __PD_SPEAR_PENDING);
		__PD_assert(port->reg->user_handler == NULL);
	    }

	    /* search the registers that are registered to this cache event
	     * and if they are still waiting for the event to be serviced
	     * mark them as serviced (it is possible that the register
	     * is now waiting on another event: it may have been redefined
	     * already) 
	     */
	    __PD_assert(port->reg->user_handler);
	    if (port->reg->user_handler == cur_p) {
		port->reg->pending = __PD_REG_AVAILABLE;
		port->reg->user_handler = NULL;
	    }
	  }
	  else {
	    __PD_assert(port->reg->user_handler == NULL);
	  }

	  /* remove this register from the waiting list attached to this
	   * event 
	   */
	  fpp  = port;
	  port = port->next;
	  PD_PORT_NODE_free(fpp);
	}

	/* delete this cache event record */
	cur_p->port_list = NULL;

	fp    = cur_p;
	cur_p = cur_p->next;

	CACHE_EVENT_free(fp);
    }
    else {
	/* the cache request is still oustanding; adjust the clock to
	 * reflect the advance in the machine state: this event is one
	 * cycle closer to being services
	 */
	cur_p->tick--;
	cur_p = cur_p->next;
    }
  }
}


/******************************************************************************
 * branch predictor helpers
 *****************************************************************************/


static int gshare_predict(uint pc)
{
  int addr;
  
  addr = ((pc / __PD_parameters->bytes_per_op) ^ GR) % SIZE_OF_BRANCH_PREDICTOR_TBL;

  return (gshare_branch_predictor_tbl[addr] > 1);
}


static int bimode_predict(uint pc)
{
  int addr;
  
  addr = (pc / __PD_parameters->bytes_per_op) % SIZE_OF_BRANCH_PREDICTOR_TBL;

  return (bimode_branch_predictor_tbl[addr] > 1);
}


static int branch_predict(uint pc)
{
  int addr;

  cur_gpred = gshare_predict(pc);
  cur_bpred = bimode_predict(pc);
  
  addr = (pc / __PD_parameters->bytes_per_op) % SIZE_OF_BRANCH_PREDICTOR_TBL;

  if (which_branch_predictor_tbl[addr] > 1)
     return cur_gpred;
  else
     return cur_bpred;
}


static void gshare_br_confirm(uint pc, int outcome)
{
  int addr;
  
  addr = ((pc / __PD_parameters->bytes_per_op) ^ GR) % SIZE_OF_BRANCH_PREDICTOR_TBL;
  
  /* branch is taken - update counter */
  if (outcome) {
    if (gshare_branch_predictor_tbl[addr] < 3) {
	gshare_branch_predictor_tbl[addr]++;
    }
    
    GR = GR << 1;
    GR = GR | 1;
    GR = GR & 0x3FF; /* keep GR to 10 bits */
  } 
  else {
    if (gshare_branch_predictor_tbl[addr] > 0) {
	gshare_branch_predictor_tbl[addr]--;
    }
    GR = GR << 1;
    GR = GR & 0x3FF; /* keep GR to 10 bits */
  }
}


static void bimode_br_confirm(uint pc, int outcome)
{
  int addr;
  
  addr = (pc / __PD_parameters->bytes_per_op) % SIZE_OF_BRANCH_PREDICTOR_TBL;
  
  /* branch is taken - update counter */
  if (outcome) {
    if (bimode_branch_predictor_tbl[addr] < 3) {
	bimode_branch_predictor_tbl[addr]++;
    }
  } 
  else {
    if (bimode_branch_predictor_tbl[addr] > 0) {
	bimode_branch_predictor_tbl[addr]--;
    }
  }
}


static void branch_confirm(uint pc, int outcome)
{
  int addr;
  
  /* Update both */
  bimode_br_confirm(pc, outcome);
  gshare_br_confirm(pc, outcome);

  addr = (pc / __PD_parameters->bytes_per_op) % SIZE_OF_BRANCH_PREDICTOR_TBL;
  
  /* gshare got it right */
  if (outcome == cur_gpred) {
    if (which_branch_predictor_tbl[addr] < 3) {
	which_branch_predictor_tbl[addr]++;
    }
  } 

  /* bimodal got it right */
  if (outcome == cur_bpred) {
    if (which_branch_predictor_tbl[addr] > 0) {
	which_branch_predictor_tbl[addr]--;
    }
  }
}


/******************************************************************************
 * custom memory allocation 
 *****************************************************************************/


static void init_custom_memory_pools()
{
  int i;

  CACHE_EVENT_pool_head    = &(CACHE_EVENT_pool[0]);
  CACHE_EVENT_pool[0].prev = NULL;
  CACHE_EVENT_pool[0].next = &(CACHE_EVENT_pool[1]);
  for (i = 1; i < POOL_SIZE-1; i++) {
    CACHE_EVENT_pool[i].prev = &(CACHE_EVENT_pool[i-1]);
    CACHE_EVENT_pool[i].next = &(CACHE_EVENT_pool[i+1]);
  }
  CACHE_EVENT_pool[i].prev = &(CACHE_EVENT_pool[i-1]);
  CACHE_EVENT_pool[i].next = NULL;

  PORT_NODE_pool_head = &(PORT_NODE_pool[0]);
  for (i = 0; i < POOL_SIZE-1; i++) {
    PORT_NODE_pool[i].next = &(PORT_NODE_pool[i+1]);
  }
  PORT_NODE_pool[i].next = NULL;
}


static struct CACHE_EVENT* CACHE_EVENT_malloc()
{
  struct CACHE_EVENT* p = CACHE_EVENT_pool_head;
  
  if (p == NULL) {
    __PD_punt("Exhausted custom memory pool for cache simulation");
  }
  
  CACHE_EVENT_pool_head = CACHE_EVENT_pool_head->next;
  return p;
}


static void CACHE_EVENT_free(struct CACHE_EVENT* p)
{
  /* enque node in the free pool */
  p->next = CACHE_EVENT_pool_head;
  p->prev = NULL;
  CACHE_EVENT_pool_head = p;
}


static struct PD_PORT_NODE* PD_PORT_NODE_malloc()
{
  struct PD_PORT_NODE* p = PORT_NODE_pool_head;

  if (p == NULL) {
    __PD_punt("Exhausted custom memory pool for cache simulation");
  }
  
  PORT_NODE_pool_head = PORT_NODE_pool_head->next;
  return p;
}


static void PD_PORT_NODE_free(struct PD_PORT_NODE* p)
{
  /* enque node in the free pool */
  p->next = PORT_NODE_pool_head;
  PORT_NODE_pool_head = p;
}


/******************************************************************************
 * utilities
 *****************************************************************************/


inline static int width_to_numbytes(__PD_width w)
{
  switch (w) {
  case __PD_B: return 1;
  case __PD_H: return 2;
  case __PD_W: return 4;
  case __PD_S: return 4;
  case __PD_D: return 8;
  }
  
  __PD_punt("Unrecognized width specification!");
  return 0;
}


inline static int clog2(unsigned int c)
{
  int i;

  for (i = -1; c != 0; i++) c >>= 1;

  return i;
}

#endif /* __PD_SSDINERO_ */
