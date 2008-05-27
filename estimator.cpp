/***************************************************************************
                          estimator.cpp  -  description
                             -------------------
    begin                : sab nov 9 2002
    copyright            : (C) 2002 by Davide Patti
    email                : dpatti@diit.unict.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "estimator.h"
#include "common.h"

#include <cmath>

// uncomment the line below to get delailed energy stats
//#define VERBOSE

// uncomment to switch to the old-style of computing memory stalls 
// (used with trimaran 3)
//#define OLD_STYLE_STALLS

Estimator::Estimator(){

    set_autoclock(false);
}
Estimator::~Estimator(){
}
/*******************************************************************************
 *                             get_functional_unit_energy 
 *******************************************************************************
 *    compute the total energy dissipation of all units of a single type
 *    The total contribute consists of two terms :
 *    activity energy dissipation + inactivity energy dissipation
 ******************************************************************************/


double Estimator::get_functional_unit_energy( uint64 total_cycles,
					     uint64 total_unit_operations,
					     const Area& area,
					     int unit_instancies) 
{

    // active power costs of a single unit, in W
    double active_power_cost = area.static_area * STATIC_PDA + 
	                       area.dynamic_area * DYNAMIC_PDA +
			       area.clock_area * CLOCK_PDA +
		               area.memory_area * MEMORY_PDA +
		               area.pla_area * PLA_PDA ;


    // inactive power costs of a single unit, in W
    double inactive_power_cost  = area.static_area * STATIC_PDS + 
				  area.dynamic_area * DYNAMIC_PDS +
				  area.clock_area * CLOCK_PDS +
				  area.memory_area * MEMORY_PDS +
				  area.pla_area * PLA_PDS ;

    // power densities have been computed at 2Ghz, so they must be
    // scaled 
    active_power_cost = active_power_cost/current_power_density_scale;
    inactive_power_cost = inactive_power_cost/current_power_density_scale;

    // overall energy consumption if all units of a considered type
    // were inactive in every cycle ( Joules)
    double total_inactive_energy = unit_instancies * inactive_power_cost * total_cycles * current_clock_T;
    
    // but sometimes they have been active, so we must subtract
    // an inactive energy contribution for each operation
    // executed by the unit(s)

    total_inactive_energy = total_inactive_energy - inactive_power_cost * total_unit_operations * current_clock_T;

    // now , let's consider the active energy consumption
    //
    double total_active_energy = total_unit_operations * active_power_cost * current_clock_T;
    double total_unit_energy = total_inactive_energy + total_active_energy;
    
    return total_unit_energy;
}


// compute inactive power per cycle for a particular register size

double Estimator::get_registerfile_standbypower(int words,int word_size)
{
    return  ( (7.99611e-5*words*word_size + 1.82395e-4*words - 2.6431e-4*word_size)*exp((-2108.95522)/(TMPR-82.60784)) )/current_power_density_scale;
}

double Estimator::get_registerfile_dynamic_power(int words,int word_size)
{
    return (4.78619e-7*words+2.5116626e-5*word_size)/current_power_density_scale;
}
// compute active power per cycle for a particular register size

double Estimator::get_registerfile_active_power(int words,int word_size)
{
    double ps = get_registerfile_standbypower(words,word_size);
    double pd = get_registerfile_dynamic_power(words,word_size);

    return ps+pd;
}


