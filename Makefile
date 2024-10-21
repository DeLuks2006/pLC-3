CC = gcc
CFLAGS = -Wall -Wshadow -Wextra
IN = src/*.c
OUT = pLC-3

all:
	$(CC) $(IN) -o $(OUT) $(CFLAGS)

debug:
	$(CC) $(IN) -o DEBUG-$(OUT) $(CFLAGS) -g

clean:
	rm $(OUT)

cursed:
	clang $(IN) -o CLANG-$(OUT) $(CFLAGS) -Ofast
	strip CLANG-$(OUT)
