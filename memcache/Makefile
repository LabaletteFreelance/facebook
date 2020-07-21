CMOCKA_DIR=cmocka
CC = gcc
RM = rm

CFLAGS  = -Wall -g
LDFLAGS = -g -pthread

SOURCES = $(wildcard *.c)
OBJETS  = $(SOURCES:.c=.o)
EXE     = libmemcache.so testsUnitaires

WORKDIR = $(shell pwd)

.PHONY: all clean cleanall run

all: $(EXE)

libmemcache.so: arbreAVL.o memcache.o
	$(CC) -shared $^ -o $@

testsUnitaires: testsUnitaires.o libmemcache.so
	$(CC) -o testsUnitaires testsUnitaires.o -L. -lmemcache -L$(CMOCKA_DIR)/lib -lcmocka $(LDFLAGS)

%.o:%.c
	$(CC) -c $(CFLAGS) -I$(CMOCKA_DIR)/include $< -fPIC

run: testsUnitaires
	LD_LIBRARY_PATH=.:cmocka/lib ./testsUnitaires

gdb: testsUnitaires
	LD_LIBRARY_PATH=.:cmocka/lib gdb --args ./testsUnitaires

valgrind: testsUnitaires
	LD_LIBRARY_PATH=.:cmocka/lib valgrind ./testsUnitaires

clean:
	$(RM) -f *~ *.bak *.tmp \
	         *.o *.s

cleanall: clean
	$(RM) -f $(EXE) $(FIC_POUR_LIV)
