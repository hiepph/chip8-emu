TARGET := chip8

CXXFLAGS += `sdl2-config --cflags`
CXXFLAGS += -g -lefence

LDFLAGS += `sdl2-config --libs` -lSDL2_mixer -lSDL2_image -lSDL2_ttf -lm

build:
	gcc -o $(TARGET) chip8.c $(LDFLAGS)

clean:
	rm -f $(TARGET)
