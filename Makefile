CC = gcc
CFLAGS = -Wall -std=c99 -Iinclude
LDFLAGS = -Lexternal/raylib/src
LIBS = -lraylib -lm -ldl -lpthread -lGL -lrt -lX11

CC_WEB = emcc
CFLAGS_WEB = -Wall -std=c99 -Iinclude -Iexternal/raylib/src
LDFLAGS_WEB = -Lexternal/raylib/src
LIBS_WEB = -lraylib -s USE_GLFW=3 -s ASYNCIFY -s WASM=1 -s ALLOW_MEMORY_GROWTH=1

RAYLIB_SRC = external/raylib/src

all: raylib main

raylib:
	$(MAKE) -C $(RAYLIB_SRC) PLATFORM=PLATFORM_DESKTOP
	cp external/raylib/src/*.h include/

web: raylib_web
	$(CC_WEB) $(CFLAGS_WEB) src/main.c -o dist/index.html $(LDFLAGS_WEB) $(LIBS_WEB) \
		--shell-file ./shell.html \
		--preload-file assets

raylib_web:
	# $(MAKE) -C $(RAYLIB_SRC) clean
	$(MAKE) -C $(RAYLIB_SRC) PLATFORM=PLATFORM_WEB

main: src/main.c
	$(CC) $(CFLAGS) src/main.c -o main $(LDFLAGS) $(LIBS)

clean:
	$(MAKE) -C $(RAYLIB_SRC) clean
	rm -f main
