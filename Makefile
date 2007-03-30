 #FLAGS = -DNDEBUG
FLAGS = -DNDEBUG
#FLAGS = -O2 -DNDEBUG
#FLAGS = -g -DDEBUG
GAINC_DIR = -I./MOGA/include
GALIB_DIR = -L./MOGA/lib

all: epic

epic: explorer.o estimator.o area.o time.o processor.o mem_hierarchy.o \
	main.o user_interface.o trimaran_interface.o \
	parameter.o common.o FuzzyApprox.o RuleList.o FuzzyWrapper.o
	g++  area.o time.o estimator.o explorer.o parameter.o user_interface.o \
	trimaran_interface.o processor.o mem_hierarchy.o main.o common.o \
	FuzzyApprox.o RuleList.o FuzzyWrapper.o \
	$(GALIB_DIR) -lmoea -o epic

estimator.o: estimator.cpp estimator.h processor.h mem_hierarchy.h \
	power_densities.h cacti_area_interface.h 
	g++ ${FLAGS} -c estimator.cpp

explorer.o: explorer.cpp explorer.h processor.h trimaran_interface.h \
	mem_hierarchy.h estimator.h parameter.h common.h FunctionApprox.h
	g++ ${GAINC_DIR} ${FLAGS} -c explorer.cpp

processor.o: processor.cpp processor.h parameter.h
	g++ ${FLAGS} -c processor.cpp

mem_hierarchy.o: mem_hierarchy.cpp mem_hierarchy.h parameter.h
	g++ ${FLAGS} -c mem_hierarchy.cpp

main.o: main.cpp user_interface.h
	g++ ${GAINC_DIR} ${FLAGS} -c main.cpp

user_interface.o: user_interface.cpp user_interface.h \
	explorer.h estimator.h trimaran_interface.h processor.h \
	mem_hierarchy.h environment.h version.h
	g++ ${GAINC_DIR} ${FLAGS} -c user_interface.cpp

trimaran_interface.o: trimaran_interface.cpp trimaran_interface.h processor.h
	g++ ${FLAGS} -c trimaran_interface.cpp

area.o: cacti.h area.c 
	gcc ${FLAGS} -c area.c

time.o: cacti.h area.c 
	gcc ${FLAGS} -c time.c

parameter.o: parameter.cpp parameter.h 
	g++ ${FLAGS} -c parameter.cpp

common.o: common.cpp common.h 
	g++ ${FLAGS} -c common.cpp

FuzzyApprox.o: FuzzyApprox.cpp FuzzyApprox.h common.h RuleList.h FunctionApprox.h
	g++ ${FLAGS} -O3 -funroll-loops -ffast-math -c FuzzyApprox.cpp

RuleList.o: RuleList.cpp RuleList.h common.h 
	g++ ${FLAGS} -O3 -funroll-loops -ffast-math -c RuleList.cpp

FuzzyWrapper.o: FuzzyWrapper.cpp FuzzyApprox.h
	g++ ${FLAGS} -O3 -c FuzzyWrapper.cpp

clean: 
	rm -f MOGA/include/moea/*.o
	rm -f MOGA/lib/libmoea.a
	rm -f *.o epic *~ core

# DO NOT DELETE
