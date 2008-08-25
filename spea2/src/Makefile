# Makefile for SPEA2

# library archive creation command
AR = ar ruvs

# compiler and flags
# compile with pgi for MPI versione, gcc otherwise
CXX = g++
CFLAGS += -g -Wall -pedantic

# all object files
SEL_OBJECTS = variator.o selector.o containers.o utility.o

libspea2.a : $(SEL_OBJECTS)
	$(AR) ../lib/$@ $?

# main file only for testing
main :	$(SEL_OBJECTS) spea2.c
	$(CXX) $(CFLAGS) -c spea2.c
	$(CXX) *.o -o $@

variator.o : variator.cpp variator.h containers.h utility.h
	$(CXX) $(CFLAGS) -c variator.cpp

selector.o : selector.cpp selector.h containers.h utility.h
	$(CXX) $(CFLAGS) -c selector.cpp

containers.o : containers.cpp containers.h utility.h
	$(CXX) $(CFLAGS) -c containers.cpp
	
utility.o : utility.c utility.h
	$(CXX) $(CFLAGS) -c utility.c

.PHONY:	clean
	
clean :
	-rm -f *~ *.o
	-rm -f ../lib/*.a