double Estimator::get_processor_energy(const Dynamic_stats& dyn_stats,
	                               const Processor& processor, 
				       const Mem_hierarchy& mem)
{
    double energy=0;

    uint64 total_cycles;

#ifdef OLD_STYLE_STALLS
 	total_cycles = get_execution_cycles(dyn_stats,mem);
#else
	total_cycles = dyn_stats.total_cycles;
#endif

    uint64 ialu_ops = dyn_stats.ialu;
    uint64 falu_ops =dyn_stats.falu;
    uint64 mem_ops = (dyn_stats.load+dyn_stats.store);
    uint64 branch_ops = dyn_stats.branch;
    uint64 cmp_ops = dyn_stats.cmp;
    uint64 pbr_ops = dyn_stats.pbr;
    uint64 issued_ops = dyn_stats.total_dynamic_operations;
    uint64 regfile_ops = issued_ops * 2;
	              
    int n_units; // number of istancies of a functional unit

    double int_units_energy;
    double float_units_energy;
    double mem_units_energy;
    double branch_units_energy;
    double decode_unit_energy;
    double gpr_register_file_energy; // general purpose
    double fp_register_file_energy;  // floating point
    double bt_register_file_energy;  // branch target 
    double pr_register_file_energy;  // predicate 
    double cr_register_file_energy;  // control 


    Area area_ialu;
    Area area_falu;
    Area area_decode;

    area_ialu.static_area = IALU_STATIC_AREA;
    area_ialu.dynamic_area = IALU_DYNAMIC_AREA;
    area_ialu.clock_area = IALU_CLOCK_AREA;
    area_ialu.memory_area = IALU_MEMORY_AREA;
    area_ialu.pla_area = IALU_PLA_AREA;

    area_falu.static_area = FALU_STATIC_AREA;
    area_falu.dynamic_area = FALU_DYNAMIC_AREA;
    area_falu.clock_area = FALU_CLOCK_AREA;
    area_falu.memory_area = FALU_MEMORY_AREA;
    area_falu.pla_area = FALU_PLA_AREA;

    area_decode.static_area = DECODE_STATIC_AREA;
    area_decode.dynamic_area = DECODE_DYNAMIC_AREA;
    area_decode.clock_area = DECODE_CLOCK_AREA;
    area_decode.memory_area = DECODE_MEMORY_AREA;
    area_decode.pla_area = DECODE_PLA_AREA;

   int num_clusters = processor.num_clusters.get_val();

    n_units = num_clusters * processor.integer_units.get_val(); 
    int_units_energy = get_functional_unit_energy( total_cycles, ialu_ops,area_ialu, n_units);

    n_units = num_clusters * processor.float_units.get_val();
    float_units_energy = get_functional_unit_energy( total_cycles, falu_ops,area_falu, n_units);

    // Each cmp (compare to predicate ) op uses a branch unit to
    // evaluate branch condition.

    n_units = num_clusters * processor.branch_units.get_val();
    branch_units_energy = get_functional_unit_energy( total_cycles, cmp_ops, area_ialu, n_units);

    //each load/store op uses a memory unit

    n_units = num_clusters * processor.memory_units.get_val();
    mem_units_energy = get_functional_unit_energy ( total_cycles, mem_ops, area_ialu, n_units);


    decode_unit_energy = get_functional_unit_energy( total_cycles, issued_ops, area_decode, ISSUE_WIDTH );

    // REGISTER FILES ENERGY ///////////////////////////////////////
    //
    // for each type of register file is computed the energy cost ,
    // that dependes on :
    // - number of register file accessess ;
    // - register file size (words);
    // - register file width 

    int words;

    /////////////// general purpose register file ///////////////////
    //
    //NOTE: rotating size is assumed = static size

    words = processor.gpr_static_size.get_val() + processor.gpr_static_size.get_val();

    double gpr_ps = get_registerfile_standbypower(words,32);
    double gpr_pd = get_registerfile_dynamic_power(words,32);

    gpr_register_file_energy = gpr_ps*total_cycles*current_clock_T + gpr_pd * regfile_ops*current_clock_T;


    //////////////////////////////////////////////////////////////////

    /////////////// floating point register file /////////////////////
    //

    words = processor.fpr_static_size.get_val() + processor.fpr_static_size.get_val();

    double fp_ps = get_registerfile_standbypower(words,64);
    double fp_pd = get_registerfile_dynamic_power(words,64);
    fp_register_file_energy = (fp_ps*total_cycles + fp_pd * falu_ops)*current_clock_T;


    //////////////////////////////////////////////////////////////////

    //////////// branch target register file  ///////////////////////
    //
    // Each pbr ( prepare to branch) op put some info about possible
    // branch into a branch target register 
    // ....and each branch operation read from a btr the branch
    // destination 

    words = processor.btr_static_size.get_val();

    double bt_register_file_ps = get_registerfile_standbypower(words,64);
    double bt_register_file_pd = get_registerfile_dynamic_power(words,64);
    double bt_register_file_power = bt_register_file_ps * total_cycles + bt_register_file_pd * (pbr_ops+branch_ops);

    bt_register_file_energy = bt_register_file_power * current_clock_T;

    //////////////////////////////////////////////////////////////////

    ///////////// predicate register file ////////////////////////////
    //
    // Each compare to predicate op write to a predicate register 
    //

    words = processor.pr_static_size.get_val() + processor.pr_static_size.get_val();

    double pr_ps = get_registerfile_standbypower(words,1);
    double pr_pd = get_registerfile_dynamic_power(words,1);

    pr_register_file_energy = (pr_ps * total_cycles + pr_pd * cmp_ops) * current_clock_T;


    ///////////////////////////////////////////////////////////////////
    
    ///////////// control register file ///////////////////////////////

    words = processor.cr_static_size.get_val() + processor.cr_static_size.get_val();

    double cr_ps = get_registerfile_standbypower(words,32);
    double cr_pd = get_registerfile_dynamic_power(words,32);

    cr_register_file_energy = (cr_ps * total_cycles + cr_pd* total_cycles)*current_clock_T;


    ///////////////////////////////////////////////////////////////////

    // total energy cost of current processor configuration
    energy = int_units_energy +
	     float_units_energy + 
	     mem_units_energy +
	     branch_units_energy +
	     decode_unit_energy +
	     gpr_register_file_energy +
	     fp_register_file_energy +
	     bt_register_file_energy +
	     pr_register_file_energy +
	     cr_register_file_energy;

#ifdef VERBOSE
    cout << "\n int_units_energy          " <<int_units_energy         ;
    cout << "\n float_units_energy        " <<float_units_energy       ;
    cout << "\n mem_units_energy          " <<mem_units_energy         ;
    cout << "\n branch_units_energy       " <<branch_units_energy      ;
    cout << "\n decode_unit_energy        " <<decode_unit_energy       ;
    cout << "\n gpr_register_file_energy  " <<gpr_register_file_energy  ;
    cout << "\n fp_register_file_energy   " <<fp_register_file_energy   ;
    cout << "\n bt_register_file_energy   " <<bt_register_file_energy   ;
    cout << "\n pr_register_file_energy   " <<pr_register_file_energy   ;
    cout << "\n cr_register_file_energy;  " <<cr_register_file_energy  ;
#endif

    return energy;
}


