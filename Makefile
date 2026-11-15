CC     = gcc
CFLAGS = -Wall -Wextra -std=c11 -g

all: banker

banker: main.c
	$(CC) $(CFLAGS) main.c -o banker

clean:
	rm -f banker

.PHONY: all clean
