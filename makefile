#need to add support for cross compile
COMP=g++

#we'll want to try and use C11 threads if the cross compiler can do it
FLAGS=-lpthread --std=c++11 -ggdb
OUT=test


all : tb.cpp AccessCache.o RWStore.o
	$(COMP) tb.cpp AccessCache.o RWStore.o $(FLAGS) -o $(OUT)

AccessCache.o : AccessCache.cpp AccessCache.h
	$(COMP) -c AccessCache.cpp AccessCache.h $(FLAGS)

RWStore.o: RWStore.cpp RWStore.h
	$(COMP) -c RWStore.cpp RWStore.h $(FLAGS)

clean :
	rm $(OUT) *.o *.gch