int Estimator::max_reg_size(const Processor& p) {

    // NOTE: register rotating size assumed = static size

    int max_size = 0;

    int gpr_size = p.gpr_static_size.get_val()+p.gpr_static_size.get_val();
    int fpr_size = p.fpr_static_size.get_val()+p.fpr_static_size.get_val();
    int btr_size = p.btr_static_size.get_val();
    int pr_size = p.pr_static_size.get_val()+p.pr_static_size.get_val();

    if (gpr_size>max_size) max_size = gpr_size;
    if (fpr_size>max_size) max_size = fpr_size;
    if (btr_size>max_size) max_size = btr_size;
    if (pr_size>max_size) max_size = pr_size;

    return max_size;
}

    
double Estimator::get_processor_area(const Processor& processor) 
{
    // ----------------------------------------------------------
    //     Processor Area
    // ----------------------------------------------------------
    //  all values are expressed in um^2
    //  0.35um technology
    //  before returning value is scaled by 0.35/TECHNOLOGY
    //  
    //  adapted from model presented in [1]
    // ***********************************************************
    // Processor kernel area 
    // ***********************************************************
    // minimum core kernel area
    // assuming np=4 , bopr = 2 , binst = 0 

    double ck0 = 1116272;

    // bits per instruction 
    int binst = 32 * ISSUE_WIDTH;

    int num_clusters = processor.num_clusters.get_val();

    // total instructions
    int I = 60;

    // kernel area increase for IF and ID stages ///////////////

    double ck1 = (444*binst + 5439) + 361*I;

    // kernel area increase  for register files ////////////////

    int bopr = (int)logtwo_area(max_reg_size(processor));

    double ck3 = 13088*(bopr-2);

    // kernel area increase for the presence of variuos units

    double alu_increase = 12736;
    double addr_unit_increase = 19412;
    double branch_unit_increase = 12736;

    double ck4 = (num_clusters*processor.integer_units.get_val() + 
	          num_clusters*processor.float_units.get_val() ) * alu_increase;

    ck4 += num_clusters*processor.memory_units.get_val() * addr_unit_increase;
    ck4 += num_clusters*processor.branch_units.get_val() * branch_unit_increase;

    // total processor kernel area
    
    double kernel_area = ck0 + ck1 + ck3 + ck4;

    // ***********************************************************
    // Register Files area
    // ***********************************************************

    // cr(32,nr,4) = 67184*nr + 5521

    /////////////////////////////////////////////////////////////////
    // NOTE:
    // register size = static + rotating , but rotating is assumed
    // equal to static portion
    int n_regs_32bit =num_clusters*processor.gpr_static_size.get_val()+
   		      num_clusters*processor.gpr_static_size.get_val()+
		      num_clusters*processor.cr_static_size.get_val()+
		      num_clusters*processor.cr_static_size.get_val();

    int n_regs_64bit = num_clusters*processor.fpr_static_size.get_val() + 
	               num_clusters*processor.fpr_static_size.get_val()+
		       num_clusters*processor.btr_static_size.get_val();


    // missing pr , only 1 bit width 

    double reg_area = (67184 * n_regs_32bit + 5521) + 2*(67184 * n_regs_64bit + 5521);

    //*************************************************************
    // Hardware units area
    //*************************************************************
    //
    // Note:
    // factor 1.32353 is derived from area values of power_conf file in PowerImpact
    // source, as the result of (area_falu/area_ialu)
   
    double int_units_area = num_clusters*processor.integer_units.get_val()*151194 ;
    double float_units_area = num_clusters*processor.float_units.get_val()*151194*1.32353;
    double addr_units_area = num_clusters*30535 * processor.memory_units.get_val();
    double branch_units_area = num_clusters*processor.branch_units.get_val()*151194;

    double units_area = int_units_area + float_units_area + addr_units_area + branch_units_area;


    // value is converted from um^2 to cm^2
    double total_area  = (kernel_area + reg_area + units_area)/100000000;

    double scaling_factor = 0.35/TECHNOLOGY;

    return total_area/scaling_factor;
}


