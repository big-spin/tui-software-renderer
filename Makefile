CC = gcc

TARGET = tui-raster

CFLAGS = -O3
LIBS = -lm
X11LIB = -lX11

DEBUGFLAGS = -g -fsanitize=address -fno-omit-frame-pointer 

SRC = src/math-utils.c src/keyboard.c src/term.c src/x11.c src/obj-loader.c src/scene-loader.c src/transformations.c src/render.c src/main.c
SRCNOX11 = src/math-utils.c src/keyboard.c src/term.c src/obj-loader.c src/scene-loader.c src/transformations.c src/render.c src/main.c

.PHONY: build debug clean no-x11

build: $(SRC)
	$(CC) $(SRC) -o $(TARGET) $(CFLAGS) $(LIBS) $(X11LIB)

no-x11: $(SRCNOX11)
	$(CC) $(SRCNOX11) -o $(TARGET) $(CFLAGS) $(LIBS) -DNO_X11

debug: $(SRC)
	$(CC) $(SRC) -o $(TARGET) $(CFLAGS) $(LIBS) $(X11LIB) $(DEBUGFLAGS)

clean:
	rm -f $(TARGET)
