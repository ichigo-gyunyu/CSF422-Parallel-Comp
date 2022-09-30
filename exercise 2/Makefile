MAKEFLAGS += --silent

CC = gcc
CFLAGS = -Wall -pthread
# CFLAGS = -Wall -g -fsanitize=address -pthread
OUT = barrier_pthreads

all: barrier_linear.o barrier_logarithmic.o barrier_pthreads.o
	$(CC) $(CFLAGS) -o $(OUT) $^
	echo "Done! Usage ./$(OUT) num_threads"

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^

clean:
	rm -rf *.o $(OUT)
