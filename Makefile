CC = gcc
TARGET = tui-raster
CFLAGS = -lm -lX11 -O3
DEBUGFLAGS = -g -fsanitize=address -fno-omit-frame-pointer 

SRC = src/math-utils.c src/term.c src/x11.c src/obj-loader.c src/scene-loader.c src/transformations.c src/render.c src/main.c

build: $(SRC)
	$(CC) $(SRC) -o $(TARGET) $(CFLAGS)

musl: $(SRC)
	x86_64-linux-musl-gcc $(SRC) -o $(TARGET) $(CFLAGS)

debug: $(SRC)
	$(CC) $(SRC) -o $(TARGET) $(CFLAGS) $(DEBUGFLAGS)

clean:
	rm -f $(TARGET)
