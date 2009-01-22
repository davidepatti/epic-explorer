#Makefile for Epic

GAINC_DIR = ./spea2/src
GALIB_DIR = ./spea2/lib
GASRC_DIR = ./spea2/src

CC = gcc
CXX = g++

# Misc stuff for mpi environment support
# if you want, adapt it and export EPIC_MPI=1 before compiling
ifdef EPIC_MPI
MPICC = mpicxx
CFLAGS += -DEPIC_MPI -DMPICH_IGNORE_CXX_SEEK
#CFLAGS += -DEPIC_MPI -DMPICH_IGNORE_CXX_SEEK -DNDEBUG -Minform=severe Ktrap=fp
else
MPICC = ${CXX}
CFLAGS += -DNDEBUG 
#CFLAGS += -DTEST -DNDEBUG 
#CFLAGS += -O2 -DNDEBUG
#CFLAGS += -g -DDEBUG
endif

all: epic

epic: ${GALIB_DIR}/libspea2.a compiler.o explorer.o alg_dep.o alg_random.o alg_sensivity.o alg_genetic.o \
	estimator.o area.o time.o processor.o mem_hierarchy.o \
	main.o user_interface.o trimaran_interface.o \
	parameter.o common.o simulate_space.o \
	FuzzyApprox.o RuleList.o FuzzyWrapper.o 
	${MPICC} compiler.o explorer.o simulate_space.o alg_dep.o alg_random.o alg_sensivity.o alg_genetic.o \
	user_interface.o trimaran_interface.o estimator.o area.o time.o \
	processor.o mem_hierarchy.o main.o parameter.o common.o \
	FuzzyApprox.o RuleList.o FuzzyWrapper.o \
	-L${GALIB_DIR} -lspea2 -o epic

compiler.o: compiler.cpp compiler.h parameter.h
	${CXX} -I${GAINC_DIR} ${CFLAGS} -c compiler.cpp

estimator.o: estimator.cpp estimator.h processor.h mem_hierarchy.h \
	power_densities.h cacti_area_interface.h 
	${CXX} ${CFLAGS} -c estimator.cpp

explorer.o: explorer.cpp explorer.h processor.h trimaran_interface.h \
	mem_hierarchy.h estimator.h parameter.h common.h \
	FunctionApprox.h FuzzyApprox.h FannApprox.h
	${MPICC} -I${GAINC_DIR} ${CFLAGS} -c explorer.cpp

simulate_space.o: simulate_space.cpp explorer.h 
	${MPICC} -I${GAINC_DIR} ${CFLAGS} -c simulate_space.cpp

alg_dep.o: alg_dep.cpp explorer.h common.h
	${MPICC} -I${GAINC_DIR} ${CFLAGS} -c alg_dep.cpp

alg_sensivity.o: alg_sensivity.cpp explorer.h common.h
	${MPICC} -I${GAINC_DIR} ${CFLAGS} -c alg_sensivity.cpp

alg_random.o: alg_random.cpp explorer.h common.h
	${MPICC} -I${GAINC_DIR} ${CFLAGS} -c alg_random.cpp

alg_genetic.o: alg_genetic.cpp explorer.h common.h
	${MPICC} -I${GAINC_DIR} ${CFLAGS} -c alg_genetic.cpp

processor.o: processor.cpp processor.h parameter.h
	${CXX} ${CFLAGS} -c processor.cpp

mem_hierarchy.o: mem_hierarchy.cpp mem_hierarchy.h parameter.h
	${CXX} ${CFLAGS} -c mem_hierarchy.cpp

main.o: main.cpp user_interface.h
	${MPICC} -I${GAINC_DIR} ${CFLAGS} -c main.cpp

user_interface.o: user_interface.cpp user_interface.h \
	explorer.h estimator.h trimaran_interface.h processor.h \
	mem_hierarchy.h environment.h version.h
	${MPICC} -I${GAINC_DIR} ${CFLAGS} -c user_interface.cpp

trimaran_interface.o: trimaran_interface.cpp trimaran_interface.h processor.h
	${MPICC} ${CFLAGS} -c trimaran_interface.cpp

area.o: cacti.h area.c 
	${CC} ${CFLAGS} -c area.c

time.o: cacti.h time.c 
	${CC} ${CFLAGS} -c time.c

parameter.o: parameter.cpp parameter.h 
	${CXX} ${CFLAGS} -c parameter.cpp

common.o: common.cpp common.h 
	${CXX} ${CFLAGS} -c common.cpp

FuzzyApprox.o: FuzzyApprox.cpp FuzzyApprox.h common.h RuleList.h FunctionApprox.h
	${CXX} ${CFLAGS} -c FuzzyApprox.cpp
#	${CXX} ${CFLAGS}  -funroll-loops -ffast-math -c FuzzyApprox.cpp

RuleList.o: RuleList.cpp RuleList.h common.h 
	${CXX} ${CFLAGS} -c RuleList.cpp
#	${CXX} ${CFLAGS} -funroll-loops -ffast-math -c RuleList.cpp

FuzzyWrapper.o: FuzzyWrapper.cpp FuzzyApprox.h
	${CXX} ${CFLAGS} -c FuzzyWrapper.cpp

.PHONY: spea2 clean cleanall

spea2: ${GALIB_DIR}/libspea2.a

${GALIB_DIR}/libspea2.a: ${GASRC_DIR}/*.cpp ${GASRC_DIR}/*.h
	${MAKE} -C ${GASRC_DIR}

clean: 
	rm -f *.o epic *~ core

cleanall: clean
	${MAKE} -C ${GASRC_DIR} clean

# DO NOT DELETE
