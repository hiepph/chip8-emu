#include
#include // OpenGL
#include "chip8.h" // cpu core implementation


chip8 myChip8;


void chip8::initialize() {
  // init registers and memory
  pc = 0x200; // program counter starts at 0x200
  opcode = 0; // reset current opcode
  I = 0; // reset current index register
  sp = 0; // reset stack pointer

  // clear display
  // clear stack
  // clear registesr V0-VF
  // clear memory

  // load fontset
  for (int i = 0; i < 80; ++i) {
    memory[i] = chip8_fontset[i];
  }

  // reset timers

  // load the program into the memory
  for (int i = 0; i < bufferSize; i++) {
    memory[i+512] = buffer[i];
  }
}

void chip8::emulateCycle() {
  // fetch opcode
  // opcode is 2-byte long
  opcode = memory[pc] << 8 | memory[pc + 1];

  // decode
  switch (opcode & 0xF000) {
  case 0x000:
    switch (opcode & 0x000F) {
    case 0x0000: //0x00E0: clears the screen
      // exec
      break;
    case 0x000E: // 0x00EE: returns from the subroutine
      // exec
      break;
    default:
      printf("Unknown opcode [0x0000]: 0x%X\n", opcode);
    }
    break;

  case 0xA000: // ANNN: set I to the address NNN
    // execute
    I = opcode & 0x0FFF; // off first 4 bits
    pc += 2; // 2-byte long instruction
    break;

  case 0x2000: // 0x2NNN: calls the subroutine at NNN
    stack[sp] = pc;
    ++sp;
    pc = opcode & 0x0FFF; // set to address NNN
    break;

  case 0x0004: // 0x8XY4:  Vx += Vy (carry: Vf = 1)
    unsigned short y = (opcode & 0x00F0) >> 4;
    unsigned short x = (opcode & 0x0F00) >> 8;
    if (V[y] > (0xFF - V[x])) {
      V[0xF] = 1; // carry
    } else {
      V[0xF] = 0;
    }
    V[x] += V[y];
    pc += 2;
    break;

  case 0x0033: // 0xFX33: store the binary-coded Vx at I, I+1, I+2
    I = opcode & 0x0FFF;
    unsigned short x =  (opcode & 0x0F00) >> 8;
    memory[I] = V[x] / 100;
    memory[I+1] = (V[x] / 10) % 10;
    memory[I+2] = (V[x] % 100) % 10;
    pc += 2;
    break;

  case 0xD000: // 0xDXYN: draw at (Vx, Vy) with 3-row height
    unsigned short y = (opcode & 0x00F0) >> 4;
    unsigned short x = (opcode & 0x0F00) >> 8;
    unsigned short height = opcode & 0x000F;
    unsigned short pixel;

    V[0xF] = 0;
    for (int yline = 0; yline < height; yline++) {
      pixel = memory[I + yline];
      for (int xline = 0; xline < 8; xline++) {
        // check if current evaluated pixel is set to 1
        if ((pixel & (0x80 >> line)) != 0) {
          if (gfx[(x + xline + ((y + yline) * 64))] == 1) {
            // collision
            V[0xF] = 1;
          }
          // set the pixel value
          gfx[x + xline + ((y + yline) * 64)] ^= 1;
        }
      }
    }

    drawFlag = true;
    pc += 2;
    break;

  case 0xE000:
    switch (opcode & 00FF) {
    case 0x009E:
      // 0xEX9E: skip the next instruction, if the key stored in Vx is pressed
      if (key[V[(opcode & 0x0F00) >> 8]] != 0) {
        pc += 4;
      } else {
        pc += 2;
      }
    }
    break;

  default:
    printf("Unknown opcode: %x%X\n", opcode);
  }

  // update timers
  if (delay_timer > 0) {
    --delay_timer;
  }

  if (sound_timer > 0) {
    if (sound_timer == 1) {
      printf("BEEP!\n");
      --sound_timer;
    }
  }
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

  return 0;
}
