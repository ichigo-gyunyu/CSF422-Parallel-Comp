MAKEFLAGS += --silent

CC = gcc
MPICC = mpicc
# CFLAGS = -Wall -pthread
# CFLAGS = -Wall -g -fsanitize=address
CFLAGS = -Wall -g
LDFLAGS =
INC =
OUT = pi_mpi
OBJS = pi_mpi.o

all: $(OUT)

$(OUT): $(OBJS)
	$(MPICC) $(CFLAGS) $(OBJS) -o $(OUT) $(LDFLAGS)

pi_mpi.o: pi_mpi.c
	$(MPICC) $(CFLAGS) $(INC) -c pi_mpi.c

run:
	mpiexec -n 12 --oversubscribe ./$(OUT) 1000000000

clean:
	rm -rf $(OUT) $(OBJS)
