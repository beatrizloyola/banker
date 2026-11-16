CC     = gcc
CFLAGS = -Wall -Wextra -std=c11 -g

all: banker

banker: banker.c
	$(CC) $(CFLAGS) banker.c -o banker

clean:
	rm -f banker

.PHONY: all clean
