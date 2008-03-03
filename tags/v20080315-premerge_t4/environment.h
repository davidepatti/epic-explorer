/***************************************************************************
                          environment.h  -  description
                             -------------------
    begin                : Mon Aug 19 2002
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

/* NOTES :
 *
 * this file must be configured according to your own system paths of
 * trimaran installation .
 * These values should be the same of the "compile_script" files that
 * can be found in:
 * ~/trimaran-workspace/projects/NAME_OF_A_PROJECT/NAME_OF_A_COMPILATION/

 IMPORTANT! : all paths are relative to your $HOME directory
 so , if you are using the exports commands of a 'compile_script' 
 file to setup these values don't add $HOME path at the beginning of
 these variables.
 Path variables should not be changed, change compiler and platform if you 
 are not using linux.
*/

#define CC "gcc"
#define CXX "gcc"

#define TRIMARAN_HOST_PLATFORM "x86lin" //  hp|x86lin|sunsol|sunos
#define TRIMARAN_HOST_COMPILER "gcc"
#define USER_BENCH_PATH1 "/trimaran/./impact/projects"

#define IMPACT_HOST_PLATFORM "x86lin"  //hp|x86lin|sunsol|sunos
#define IMPACT_HOST_COMPILER "gcc"
#define IMPACT_ROOT "/trimaran/./impact"
#define IMPACT_REL_PATH "/trimaran/./impact"
#define IMPACT_BIN_PATH "/trimaran/./impact/bin/x86lin_c"
#define IMPACT_LIB_PATH "/trimaran/./impact/lib/x86lin_c"
#define STD_PARMS_FILE "/trimaran-workspace/parms/std/impact/STD_PARMS.TRIMARAN"
#define IMPACT_BUILD_TYPE "x86lin"  //hp|x86lin|sunsol|sunos

#define ELCOR_HOME "/trimaran/./elcor"
#define ELCOR_REL_PATH "/trimaran/./elcor"
#define ELCOR_BIN_PATH "/trimaran/./elcor/bin"
#define ELCOR_PARMS_FILE "/trimaran-workspace/parms/std/elcor/ELCOR_PARMS"
#define ELCOR_USER "ELCOR"

#define SIMU_HOME "/trimaran/./simu"
#define SIMU_REL_PATH "/trimaran/./simu"
#define SIMU_BIN_PATH "/trimaran/./simu/bin"
#define SIMU_PARMS_FILE "/trimaran-workspace/parms/std/simu/SIMULATOR_DEFAULTS"
#define DEFAULT_PROJECT "full"

#define DINERO_HOME "/trimaran/./dinero"
#define DINERO_REL_PATH "/trimaran/./dinero"