void Estimator::set_autoclock(bool enabled)
{
    this->autoclock = enabled;

    if (!enabled)
    {
	current_clock_freq = DEFAULT_CLOCK;
	current_clock_T = 1/current_clock_freq;
	current_power_density_scale = 2e9/current_clock_freq;
    }
}

// Register file access time - tech_size TECHNOLOGY assumed
double Estimator::get_registerfile_access_time(int words,int size,int write_ports,int read_ports)
{

}

// Cache access time - Based on Cacti models
double Estimator::get_cache_access_time(const Mem_hierarchy::Cache& cache)
{
    parameter_type  parameters;
    area_type  arearesult_subbanked;
    arearesult_type  arearesult;
    result_type  result;

    double area ;
    double NSubbanks = 1;

    /* Cacti parameters, asssuming  : 
     RWP = 1; ERP = 2; EWP = 1; NSER = 0; Ndbl = 1; Ndwl = 1; Nspd = 1; Ntbl = 1; Ntwl = 1; Ntspd = 1;
    NSubbanks = 1; */

    int A = cache.associativity.get_val();
    int B = cache.block_size.get_val();
    int C = cache.size.get_val();


    if (A==C/B)  // associativity = total number of blocks
	parameters.fully_assoc=1;
    else
	parameters.fully_assoc=0;

    // check for cacti feasible cache
    if (C/(8*B*A)<=0 && !parameters.fully_assoc)
    {
	int pippo;
	if (C/(8*8*A)>0) // if is possible to solve the problem setting min block size
	{
	    do  // decrease B until first valid combination is found
	    {
		B = B/2;

	    } while (C/(8*B*A)<=0);
	}
	else // even setting B to min it's not sufficient
	{
	    B=8; // set to the minimun, to make things easier

	    if (65536/(8*B*A)>0) // it's possible to solve the problem with max size val
	    {
		do
		{
		    C=C*2;
		} while (C/(8*B*A)<=0);
	    }
	    else // impossible to solve the problem , this should not happen
	    {
		cout << "\n ERROR :Impossible to find a valid alternative to cache config : ";
		cout << "\n A = " << cache.associativity.get_val();
		cout << "\n B = " << cache.block_size.get_val();
		cout << "\n C = " << cache.size.get_val();
		wait_key();
	    };
	}
#ifdef VERBOSE
	cout << "\n Cache config replaced for access time computing ";
	cout << "\n Original config : ";
	cout << "\n A = " << cache.associativity.get_val();
	cout << "\n B = " << cache.block_size.get_val();
	cout << "\n C = " << cache.size.get_val();
	cout << "\n\n new config : ";
	cout << "\n A = " << A;
	cout << "\n B = " << B;
	cout << "\n C = " << C;
#endif
	
    }


    int baddr = ADDRESS_BITS;
    int b0 = BITOUT;

    parameters.cache_size= C;
    parameters.block_size= B;
    parameters.associativity = A;
    parameters.num_readwrite_ports = 0; //RWP
    parameters.num_read_ports = ISSUE_WIDTH*2  ;  // ERP
    parameters.num_write_ports = ISSUE_WIDTH; // EWR
    parameters.num_single_ended_read_ports = 0; // NSER
    parameters.number_of_sets = C/(B*A);
    parameters.fudgefactor = 0.8/TECHNOLOGY;
    parameters.tech_size = TECHNOLOGY;
    parameters.VddPow = VDD;

    calculate_time(&result,&arearesult,&arearesult_subbanked,&parameters,&NSubbanks);

    double access_time = result.access_time;
    double cycle_time = result.cycle_time;

    return access_time;
}

