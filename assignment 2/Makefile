MAKEFLAGS += --silent

CC = gcc
MPICC = mpicc
# CFLAGS = -Wall -pthread
# CFLAGS = -Wall -g -fsanitize=address
CFLAGS = -Wall -g
LDFLAGS =
INC =
OUTE = encoder
OUTD = decoder
OBJS = encoder.o \
	   decoder.o \
	   pq.o

all: $(OUTE) $(OUTD)

$(OUTE): encoder.o pq.o
	$(MPICC) $(CFLAGS) encoder.o pq.o -o $(OUTE) $(LDFLAGS)

$(OUTD): decoder.o
	$(MPICC) $(CFLAGS) decoder.o -o $(OUTD) $(LDFLAGS)

encoder.o: encode_parallel.c huffman.h
	$(MPICC) $(CFLAGS) $(INC) -c encode_parallel.c -o encoder.o

decoder.o: decode_parallel.c huffman.h
	$(MPICC) $(CFLAGS) $(INC) -c decode_parallel.c -o decoder.o

pq.o: Utils/pq.c Utils/pq.h
	$(MPICC) $(CFLAGS) $(INC) -c Utils/pq.c -o pq.o

clean:
	rm -rf $(OUTE) $(OUTD) $(OBJS)
