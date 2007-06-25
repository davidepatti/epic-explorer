/***************************************************************************
                          user_interface.h  -  description
                             -------------------
    begin                : Sat Aug 24 2002
    copyright            : (C) 2002 by Davide Patti
    email                : davidepatti@email.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/////////////////////////////////////////////////////////////////////
// low level parameter of cache power model /////////////////////////
// -----------------------------------------------------------------
//
// Formulas and values adapted from Cacti source code formulas
//
// computed for 0.8um technology - 
// Total resulting cache energy will be scaled to 0.13um dividing by
// 0.8/0.13 factor
// //////////////////////////////////////////////////////////////////

#define C_GATE 1.95e-15 
#define C_GATE_PASS 1.45e-15
#define C_BIT 4.4e-15 // 16um of m2 @ 275fF/um
#define C_WORDWIRE 1.8e-15 // 8um of m3 @ 225fF/um
#define L_EFF 0.8
#define C_POLYWIRE 0.25e-15
#define C_NDIFF_OVERLAP 0.138e-15
#define C_PDIFF_OVERLAP 0.138e-15
#define C_NDIFFAREA 0.137e-15
#define C_PDIFFAREA 0.343e-15
#define C_NDIFFSIDE 0.275e-15
#define C_PDIFFSIDE 0.275e-15
#define C_NOXIDE_OVERLAP 0.263e-15 // fF/um assuming 25% Miller effect
#define C_POXIDE_OVERLAP 0.338e-15 // fF/um assuming 25% Miller effect

#define C_NOVERLAP (C_NDIFF_OVERLAP+C_NOXIDE_OVERLAP)
#define C_POVERLAP (C_PDIFF_OVERLAP+C_POXIDE_OVERLAP)

#define C_METAL 0.275e-15
#define W 1
#define L_POLY 2
#define WIREPITCH (0.8*2+0.8*3)


#define C_OUT_A2M 20e-12
#define C_OUT_D2M 20e-12
#define C_OUT_A2C 0.5e-12
#define C_OUT_D2C 0.5e-12

// computed as gatecappass(Wmemcella,0.0) in array wordline section

#define C_G_Q1 (2.4*L_EFF*C_GATE_PASS)


#define C_G_QPB (80*L_EFF*C_GATE)  //+L_POLY*L_EFF*C_POLYWIRE
#define C_G_QPA (80*L_EFF*C_GATE)  //+L_POLY*L_EFF*C_POLYWIRE
#define C_G_QP (80*L_EFF*C_GATE)  //+L_POLY*L_EFF*C_POLYWIRE

//Bitline transistors Capacitance - values derived from Cacti source
//
// Called Cbitrow in cacti source 
#define C_D_Q1 (3*L_EFF*2.4*C_NDIFFAREA+(6*L_EFF+2.4)*C_NDIFFSIDE+2.4*C_NOVERLAP)

#define C_D_QPA (3*L_EFF*80*C_PDIFFAREA/2+6*L_EFF*C_PDIFFSIDE+80*C_POVERLAP)
#define C_D_QP (3*L_EFF*80*C_PDIFFAREA/2+6*L_EFF*C_PDIFFSIDE+80*C_POVERLAP)


// See decoder section in Cacti source code
//
#define C_IN_DEC ( (3*L_EFF*100/2*C_PDIFFAREA+6*L_EFF*C_PDIFFSIDE+100*C_POVERLAP)+\
	           (3*L_EFF*600/2*C_NDIFFAREA+6*L_EFF*C_NDIFFSIDE+60*C_NOVERLAP)+\
		   4*(150*L_EFF*C_GATE+10*C_POLYWIRE*L_EFF) )


#define ADDRESS_BITS 32
#define BITOUT 64

////// End of low level parameters of cache power model///////////////


//////////////////////////////////////////////////////////////////////
// Declarations needed to interface Estimator class to the cache area
// estimation functions of area.c 


typedef struct {
	double height;
	double width;
} area_type;

typedef struct {
        area_type dataarray_area,datapredecode_area;
        area_type datacolmuxpredecode_area,datacolmuxpostdecode_area;
	area_type datawritesig_area;
        area_type tagarray_area,tagpredecode_area;
        area_type tagcolmuxpredecode_area,tagcolmuxpostdecode_area;
        area_type tagoutdrvdecode_area;
        area_type tagoutdrvsig_area;
        double totalarea;
	double total_dataarea;
        double total_tagarea;
	double max_efficiency, efficiency;
	double max_aspect_ratio_total, aspect_ratio_total;
} arearesult_type;

typedef struct {
   int cache_size;
   int number_of_sets;
   int associativity;
   int block_size;
   int num_write_ports;
   int num_readwrite_ports;
   int num_read_ports;
   int num_single_ended_read_ports;
  char fully_assoc;
  float fudgefactor;
  float tech_size;
  float VddPow;
} parameter_type;

typedef struct {
   double access_time,cycle_time;
  double senseext_scale;
  double total_power;
   int best_Ndwl,best_Ndbl;
  double max_power, max_access_time;
   int best_Nspd;
   int best_Ntwl,best_Ntbl;
   int best_Ntspd;
  int best_muxover;
   double total_routing_power;
   double total_power_without_routing, total_power_allbanks;
   double subbank_address_routing_delay,subbank_address_routing_power;
   double decoder_delay_data,decoder_delay_tag;
   double decoder_power_data,decoder_power_tag;
   double dec_data_driver,dec_data_3to8,dec_data_inv;
   double dec_tag_driver,dec_tag_3to8,dec_tag_inv;
   double wordline_delay_data,wordline_delay_tag;
   double wordline_power_data,wordline_power_tag;
   double bitline_delay_data,bitline_delay_tag;
   double bitline_power_data,bitline_power_tag;
  double sense_amp_delay_data,sense_amp_delay_tag;
  double sense_amp_power_data,sense_amp_power_tag;
  double total_out_driver_delay_data;
  double total_out_driver_power_data;
   double compare_part_delay;
   double drive_mux_delay;
   double selb_delay;
   double compare_part_power;
   double drive_mux_power;
   double selb_power;
   double data_output_delay;
   double data_output_power;
   double drive_valid_delay;
   double drive_valid_power;
   double precharge_delay;
  int data_nor_inputs;
  int tag_nor_inputs;
} result_type;

extern "C" double logtwo_area(double x);
extern "C" void area_subbanked(int baddr,int b0,int RWP,int ERP,int EWP,int Ndbl,int Ndwl,int Nspd,int Ntbl,int Ntwl,int Ntspd,double NSubbanks,parameter_type * parameters,area_type * result_subbanked,arearesult_type * result);
extern "C" void calculate_time(result_type * result,arearesult_type * arearesult,area_type * arearesult_subbanked,
                               parameter_type * parameters,double * NSubbanks);

//////////////////////////////////////////////////////////////////////
