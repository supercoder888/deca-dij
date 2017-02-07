CC=gcc
#CFLAGS=-I.
CFLAGS=-I. -I/Users/pavelgladyshev/include/
#LIB_DIR=.
LIB_DIR=/Users/pavelgladyshev/lib/

# on Linux:
#LIBS=-ltm -ltsk 

# on Mac:
LIBS=-ltsk  
DEPS=estimator.h detector.h deca.h bd.h profiler.h
OBJ=main.o bd.o detector.o deca.o profiler.o estimator.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) 

deca: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) -L$(LIB_DIR) $(LIBS)
