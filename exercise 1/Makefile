MAKEFLAGS += --silent

CC = gcc
CFLAGS = -Wall -pthread -g -fsanitize=address
OBJS = $(SRCS:.c=.o)
OUT = pthreads
SRCS = $(wildcard *.c)

build: compile
	$(CC) $(OBJS) $(CFLAGS) -o $(OUT)
	echo "Done! Usage ./$(OUT) x n"

compile:
	$(CC) $(CFLAGS) -c $(SRCS)

readme:
	$(CC) $(CFLAGS) -DREADME $(SRCS) -o $(OUT)

clean:
	rm -rf $(OBJS) $(OUT)