double Estimator::get_cache_area(const Mem_hierarchy::Cache& cache)
{
    parameter_type  parameters;
    area_type  result_subbanked;
    arearesult_type  result;

    double area ;
    double NSubbanks = 1;

    /* Cacti parameters
     * Assuming  : 

     RWP = 1; ERP = 0; EWP = 0; NSER = 0; Ndbl = 1; Ndwl = 1; Nspd = 1; Ntbl = 1; Ntwl = 1; Ntspd = 1;
    NSubbanks = 1; 
      */

    int A = cache.associativity.get_val();
    int B = cache.block_size.get_val();
    int C = cache.size.get_val();

    if (A==C/B)  // associativity = total number of blocks
	parameters.fully_assoc=1;
    else
	parameters.fully_assoc=0;

    int baddr = ADDRESS_BITS;
    int b0 = BITOUT;

    parameters.cache_size= C;
    parameters.block_size= B;
    parameters.associativity = A;
    parameters.num_readwrite_ports = 0; //RWP
    parameters.num_read_ports = 2;  // ERP
    parameters.num_write_ports = 1; // EWR
    parameters.num_single_ended_read_ports = 0; // NSER
    parameters.number_of_sets = C/(B*A);
    parameters.fudgefactor = 0.8/TECHNOLOGY;
    parameters.tech_size = TECHNOLOGY;
    parameters.VddPow = VDD;

    area_subbanked(baddr, b0,0,2,1,1,1,1,1,1,1,NSubbanks,&parameters,&result_subbanked, &result);

    // expressed in cm^2
    
    area = result.totalarea/100000000.0; 

    return area;
}

double Estimator::get_mem_hierarchy_area(const Mem_hierarchy& mem_hierarchy)
{
    double l1d_area = get_cache_area(mem_hierarchy.L1D);
    double l1i_area = get_cache_area(mem_hierarchy.L1I);
    double l2u_area = get_cache_area(mem_hierarchy.L2U);

    return l1d_area+l1i_area+l2u_area;
}

double Estimator::get_total_area(Processor& processor, Mem_hierarchy& mem_hierarchy)
{
    double mem_area = get_mem_hierarchy_area(mem_hierarchy);
    double processor_area = get_processor_area(processor);

    return mem_area+processor_area;

}

