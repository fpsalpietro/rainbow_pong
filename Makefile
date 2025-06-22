CC = gcc
CFLAGS = -Wall -O2 `sdl2-config --cflags`
LIBS = `sdl2-config --libs` -lSDL2_ttf -lSDL2_mixer

TARGET = pong
SRC = main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

