#Makefile for Epic

GAINC_DIR = ./spea2/src
GALIB_DIR = ./spea2/lib
GASRC_DIR = ./spea2/src

ifdef EPIC_MPI
CC = pgcc
CXX = pgCC
MPICC = mpicxx
#MPICC = mpiCC
CFLAGS += -DEPIC_MPI -DMPICH_IGNORE_CXX_SEEK -DNDEBUG -Minform=severe
#CFLAGS += -DEPIC_MPI -DMPICH_IGNORE_CXX_SEEK -DNDEBUG -Minform=warn
else
CC = gcc
CXX = g++
MPICC = ${CXX}
CFLAGS += -DNDEBUG 
#CFLAGS += -O2 -DNDEBUG
#CFLAGS += -g -DDEBUG
endif

all: epic

epic: ${GALIB_DIR}/libspea2.a explorer.o estimator.o area.o time.o processor.o mem_hierarchy.o \
	main.o user_interface.o trimaran_interface.o \
	parameter.o common.o simulate_space.o \
	FuzzyApprox.o RuleList.o FuzzyWrapper.o
	${MPICC} area.o time.o estimator.o explorer.o parameter.o user_interface.o \
	trimaran_interface.o processor.o mem_hierarchy.o main.o common.o \
	FuzzyApprox.o RuleList.o FuzzyWrapper.o simulate_space.o \
	-L${GALIB_DIR} -lspea2 -o epic

estimator.o: estimator.cpp estimator.h processor.h mem_hierarchy.h \
	power_densities.h cacti_area_interface.h 
	${CXX} ${CFLAGS} -c estimator.cpp

explorer.o: explorer.cpp explorer.h processor.h trimaran_interface.h \
	mem_hierarchy.h estimator.h parameter.h common.h \
	FunctionApprox.h FuzzyApprox.h FannApprox.h
	${CXX} -I${GAINC_DIR} ${CFLAGS} -c explorer.cpp

simulate_space.o: simulate_space.cpp explorer.h 
	${MPICC} -I${GAINC_DIR} ${CFLAGS} -c simulate_space.cpp

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

time.o: cacti.h area.c 
	${CC} ${CFLAGS} -c time.c

parameter.o: parameter.cpp parameter.h 
	${CXX} ${CFLAGS} -c parameter.cpp

common.o: common.cpp common.h 
	${CXX} ${CFLAGS} -c common.cpp

FuzzyApprox.o: FuzzyApprox.cpp FuzzyApprox.h common.h RuleList.h FunctionApprox.h
	${CXX} ${CFLAGS} -O3 -c FuzzyApprox.cpp
#	${CXX} ${CFLAGS} -O3 -funroll-loops -ffast-math -c FuzzyApprox.cpp

RuleList.o: RuleList.cpp RuleList.h common.h 
	${CXX} ${CFLAGS} -O3 -c RuleList.cpp
#	${CXX} ${CFLAGS} -O3 -funroll-loops -ffast-math -c RuleList.cpp

FuzzyWrapper.o: FuzzyWrapper.cpp FuzzyApprox.h
	${CXX} ${CFLAGS} -O3 -c FuzzyWrapper.cpp

.PHONY: spea2 clean cleanall

spea2: ${GALIB_DIR}/libspea2.a

${GALIB_DIR}/libspea2.a: ${GASRC_DIR}/*.cpp ${GASRC_DIR}/*.h
	${MAKE} -C ${GASRC_DIR}

clean: 
	rm -f *.o epic *~ core

cleanall: clean
	${MAKE} -C ${GASRC_DIR} clean

# DO NOT DELETE
