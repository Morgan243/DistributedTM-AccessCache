#need to add support for cross compile
COMP=g++
CROSS_PPC=powerpc-linux-g++

#we'll want to try and use C11 threads if the cross compiler can do it
FLAGS=-lpthread -ggdb
OUT=test

x86 : tb_x86

ppc : tb_ppc

tb_x86 : tb.cpp AccessCache_x86.o RWStore_x86.o
	$(COMP) tb.cpp AccessCache.o RWStore.o $(FLAGS) -o $(OUT)

AccessCache_x86.o : AccessCache.cpp AccessCache.h
	$(COMP) -c AccessCache.cpp AccessCache.h $(FLAGS)

RWStore_x86.o: RWStore.cpp RWStore.h
	$(COMP) -c RWStore.cpp RWStore.h $(FLAGS)


tb_ppc : tb.cpp AccessCache_ppc.o RWStore_ppc.o
	$(CROSS_PPC) tb.cpp AccessCache.o RWStore.o $(FLAGS) -o $(OUT)

AccessCache_ppc.o: AccessCache.cpp AccessCache.h
	$(CROSS_PPC) -c AccessCache.cpp AccessCache.h $(FLAGS)

RWStore_ppc.o: RWStore.cpp RWStore.h
	$(CROSS_PPC) -c RWStore.cpp RWStore.h $(FLAGS)

clean :
	rm $(OUT) *.o *.gch
