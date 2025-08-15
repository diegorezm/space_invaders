CC = gcc
CFLAGS = -Wall -std=c99 -Iinclude
LDFLAGS = -Lexternal/raylib/src
LIBS = -lraylib -lm -ldl -lpthread -lGL -lrt -lX11

RAYLIB_SRC = external/raylib/src

all: raylib main

raylib:
	$(MAKE) -C $(RAYLIB_SRC) PLATFORM=PLATFORM_DESKTOP
	cp external/raylib/src/*.h include/

main: src/main.c
	$(CC) $(CFLAGS) src/main.c -o main $(LDFLAGS) $(LIBS)

clean:
	$(MAKE) -C $(RAYLIB_SRC) clean
	rm -f main