double Estimator::get_cache_energy(const uint64& r_hit,
	                           const uint64& r_miss,
				   const uint64& w_hit,
				   const uint64& w_miss,
				   const uint64& N_wb_req,
				   const double& addr_trans_prob,
				   const int& L_higher_block,
				   const Mem_hierarchy::Cache& cache)
{

    uint64 accesses = r_hit + r_miss + w_hit + w_miss;
    int D = cache.size.get_val(); 
    int m = cache.associativity.get_val(); 
    int L = cache.block_size.get_val(); 
    int sb = L;

    //int W_word = (int)(logtwo_area(sb)); // bits for subblock
    int S = D/(L*m); // number of sets

    int W_addr_bus = ADDRESS_BITS; // width of address bus
    double W_avg_data = (8+16+32)/3;  // average data width of a write request

    // number of bit per tag
    int T = (int)(W_addr_bus - logtwo_area(L) - logtwo_area(S));

    int St = 1; // number of status bit per block frame

    double C_wordwire = C_WORDWIRE;
    double C_bit_pr = S * (0.5*C_D_Q1+C_BIT);
    double C_bit_rw = S * (0.5*C_D_Q1+C_BIT)+C_D_QP+C_D_QPA;
    double C_awire = 0.25*8*L*m*C_WORDWIRE;

    //int N_bitlines = (T*m+St+W_word*m)*2; // ( if subblock )
    int N_bitlines = (T*m+St+8*L*m)*2; // ( if subblock )
    
    // Transition counts 
    ////////////////////////////////////////////////////////////////////
    double N_bit_pr = 0.5*accesses*N_bitlines;
    double N_bit_r = 0.5*accesses*N_bitlines;
    double N_bit_w = 0.5*r_miss*(T*1+St+8*L*1)*2+0.5*w_hit*(St+W_avg_data)*2;
    double N_out_a2m = 0.5*(r_miss+w_miss+N_wb_req)*W_addr_bus; // if write-back
    double N_out_d2m = 0.5*w_miss*W_avg_data+0.5*N_wb_req*8*L;// ( write back)
    double N_out_d2c; // transitions caused from delivering data to an higher level

    N_out_d2c = 0.5*(r_hit+r_miss)*8*L_higher_block;

    double N_a_input = addr_trans_prob*accesses*W_addr_bus;

    /////////////////////////////////////////////////////////////////////

    /// Energy dissipated in bitlines 

    double E_bit = 0.5*(VDD*VDD)*(N_bit_pr*C_bit_pr + N_bit_w*C_bit_rw + N_bit_r*C_bit_rw+m*(8*L+T+St)*accesses*(C_G_QPA+C_G_QPB+C_G_QP) );

    /// Energy dissipated in wordlines
    
    double E_word = (VDD*VDD)*accesses*m*(L*8+T+St)*(2*C_G_Q1+C_wordwire);

    // Energy dissipated in the output lines

    int N_out_a2c = 0;
    double E_a_output = 0.5*(VDD*VDD)*(N_out_a2m*C_OUT_A2M+N_out_a2c*C_OUT_A2C);
    double E_d_output = 0.5*(VDD*VDD)*(N_out_d2m*C_OUT_D2M+N_out_d2c*C_OUT_D2C);

    double E_output = E_a_output + E_d_output;

    // Energy dissipated in the address input lines

    double E_a_input = 0.5 * (VDD*VDD)*N_a_input*((m+1)*2*S*C_IN_DEC+C_awire);

    double E_cache = E_bit + E_word + E_output + E_a_input;

    // must scale from 0.8um technology to 0.13um 
    // As suggested from CACTI authors, a simple way is to divide
    // energy by the scaling factor

    double scaling_factor = 0.8/TECHNOLOGY;

    E_cache = E_cache/scaling_factor;

    return E_cache;
}

double Estimator::get_main_memory_energy(const uint64& n_request)
{
    // TODO: each request is bigger than a word
    double average_access_consumption = 4.95e-9;
    return n_request*average_access_consumption;
}

