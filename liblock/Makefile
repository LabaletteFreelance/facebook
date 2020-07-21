BIN= liblock.so libmylock.so

CFLAGS=-Wall -g
LDFLAGS=-ldl

all: $(BIN)

liblock.so: mutex.o cond.o
	$(CC) -shared $^ -o $@ $(LDFLAGS)

mutex.o: mutex.c
	$(CC) -c $^ -fPIC $(CFLAGS)

cond.o: cond.c
	$(CC) -c $^ -fPIC $(CFLAGS)


libmylock.so: my_mutex.o my_cond.o
	$(CC) -shared $^ -o $@ $(LDFLAGS)

my_mutex.o: my_mutex.c
	$(CC) -c $^ -fPIC $(CFLAGS)

my_cond.o: my_cond.c
	$(CC) -c $^ -fPIC $(CFLAGS)

clean:
	rm -f $(BIN) *.o
