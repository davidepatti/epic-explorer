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
 * These values should be the same of the "envrc" files that
 * can be found in trimaran/scripts directory.

 IMPORTANT! : all paths are relative to your the BASE_DIR defined in common.h
*/

// new exports for trimaran 4.0

#define TRIMARAN_HOST_TYPE "x86lin"
//export TRIMARAN_ROOT=~/trimaran should be set in common.h
#define TRIMARAN_HOST_COMPILER "gcc"
#define IMPACT_REL_PATH "/trimaran/openimpact"
#define IMPACT_ROOT "/trimaran/openimpact"
#define IMPACT_HOST_PLATFORM "x86lin"  //hp|x86lin|sunsol|sunos
#define IMPACT_BUILD_TYPE "x86lin"  //hp|x86lin|sunsol|sunos
#define IMPACT_HOST_COMPILER "gcc"
#define STD_PARMS_FILE "/trimaran/openimpact/parms/STD_PARMS.IMPACT"
#define DEFAULT_PROJECT "full"
#define CC "gcc"
#define CXX "g++"

#define ELCOR_HOME "/trimaran/elcor"
#define ELCOR_REL_PATH "/trimaran/elcor"
#define ELCOR_BIN_PATH "/trimaran/elcor/bin"
#define ELCOR_PARMS_FILE "/trimaran/elcor/parms/ELCOR_PARMS"
#define ELCOR_USER "ELCOR"

#define SIMU_HOME "/trimaran/simu"
#define SIMU_REL_PATH "/trimaran/simu"
#define SIMU_BIN_PATH "/trimaran/simu/bin"
#define SIMU_PARMS_FILE "/trimaran/simu/parms/SIMULATOR_DEFAULTS"

#define DINERO_HOME "/trimaran/simu/dinero"

//export ARM_REL_PATH=${TRIMARAN_ROOT}/arm
//
#define GUI_REL_PATH "/trimaran/gui"
#define GUI_BIN "/trimaran/gui/bin"
#define  TAS_HOME "/trimaran/tas"
#define  TAS_BIN "/trimaran/tas/bin"

#define  TAS_PARMS_FILE "/trimaran/tas/parms/TAS_PARMS"
#define  TAS_REL_PATH "/trimaran/tas"

//#define USER_BENCH_PATH1 "/trimaran/./impact/projects"

#define USER_BENCH_PATH1 "/trimaran/benchmarks/simple /trimaran/benchmarks/utils \
                         /trimaran/benchmarks/mediabench \
                         /trimaran/benchmarks/netbench \
                         /trimaran/benchmarks/encryption \
                         /trimaran/benchmarks/mibench/automotive \
                         /trimaran/benchmarks/mibench/network \
                         /trimaran/benchmarks/mibench/telecomm \
                         /trimaran/benchmarks/mibench/security \
                         /trimaran/benchmarks/mibench/consumer \
                         /trimaran/benchmarks/specint92 \
                         /trimaran/benchmarks/specfp92 \
                         /trimaran/benchmarks/specint95 \
                         /trimaran/benchmarks/specfp95 \
                         /trimaran/benchmarks/specint2000 \
                         /trimaran/benchmarks/specfp2000 \
                         /trimaran/benchmarks/specint2006 \
                         /trimaran/benchmarks/specfp2006 \
                         /trimaran/benchmarks/perfect \
                         /trimaran/benchmarks/integer_bench \
                         /trimaran/benchmarks/eecs583 \
                         /trimaran/benchmarks/streams \
                         /trimaran/benchmarks/olden \
                         /trimaran/benchmarks/kernels"

// TO CHECK - are these exports necessary ?
//
#define  DAVINCIHOME "/usr/share/daVinci_V2.1"
#define  DAVINCIEXE "/usr/share/daVinci_V2.1"
//
//# M5elements
#define M5_ROOT "/trimaran/m5/install"
#define M5_OUTPUT_DIR "."
#define M5_CONFIG_FILE "/trimaran/m5/install/m5/m5elements/default.py"
//export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$M5_ROOT/m5/build/ALPHA
//
//#benchmark related stuff
#define  PGPPATH "./"
//
//export PATH=${PATH}:${IMPACT_REL_PATH}/bin:${IMPACT_REL_PATH}/scripts:${IMPACT_REL_PATH}/driver:${ELCOR_BIN}:${SIMU_BIN_PATH}:${TRIMARAN_ROOT}/scripts:${GUI_BIN}:${TAS_BIN}:${ARM_REL_PATH}
//
#define SUIFHOME "/trimaran/suif"
#define MACHINE "x86-redhat-linux"
#define COMPILER_NAME "gcc"
//eval `$SUIFHOME/setup_suif -sh 2> /dev/null`
//export SUIFPATH=$SUIFPATH:/usr/bin

