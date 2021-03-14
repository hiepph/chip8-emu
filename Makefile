TARGET := chip8

LDFLAGS += `sdl2-config --libs` -lm

build:
	gcc -o $(TARGET) chip8.c $(LDFLAGS)

clean:
	rm -f $(TARGET)
