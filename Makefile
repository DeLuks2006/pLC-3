CC = gcc
CFLAGS = -Wall -Wshadow -Wextra
IN = src/main.c src/utils.c
OUT = pLC-3

all:
	$(CC) $(IN) -o $(OUT) $(CFLAGS)

debug:
	$(CC) $(IN) -o $(OUT) $(CFLAGS) -g

clean:
	rm $(OUT)
