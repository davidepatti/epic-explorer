
/***************************************************************************
                          user_interface.h  -  description
                             -------------------
    begin                : Sat Aug 24 2002
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
/////////////////////////////////////////////////////////////////////
// Active/Inactive Power densities parameters for Cai-Lim model
//
// expressed in W/(mm^2)
//
// Computed for 0.13um technology at 1.3 V
//
// Since GPR,FPR,PR,BTR,CR register file size can change, so their
// dynamic/static power dissipation cannot be computed a priori
// specifying area composition. See estimator.cpp for more details.
//
//
// //////////////////////////////////////////////////////////////////

#define VDD 1.3
#define IUSSUE_WIDTH 6
#define TECHNOLOGY 0.13

#define DEFAULT_CLOCK 200e6

#define STATIC_PDA    6.05  
#define STATIC_PDS   0.605    
#define DYNAMIC_PDA   7.72      
#define DYNAMIC_PDS  0.772
#define CLOCK_PDA     8.43   
#define CLOCK_PDS     8.43
#define MEMORY_PDA   10.75    
#define MEMORY_PDS    5.25
#define PLA_PDA      91.75  
#define PLA_PDS      9.175

// Area occupation of each type of circuit in the considered units
// Expressed in  mm^2
//

//<DECODE>
#define DECODE_STATIC_AREA  0.1745
#define DECODE_DYNAMIC_AREA  0.032
#define DECODE_CLOCK_AREA  0.027
#define DECODE_MEMORY_AREA  0
#define DECODE_PLA_AREA  0.064

//<IALU>
#define IALU_STATIC_AREA  0.238
#define IALU_DYNAMIC_AREA  0.085
#define IALU_CLOCK_AREA  0.017
#define IALU_MEMORY_AREA  0.0
#define IALU_PLA_AREA  0.0

//<FALU>
#define FALU_STATIC_AREA  0.315
#define FALU_DYNAMIC_AREA  0.1125
#define FALU_CLOCK_AREA  0.0225
#define FALU_MEMORY_AREA  0.0
#define FALU_PLA_AREA  0.0

// used for register files power 
#define TMPR 283
