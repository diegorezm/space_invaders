CC = gcc
CFLAGS = -Wall -std=c99 -Iinclude
LDFLAGS = -Lexternal/raylib/src
LIBS = -lraylib -lm -ldl -lpthread -lGL -lrt -lX11

CC_WEB = emcc
CFLAGS_WEB = -Wall -std=c99 -Iinclude ./external/raylib/src/libraylib.web.a -Iinclude/raylib.h -L. -Lexternal/raylib/src/libraylib.web.a
LDFLAGS_WEB = -Lexternal/raylib/src
LIBS_WEB = -lraylib -s USE_GLFW=3 -s ASYNCIFY -s WASM=1 -s ALLOW_MEMORY_GROWTH=1

RAYLIB_SRC = external/raylib/src

# emcc -o game.html game.c -Os -Wall ./path-to/libraylib.a -I. -Ipath-to-raylib-h -L. -Lpath-to-libraylib-a -s USE_GLFW=3 -s ASYNCIFY --shell-file path-to/shell.html -DPLATFORM_WEB

all: raylib main

raylib:
	$(MAKE) -C $(RAYLIB_SRC) PLATFORM=PLATFORM_DESKTOP
	cp external/raylib/src/*.h include/

web: raylib_web
	rm -rf dist
	mkdir dist
	emcc src/main.c -o dist/index.html \
			-Os -Wall \
			external/raylib/src/libraylib.web.a \
			-Iinclude -Iexternal/raylib/src \
			-Lexternal/raylib/src \
			-s USE_GLFW=3 \
			-s ASYNCIFY \
			-s WASM=1 \
			-s ALLOW_MEMORY_GROWTH=1 \
			--shell-file ./shell.html \
			--preload-file assets \
			-DPLATFORM_WEB

raylib_web: 
	$(MAKE) -C $(RAYLIB_SRC) clean
	make -C $(RAYLIB_SRC) PLATFORM=PLATFORM_WEB EMSDK_PATH="$EMSDK" -B
	cp external/raylib/src/*.h include/

main: src/main.c
	$(CC) $(CFLAGS) src/main.c -o main $(LDFLAGS) $(LIBS)

clean:
	$(MAKE) -C $(RAYLIB_SRC) clean
	rm -f main
