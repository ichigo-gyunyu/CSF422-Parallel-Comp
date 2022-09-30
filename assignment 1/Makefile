MAKEFLAGS += --silent

CC = gcc
CFLAGS = -fopenmp -lm -lpthread
# CFLAGS = -g -fsanitize=address -fopenmp -lm -lpthread
OUT_B = blelloch
OUT_H = hillis

all: blelloch.o hillis.o
	$(CC) $(CFLAGS) -o $(OUT_B) blelloch.o
	$(CC) $(CFLAGS) -o $(OUT_H) hillis.o
	echo "Done! ./$(OUT_B) or ./$(OUT_H)"

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^

smallh:
	$(CC) $(CFLAGS) -DSMALL -o $(OUT_H) hillis.c

smallb:
	$(CC) $(CFLAGS) -DSMALL -o $(OUT_B) blelloch.c

clean:
	rm -rf *.o $(OUT_B) $(OUT_H)