Estimate Estimator::get_estimate(const Dynamic_stats& dyn_stats, 
	                         const Mem_hierarchy& mem, 
				 const Processor& processor)
{
    string tmp;
    Estimate estimate;

    estimate.L1D_access_time = get_cache_access_time(mem.L1D);
    estimate.L1I_access_time = get_cache_access_time(mem.L1I);

    // If auto clock option is enabled, CLOCK_FREQ and current_clock_T are recalibrated
    // to allow L1 access time
    if (this->autoclock) 
    {
	double max_access_time = max(estimate.L1D_access_time,estimate.L1I_access_time);
	current_clock_T = max_access_time*1.1; 
	current_clock_freq= 1/(double)current_clock_T;
#ifdef VERBOSE
	cout << "\n updating clock frequency ";
	cout << "\n CLOCK_FREQ " << current_clock_freq;
	cout << "\n current_clock_T " << current_clock_T;
#endif
    }
    else
    {
	current_clock_freq = DEFAULT_CLOCK;
	current_clock_T = 1/(double)current_clock_freq;
    }

    // power densities were originally computed for 2Ghz frequency
    current_power_density_scale = 2e9/current_clock_freq;

    estimate.clock_freq = current_clock_freq;
    estimate.clock_T = current_clock_T;
    estimate.power_density_scale = current_power_density_scale;

    // Processor energy ////////////////////////////////

    estimate.total_processor_energy= get_processor_energy(dyn_stats,processor,mem);

    // Address bus transitions ////////////////////////

    estimate.L1D_transition_p = dyn_stats.L1D_transition_p;
    estimate.L1I_transition_p = dyn_stats.L1I_transition_p;

    // L1 data cache energy ////////////////////////////

    uint64 r_hit = dyn_stats.L1D_r_hit;
    uint64 r_miss = dyn_stats.L1D_r_miss;
    uint64 w_hit = dyn_stats.L1D_w_hit;
    uint64 w_miss = dyn_stats.L1D_w_miss;

    uint64 N_wb_req = dyn_stats.L1D_writebacks;

    int hb = 19; // average data width of data delivery to CPU
   
    estimate.L1D_energy= get_cache_energy(r_hit,r_miss,w_hit,w_miss,N_wb_req,dyn_stats.L1D_transition_p,hb,mem.L1D);

    // L1 instruction energy //////////////////////////

    r_hit = dyn_stats.L1I_hit;
    r_miss = dyn_stats.L1I_miss;
    w_hit = 0;
    w_miss = 0;
    N_wb_req = dyn_stats.L1I_writebacks;

    estimate.L1I_energy= get_cache_energy(r_hit,r_miss,w_hit,w_miss,N_wb_req,dyn_stats.L1I_transition_p,hb,mem.L1I );
    
    // L2 unified cache energy ///////////////////////

    // TODO: find how m5 distinguish between r/w
    r_hit = dyn_stats.L2U_hit/2;
    r_miss = dyn_stats.L2U_miss/2;
    w_hit = r_hit;
    w_miss = r_miss;

    N_wb_req = dyn_stats.L2U_writebacks;

    // average higher level data delivery width 

    hb = (8*(mem.L1D.block_size.get_val()+mem.L1I.block_size.get_val()))/2;

    estimate.L2U_energy= get_cache_energy(r_hit,r_miss,w_hit,w_miss,N_wb_req,dyn_stats.L2U_transition_p,hb,mem.L2U);

    estimate.main_memory_energy = get_main_memory_energy(dyn_stats.SDRAM_accesses);

    /////////////////////////////////////////////////

    estimate.total_cache_energy = estimate.L1D_energy + estimate.L1I_energy + estimate.L2U_energy;
    estimate.total_system_energy = estimate.total_cache_energy + estimate.total_processor_energy + estimate.main_memory_energy;

#ifdef OLD_STYLE_STALLS
    estimate.execution_cycles = get_execution_cycles(dyn_stats,mem);
    estimate.stall_cycles = estimate.execution_cycles - dyn_stats.compute_cycles;
#else
    estimate.execution_cycles = dyn_stats.total_cycles;
    estimate.stall_cycles = dyn_stats.stall_cycles;
#endif

    estimate.compute_cycles = dyn_stats.compute_cycles;
    estimate.execution_time = estimate.execution_cycles * current_clock_T;
    estimate.IPC = (double)dyn_stats.total_dynamic_operations/estimate.execution_cycles;
    estimate.total_average_power = estimate.total_system_energy/estimate.execution_time;

    // AREA estimates /////////////////////////////////
    
    estimate.L1D_area = get_cache_area(mem.L1D);
    estimate.L1I_area = get_cache_area(mem.L1I);
    estimate.L2U_area = get_cache_area(mem.L2U);
    estimate.processor_area = get_processor_area(processor);
    estimate.total_area = estimate.L1D_area + estimate.L1I_area + estimate.L2U_area + estimate.processor_area;
    return estimate;
}

