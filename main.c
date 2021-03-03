#include
#include // OpenGL
#include "chip8.h" // cpu core implementation


chip8 myChip8;


void chip8::initialize() {
  // init registers and memory
}

void chip8::emulateCycle() {
  // fetch opcode
  // opcode is 2-byte long
  opcode = memory[pc] << 8 | memory[pc + 1];

  // decode

  // execute
  I = opcode & 0x0FFF; // off first 4 bits
  pc += 2; // 2-byte long instruction

  // update timers
}

int main(int argc, char **argv) {
  // setup render system
  setupGraphics();
  // register input callbacks
  setupInput();


  // initialize system
  myChip8.initialize();
  myChip8.loadGame("pong");

  for() {
    myChip8.emulateCycle();

    // redraw screen
    if (myChip8.drawFlag) drawGraphics();

    // remember key state
    myChip8.setKeys();
  }

  unsigned short opcode;
  unsigned char memory[4096];
  unsigned char V[16];
  unsigned short I;
  unsigned short pc;
  unsigned char gfx[64 * 32];
  unsigned char delay_timer;
  unsigned char sound_timer;
  unsigned short stack[16];
  unsigned short sp;
  unsigned char key[16];

  return 0
}
