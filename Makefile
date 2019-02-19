CC=gcc
CFLAGS=-I.
LIB_DIR=.

LIBS=-ltsk  
DEPS=estimator.h detector.h deca.h bd.h profiler.h
OBJ=main.o bd.o detector.o deca.o profiler.o estimator.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) 

deca: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) -L$(LIB_DIR) $(LIBS)
