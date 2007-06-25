#------------------------------------------------------------------------------------
# Usage: 
# 	make depend   to generate dependence relation.
#                     then copy those relation for debugging version, 
#       make debug    to make debugging version of the library.
#       make          to make optimized version.
#--------------------------------------------------------xianming Chen, March 21, 2001


CC = g++

#set default compiling rule for debugging.

.SUFFIXES: .o .C
.SUFFIXES:.do .C          

%.do:%.C
	$(CC) -DDEBUG -g  -c  $< -o $@  
%.o:%.C
	$(CC) -O2 -DNDEBUG -c  $<  -o $@  



AR= ar rvs

LIB= libmoea.a
D_LIB= libmoea.d.a

HDRS=\
AlleleStr.h        Assign.h           Bin2DecGenome.h    Genome.h           Grid_Loc.h         \
MOEA.h             NPGA.h             NSGA.h             Normalizer.h       ObjectiveVector.h  \
PTSGA.h            PTree.h            PTreeNode.h        ParameterList.h    Pareto.h           \
Population.h       Random.h           Reduction.h        SPEA.h             Select.h           \
VEGA.h             Vector.h           

SRCS=\
AlleleStr.C      Assign.C         Bin2DecGenome.C  Genome.C         Grid_Loc.C       \
MOEA.C           Normalizer.C     PTSGA.C          PTree.C          PTreeNode.C      \
ParameterList.C  Pareto.C         Population.C     Random.C         Reduction.C      \
Select.C         

OBJS=\
AlleleStr.o      Assign.o         Bin2DecGenome.o  Genome.o         Grid_Loc.o       \
MOEA.o           Normalizer.o     PTSGA.o          PTree.o          PTreeNode.o      \
ParameterList.o  Pareto.o         Population.o     Random.o         Reduction.o      \
Select.o         

D_OBJS=\
AlleleStr.do      Assign.do         Bin2DecGenome.do  Genome.do         Grid_Loc.do   \
MOEA.do           Normalizer.do     PTSGA.do          PTree.do          PTreeNode.do   \
ParameterList.do  Pareto.do         Population.do     Random.do         Reduction.do    \
Select.do         


$(LIB): $(OBJS)
	$(AR) $(LIB) $?
	mv $(LIB) ../../lib/
	@echo $(LIB) is now up-to-date

$(D_LIB): $(D_OBJS)
	$(AR) $(D_LIB) $?
	mv $(D_LIB) ../../lib/
	@echo $(D_LIB) is now up-to-date

.c.a:;
.C.a:;

debug : $(D_LIB)

clear:
	rm -f *.o *.do *~ core 

depend:
	makedepend  $(SRCS)

# DO NOT DELETE THIS LINE -- make depend depends on it.