#ifdef OLD_STYLE_STALLS
int Estimator::get_miss_penality(const Mem_hierarchy::Cache& cache)
{
    int latency ; // of lower level 
    int width;    // in bytes/cycle from lower level 

    switch (cache.type)
    {
	case L1D_CACHE:
	    latency = 10; 
	    width = 8;    
	    break;
	case L1I_CACHE:
	    latency = 10;
	    width = 8;
	    break;
	case L2U_CACHE:
	    latency = 30;
	    width = 8;
	    break;
    };

    return ( latency + cache.block_size.get_val()/width );
}


double Estimator::get_cycles_per_hit(const Mem_hierarchy::Cache& cache)
{
    char ch;

    switch (cache.associativity.get_val())
    {

	case 1:
	    return 1;
	    break;

	case 2:
	    return 1.1;
	    break;

	case 4:
	    return 1.12;
	    break;

	case 8:
	    return 1.14;
	    break;

	case 16:
	    return 1.16;
	    break;
	default:
	    cout << "\n Error in Estimator::get_cycles_per_hit() : ";
	    cout << "\n Wrong associativity value: " << cache.associativity.get_val();
	    exit(-1);
    }
    return -1;
}
uint64 Estimator::get_execution_cycles(const Dynamic_stats& dyn_stats,const Mem_hierarchy& m )
{
    uint64 processor_cycles = dyn_stats.compute_cycles;

    uint64 L1D_miss_cycles = dyn_stats.L1D_miss*get_miss_penality(m.L1D);
    uint64 L1I_miss_cycles = dyn_stats.L1I_miss*get_miss_penality(m.L1I);
    uint64 L2U_miss_cycles = dyn_stats.L2U_miss*get_miss_penality(m.L2U);

    uint64 memory_stall_cyles = L1D_miss_cycles + L1I_miss_cycles + L2U_miss_cycles;

    double L1D_extra_hit_cycles=dyn_stats.L1D_hit*(get_cycles_per_hit(m.L1D) - 1);
    double L1I_extra_hit_cycles= dyn_stats.L1I_hit*(get_cycles_per_hit(m.L1I)-1);
    double L2U_extra_hit_cycles =dyn_stats.L2U_hit*(get_cycles_per_hit(m.L2U)-1);

    uint64 total_extra_hit_cycles = (uint64)(L1D_extra_hit_cycles+L1I_extra_hit_cycles+L2U_extra_hit_cycles);

#ifdef VERBOSE
    cout << "\n -------------- Cache stats ------------------";
    cout << "\n L1D_misses : " << dyn_stats.L1D_miss << " -> miss cyles :" << L1D_miss_cycles;
    cout << "\n L1I_misses : " << dyn_stats.L1I_miss<< " -> miss cyles :" << L1I_miss_cycles;;
    cout << "\n L2U_misses : " << dyn_stats.L2U_miss<< " -> miss cyles :" << L2U_miss_cycles;;
    
    
    cout << "\n Memory stall cycles : " << memory_stall_cyles;

    cout << "\n L1D_hit " << dyn_stats.L1D_hit;
    cout << "\n L1I_hit " << dyn_stats.L1I_hit;
    cout << "\n L2U_hit " << dyn_stats.L2U_hit;


    cout << "\n -------------- Extra cache stats ------------";
    cout << "\n L1I_extra_hit_cycles: " << L1I_extra_hit_cycles;
    cout << "\n L1D_extra_hit_cycles: " << L1D_extra_hit_cycles;
    cout << "\n L2U_extra_hit_cycles: " << L2U_extra_hit_cycles;
    cout << "\n ---------------------------------------------";
#endif
    
    uint64 total_cycles = processor_cycles + total_extra_hit_cycles+ memory_stall_cyles;

    return total_cycles;
}
#endif


