CC = gcc

TARGET = tui-raster

CFLAGS = -O3
LIBS = -lm
X11LIB = -lX11

DEBUGFLAGS = -g -fsanitize=address -fno-omit-frame-pointer 

SRC = src/math-utils.c src/keyboard.c src/term.c src/x11.c src/obj-loader.c src/scene-loader.c src/transformations.c src/render.c src/main.c
SRCNOX11 = src/math-utils.c src/keyboard.c src/term.c src/obj-loader.c src/scene-loader.c src/transformations.c src/render.c src/main.c

.PHONY: build debug clean no-x11 install uninstall

build: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LIBS) $(X11LIB)

no-x11: $(SRCNOX11)
	$(CC) $(CFLAGS) -DNO_X11 $(SRCNOX11) -o $(TARGET) $(LIBS)

debug: $(SRC)
	$(CC) $(CFLAGS) $(DEBUGFLAGS) $(SRC) -o $(TARGET) $(LIBS) $(X11LIB)

clean:
	rm -f $(TARGET)

install: $(TARGET)
	sudo cp $(TARGET) /usr/bin/$(TARGET)

uninstall:
	sudo rm -f /usr/bin/$(TARGET)