AlleleStr.o: AlleleStr.h /usr/include/assert.h /usr/include/features.h
AlleleStr.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h Random.h
Assign.o: Assign.h Population.h Genome.h AlleleStr.h /usr/include/assert.h
Assign.o: /usr/include/features.h /usr/include/sys/cdefs.h
Assign.o: /usr/include/gnu/stubs.h Random.h ObjectiveVector.h Vector.h
Assign.o: /usr/include/math.h /usr/include/bits/huge_val.h
Assign.o: /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h
Assign.o: Normalizer.h Pareto.h Bin2DecGenome.h MOEA.h /usr/include/stdlib.h
Assign.o: Select.h Reduction.h ParameterList.h
Bin2DecGenome.o: Bin2DecGenome.h Genome.h AlleleStr.h /usr/include/assert.h
Bin2DecGenome.o: /usr/include/features.h /usr/include/sys/cdefs.h
Bin2DecGenome.o: /usr/include/gnu/stubs.h Random.h ObjectiveVector.h Vector.h
Bin2DecGenome.o: /usr/include/math.h /usr/include/bits/huge_val.h
Bin2DecGenome.o: /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h
Bin2DecGenome.o: Normalizer.h Pareto.h
Genome.o: Genome.h AlleleStr.h /usr/include/assert.h /usr/include/features.h
Genome.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h Random.h
Genome.o: ObjectiveVector.h Vector.h /usr/include/math.h
Genome.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
Genome.o: /usr/include/bits/mathcalls.h Normalizer.h Pareto.h Bin2DecGenome.h
Grid_Loc.o: Grid_Loc.h Pareto.h /usr/include/assert.h /usr/include/features.h
Grid_Loc.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
MOEA.o: MOEA.h /usr/include/stdlib.h /usr/include/features.h
MOEA.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h Population.h
MOEA.o: Genome.h AlleleStr.h /usr/include/assert.h Random.h ObjectiveVector.h
MOEA.o: Vector.h /usr/include/math.h /usr/include/bits/huge_val.h
MOEA.o: /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h
MOEA.o: Normalizer.h Pareto.h Bin2DecGenome.h Select.h Assign.h Reduction.h
MOEA.o: ParameterList.h /usr/include/string.h
Normalizer.o: Normalizer.h
PTSGA.o: PTSGA.h MOEA.h /usr/include/stdlib.h /usr/include/features.h
PTSGA.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h Population.h
PTSGA.o: Genome.h AlleleStr.h /usr/include/assert.h Random.h
PTSGA.o: ObjectiveVector.h Vector.h /usr/include/math.h
PTSGA.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
PTSGA.o: /usr/include/bits/mathcalls.h Normalizer.h Pareto.h Bin2DecGenome.h
PTSGA.o: Select.h Assign.h Reduction.h ParameterList.h PTree.h PTreeNode.h
PTSGA.o: Grid_Loc.h
PTree.o: PTSGA.h MOEA.h /usr/include/stdlib.h /usr/include/features.h
PTree.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h Population.h
PTree.o: Genome.h AlleleStr.h /usr/include/assert.h Random.h
PTree.o: ObjectiveVector.h Vector.h /usr/include/math.h
PTree.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
PTree.o: /usr/include/bits/mathcalls.h Normalizer.h Pareto.h Bin2DecGenome.h
PTree.o: Select.h Assign.h Reduction.h ParameterList.h PTree.h PTreeNode.h
PTree.o: Grid_Loc.h
PTreeNode.o: PTreeNode.h /usr/include/stdlib.h /usr/include/features.h
PTreeNode.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h Grid_Loc.h
PTreeNode.o: Pareto.h /usr/include/assert.h Genome.h AlleleStr.h Random.h
PTreeNode.o: ObjectiveVector.h Vector.h /usr/include/math.h
PTreeNode.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
PTreeNode.o: /usr/include/bits/mathcalls.h Normalizer.h
ParameterList.o: ParameterList.h Pareto.h /usr/include/assert.h
ParameterList.o: /usr/include/features.h /usr/include/sys/cdefs.h
ParameterList.o: /usr/include/gnu/stubs.h
Pareto.o: Pareto.h /usr/include/assert.h /usr/include/features.h
Pareto.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
Population.o: Population.h Genome.h AlleleStr.h /usr/include/assert.h
Population.o: /usr/include/features.h /usr/include/sys/cdefs.h
Population.o: /usr/include/gnu/stubs.h Random.h ObjectiveVector.h Vector.h
Population.o: /usr/include/math.h /usr/include/bits/huge_val.h
Population.o: /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h
Population.o: Normalizer.h Pareto.h Bin2DecGenome.h
Random.o: Random.h
Reduction.o: Reduction.h Population.h Genome.h AlleleStr.h
Reduction.o: /usr/include/assert.h /usr/include/features.h
Reduction.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h Random.h
Reduction.o: ObjectiveVector.h Vector.h /usr/include/math.h
Reduction.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
Reduction.o: /usr/include/bits/mathcalls.h Normalizer.h Pareto.h
Reduction.o: Bin2DecGenome.h
Select.o: Select.h Population.h Genome.h AlleleStr.h /usr/include/assert.h
Select.o: /usr/include/features.h /usr/include/sys/cdefs.h
Select.o: /usr/include/gnu/stubs.h Random.h ObjectiveVector.h Vector.h
Select.o: /usr/include/math.h /usr/include/bits/huge_val.h
Select.o: /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h
Select.o: Normalizer.h Pareto.h Bin2DecGenome.h
