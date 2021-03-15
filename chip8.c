#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "display.h"


#define START_ADDRESS 0x200
#define FONTSET_SIZE 80
#define FONTSET_START_ADDRESS 0x50
#define MEM_SIZE 4096

#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32


const uint8_t fontset[FONTSET_SIZE] = {
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};


typedef struct {
  uint8_t V[16]; // V registers (V0-VF)
  uint8_t memory[4096]; // memory (4K)
  uint16_t I; // index register
  uint16_t pc; // program counter
  uint16_t stack[16]; // stack (16 levels)
  uint8_t sp; // stack pointer
  uint8_t delay_timer;
  uint8_t sound_timer;
  uint8_t keypad[16];
  uint32_t display[64 * 32];
  uint16_t opcode;
} Chip8;


// fallback function which does nothing
void OP_NULL(Chip8* ch8, uint16_t opcode) {
  printf("ERROR: unknown opcode %d\n", opcode);
  exit(1);
}

// table of function pointers
// available leading: 0, 8, E, F
// other unique codes:
// $1nnn
// $2nnn
// $3xnn
// $4xnn
// $5xy0
// $6xnn
// $7xnn
// $9xy0
// $Annn
// $Bnnn
// $Cxnn
// $Dxyn
void (*table[0xF + 1]) (Chip8* ch8, uint16_t opcode);
// leading 0:
// $00E0
// $00EE
void (*table0[0xE + 1]) (Chip8* ch8, uint16_t opcode);
// leading 8:
// $8xy0
// $8xy1
// $8xy2
// $8xy3
// $8xy4
// $8xy5
// $8xy6
// $8xy7
// $8xyE
void (*table8[0xE + 1]) (Chip8* ch8, uint16_t opcode);
// leading E:
// $ExA1
// $Ex9E
void (*tableE[0xE + 1]) (Chip8* ch8, uint16_t opcode);
// leading F:
// $Fx07
// $Fx0A
// $Fx15
// $Fx18
// $Fx1E
// $Fx29
// $Fx33
// $Fx55
// $Fx65
void (*tableF[0x65 + 1]) (Chip8* ch8, uint16_t opcode);

// point to leading table
// & 0x000F: off first 3 bits
void Table0(Chip8* ch8, uint16_t opcode) {
  table0[opcode & 0x000F](ch8, opcode);
}

void Table8(Chip8* ch8, uint16_t opcode) {
  table8[opcode & 0x000F](ch8, opcode);
}

void TableE(Chip8* ch8, uint16_t opcode) {
  tableE[opcode & 0x000F](ch8, opcode);
}

void TableF(Chip8* ch8, uint16_t opcode) {
  tableF[opcode & 0x00FF](ch8, opcode);
}

// clear the screen
void OP_00E0(Chip8* ch8, uint16_t opcode) {
  memset(ch8->display, 0, sizeof(ch8->display));
}

// return from a subroutine
void OP_00EE(Chip8* ch8, uint16_t opcode) {
  // pop from the stack
  --ch8->sp;
  ch8->pc = ch8->stack[ch8->sp];
}

// jump to address nnn
void OP_1nnn(Chip8* ch8, uint16_t opcode) {
  uint16_t nnn = opcode & 0x0FFF;
  ch8->pc = nnn;
}

// call subroutine at nnn
void OP_2nnn(Chip8* ch8, uint16_t opcode) {
  uint16_t nnn = opcode & 0x0FFF;

  // * put the current PC on the top of the stack
  ch8->stack[ch8->sp] = ch8->pc;
  ++ch8->sp;
  ch8->pc = nnn;
}

// skip the next instruction if Vx == nn
void OP_3xnn(Chip8* ch8, uint16_t opcode) {
  uint8_t x = (opcode & 0x0F00) >> 8;
  uint8_t nn = opcode & 0x00FF;
  if (ch8->V[x] == nn) {
    ch8->pc += 2;
  }
}

// skip the next instruction if Vx != nn
void OP_4xnn(Chip8* ch8, uint16_t opcode) {
  uint8_t x = (opcode & 0x0F00) >> 8;
  uint8_t nn = opcode & 0x00FF;
  if (ch8->V[x] != nn) {
    ch8->pc += 2;
  }
}

// skip the next instruction if Vx == Vy
void OP_5xy0(Chip8* ch8, uint16_t opcode) {
  uint8_t x = (opcode & 0x0F00) >> 8;
  uint8_t y = (opcode & 0x00F0) >> 4;
  if (ch8->V[x] == ch8->V[y]) {
    ch8->pc += 2;
  }
}

// Vx = nn
void OP_6xnn(Chip8* ch8, uint16_t opcode) {
  uint8_t x = (opcode & 0x0F00) >> 8;
  uint8_t nn = opcode & 0x00FF;
  ch8->V[x] = nn;
}

// Vx += nn
void OP_7xnn(Chip8* ch8, uint16_t opcode) {
  uint8_t x = (opcode & 0x0F00) >> 8;
  uint8_t nn = opcode & 0x00FF;
  ch8->V[x] += nn;
}

// Vx = Vy
void OP_8xy0(Chip8* ch8, uint16_t opcode) {
  uint8_t x = (opcode & 0x0F00) >> 8;
  uint8_t y = (opcode & 0x00F0) >> 4;
  ch8->V[x] = ch8->V[y];
}

// Vx = Vx | Vy
void OP_8xy1(Chip8* ch8, uint16_t opcode) {
  uint8_t x = (opcode & 0x0F00) >> 8;
  uint8_t y = (opcode & 0x00F0) >> 4;
  ch8->V[x] |= ch8->V[y];
}

// Vx = Vx & Vy
void OP_8xy2(Chip8* ch8, uint16_t opcode) {
  uint8_t x = (opcode & 0x0F00) >> 8;
  uint8_t y = (opcode & 0x00F0) >> 4;
  ch8->V[x] &= ch8->V[y];
}

// Vx = Vx ^ Vy
void OP_8xy3(Chip8* ch8, uint16_t opcode) {
  uint8_t x = (opcode & 0x0F00) >> 8;
  uint8_t y = (opcode & 0x00F0) >> 4;
  ch8->V[x] ^= ch8->V[y];
}

// Vx += Vy
// carry ? Vf = 1 : Vf = 0
void OP_8xy4(Chip8* ch8, uint16_t opcode) {
  uint8_t x = (opcode & 0x0F00) >> 8;
  uint8_t y = (opcode & 0x00F0) >> 4;
  uint16_t sum = ch8->V[x] + ch8->V[y];

  // check if overflow 8 bits
  if (sum > 255) {
    ch8->V[0xF] = 1;
  } else {
    ch8->V[0xF] = 0;
  }

  // keep only lowest 8 bits
  ch8->V[x] = sum & 0xFF;
}

// Vx -= Vy
// borrow ? Vf = 0 : Vf = 1
void OP_8xy5(Chip8* ch8, uint16_t opcode) {
  uint8_t x = (opcode & 0x0F00) >> 8;
  uint8_t y = (opcode & 0x00F0) >> 4;

  if (ch8->V[x] <= ch8->V[y]) {
    ch8->V[0xF] = 0;
  } else {
    ch8->V[0xF] = 1;
  }

  ch8->V[x] -= ch8->V[y];
}

// Stores the least significant bit of VX in VF
// Vx >>= 1
void OP_8xy6(Chip8* ch8, uint16_t opcode) {
  uint8_t x = (opcode & 0x0F00) >> 8;
  ch8->V[0xF] = ch8->V[x] & 0x1;
  ch8->V[x] >>= 1;
}

// Vx = Vy - Vx
// borrow ? Vf = 0 : Vf = 1
void OP_8xy7(Chip8* ch8, uint16_t opcode) {
  uint8_t x = (opcode & 0x0F00) >> 8;
  uint8_t y = (opcode & 0x00F0) >> 4;

  if (ch8->V[y] <= ch8->V[x]) {
    ch8->V[0xF] = 0;
  } else {
    ch8->V[0xF] = 1;
  }

  ch8->V[x] = ch8->V[y] - ch8->V[x];
}

// Stores the most significant bit of VX in VF
// Vx <<= 1
void OP_8xyE(Chip8* ch8, uint16_t opcode) {
  uint8_t x = (opcode & 0x0F00) >> 8;
  ch8->V[0xF] = (ch8->V[x] & 0x80) >> 7;
  ch8->V[x] <<= 1;
}

// skip the next instruction if Vx != Vy
void OP_9xy0(Chip8* ch8, uint16_t opcode) {
  uint8_t x = (opcode & 0x0F00) >> 8;
  uint8_t y = (opcode & 0x00F0) >> 4;
  if (ch8->V[x] != ch8->V[y]) {
    ch8->pc += 2;
  }
}

// I = nnn
void OP_Annn(Chip8* ch8, uint16_t opcode) {
  uint16_t nnn = opcode & 0x0FFF;
  ch8->I = nnn;
}

// PC = V0 + nnn
void OP_Bnnn(Chip8* ch8, uint16_t opcode) {
  uint16_t nnn = opcode & 0x0FFF;
  ch8->pc = ch8->V[0] + nnn;
}

// Vx = rand() & nn
void OP_Cxnn(Chip8* ch8, uint16_t opcode) {
  uint8_t x = (opcode & 0x0F00) >> 8;
  uint8_t nn = opcode & 0x00FF;
  ch8->V[x] = rand() & nn;
}

// draw(Vx, Vy, n)
// Vf = collision
void OP_Dxyn(Chip8* ch8, uint16_t opcode) {
  uint8_t x = (opcode & 0x0F00) >> 8;
  uint8_t y = (opcode & 0x00F0) >> 4;
  uint8_t n = opcode & 0x000F;

  // wrap if going beyond screen boundaries
  uint8_t x_pos = ch8->V[x] % DISPLAY_WIDTH;
  uint8_t y_pos = ch8->V[y] % DISPLAY_HEIGHT;

  ch8->V[0xF] = 0;
  // draw 8x(N+1) sprites
  for (unsigned int row = 0; row < n; ++row) {
    uint8_t sprite_byte = ch8->memory[ch8->I + row];

    for (unsigned int col = 0; col < 8; ++col) {
      uint8_t sprite_pixel = sprite_byte & (0x80 >> col);
      uint32_t* screen_pixel = &ch8->display[(y_pos + row) * DISPLAY_WIDTH + (x_pos + col)];

      if (sprite_pixel) {
        // collision
        if (*screen_pixel == 0xFFFFFFFF) {
          ch8->V[0xF] = 1;
        }

        *screen_pixel ^= 0xFFFFFFFF;
      }
    }
  }
}

// if key() == Vx -> skips the next instruction
void OP_Ex9E(Chip8* ch8, uint16_t opcode) {
  uint8_t x = (opcode & 0x0F00) >> 8;
  uint8_t key = ch8->V[x];
  if (ch8->keypad[key]) {
    ch8->pc += 2;
  }
}

// if key() != Vx -> skips the next instruction
void OP_ExA1(Chip8* ch8, uint16_t opcode) {
  uint8_t x = (opcode & 0x0F00) >> 8;
  uint8_t key = ch8->V[x];
  if (!ch8->keypad[key]) {
    ch8->pc += 2;
  }
}


// Vx = get_delay()
void OP_Fx07(Chip8* ch8, uint16_t opcode) {
  uint8_t x = (opcode & 0x0F00) >> 8;
  ch8->V[x] = ch8->delay_timer;
}

// Vx = get_key()
void OP_Fx0A(Chip8* ch8, uint16_t opcode) {
  printf("NOT YET IMPLEMENTED\n");
}

// delay_timer(Vx)
void OP_Fx15(Chip8* ch8, uint16_t opcode) {
  uint8_t x = (opcode & 0x0F00) >> 8;
  ch8->delay_timer = ch8->V[x];
}

// sound_timer(V[x])
void OP_Fx18(Chip8* ch8, uint16_t opcode) {
  uint8_t x = (opcode & 0x0F00) >> 8;
  ch8->sound_timer = ch8->V[x];
}

// I += V[x]
void OP_Fx1E(Chip8* ch8, uint16_t opcode) {
  uint8_t x = (opcode & 0x0F00) >> 8;
  ch8->I += ch8->V[x];
}

// I = sprite_addr[Vx]
// set I to the location of the sprite for Vx
void OP_Fx29(Chip8* ch8, uint16_t opcode) {
  uint8_t x = (opcode & 0x0F00) >> 8;
  // font characters are located at 0x50, 5 bytes each
  ch8->I = FONTSET_START_ADDRESS + (ch8->V[x] * 5);
}

// set_BCD(Vx)
// *(I+0) = BCD(3);
// *(I+1) = BCD(2);
// *(I+2) = BCD(1);
void OP_Fx33(Chip8* ch8, uint16_t opcode) {
  uint8_t x = (opcode & 0x0F00) >> 8;
  uint8_t vx = ch8->V[x];

  ch8->memory[ch8->I + 2] = vx % 10;
  vx /= 10;
  ch8->memory[ch8->I + 1] = vx % 10;
  vx /= 10;
  ch8->memory[ch8->I] = vx % 10;
}

// reg_dum(Vx, &I)
// store V0 -> Vx in memory starting from index
void OP_Fx55(Chip8* ch8, uint16_t opcode) {
  uint8_t x = (opcode & 0x0F00) >> 8;
  for (uint8_t i = 0; i <= x; ++i) {
    ch8->memory[ch8->I + i] = ch8->V[i];
  }
}

// reg_load(Vx, &I)
// fill V0->Vx with memory starting from index
void OP_Fx65(Chip8* ch8, uint16_t opcode) {
  uint8_t x = (opcode & 0x0F00) >> 8;
  for (uint8_t i = 0; i <= x; ++i) {
    ch8->V[i] = ch8->memory[ch8->I + i];
  }
}

// initialize the Chip8
void initialize(Chip8* ch8) {
  ch8->pc = START_ADDRESS;

  // set black screen
  memset(ch8->display, 0, sizeof(ch8->display));

  // load fonts
  for (uint8_t i = 0; i < FONTSET_SIZE; ++i) {
    ch8->memory[FONTSET_START_ADDRESS + i] = fontset[i];
  }

  // do nothing for unknown opcode
  /* for (uint16_t i = 0; i < sizeof(table); ++i) { */
  /*   table[i] = OP_NULL; */
  /* } */
  /* for (uint16_t i = 0; i < sizeof(table0); ++i) { */
  /*   table0[i] = OP_NULL; */
  /* } */
  /* for (uint16_t i = 0; i < sizeof(table8); ++i) { */
  /*   table8[i] = OP_NULL; */
  /* } */
  /* for (uint16_t i = 0; i < sizeof(tableE); ++i) { */
  /*   tableE[i] = OP_NULL; */
  /* } */
  /* for (uint16_t i = 0; i < sizeof(tableF); ++i) { */
  /*   tableF[i] = OP_NULL; */
  /* } */

  table[0x0] = Table0;
  table[0x1] = OP_1nnn;
  table[0x2] = OP_2nnn;
  table[0x3] = OP_3xnn;
  table[0x4] = OP_4xnn;
  table[0x5] = OP_5xy0;
  table[0x6] = OP_6xnn;
  table[0x7] = OP_7xnn;
  table[0x8] = Table8;
  table[0x9] = OP_9xy0;
  table[0xA] = OP_Annn;
  table[0xB] = OP_Bnnn;
  table[0xC] = OP_Cxnn;
  table[0xD] = OP_Dxyn;
  table[0xE] = TableE;
  table[0xF] = TableF;

  table0[0x0] = OP_00E0;
  table0[0xE] = OP_00EE;

  table8[0x0] = OP_8xy0;
  table8[0x1] = OP_8xy1;
  table8[0x2] = OP_8xy2;
  table8[0x3] = OP_8xy3;
  table8[0x4] = OP_8xy4;
  table8[0x5] = OP_8xy5;
  table8[0x6] = OP_8xy6;
  table8[0x7] = OP_8xy7;
  table8[0xE] = OP_8xyE;

  tableE[0x1] = OP_ExA1;
  tableE[0xE] = OP_Ex9E;

  tableF[0x07] = OP_Fx07;
  tableF[0x0A] = OP_Fx0A;
  tableF[0x15] = OP_Fx15;
  tableF[0x18] = OP_Fx18;
  tableF[0x1E] = OP_Fx1E;
  tableF[0x29] = OP_Fx29;
  tableF[0x33] = OP_Fx33;
  tableF[0x55] = OP_Fx55;
  tableF[0x65] = OP_Fx65;
}

// load application into chip 8
void load_ROM(Chip8* ch8, char const* filename) {
  FILE *file = fopen(filename, "rb");
  fread(ch8->memory+START_ADDRESS, 1, MEM_SIZE - START_ADDRESS, file);
}

void cycle(Chip8* ch8) {
  // fetch opcode
  // opcode is 2-byte long
  // e.g: 0xA2F0
  // << 8: 0xA200, | 0xF0 -> 0xA2F0
  ch8->opcode = (ch8->memory[ch8->pc] << 8) | ch8->memory[ch8->pc + 1];
  printf("0x%04x\n", ch8->opcode);
  ch8->pc += 2;

  // decode + execute
  // e.g.: 0xA2F0
  // & 0xF000: off last 3 bits -> 0xA000
  // >> 12: shift -> 0xA
  table[(ch8->opcode & 0xF000) >> 12](ch8, ch8->opcode);

  // decrease the delay/sound timer if it's been set
  if (ch8->delay_timer > 0) --ch8->delay_timer;
  if (ch8->sound_timer > 0) --ch8->sound_timer;
}


int main(int argc, char *argv[]) {
  // initialize random seed
  srand(time(NULL));

  if (argc != 4) {
    printf("USAGE: %s [scale] [delay] [ROM]\n", argv[0]);
    exit(1);
  }

  uint8_t display_scale = atoi(argv[1]);
  uint8_t delay = atoi(argv[2]);
  char const* rom_filename = argv[3];

  App app;
  memset(&app, 0, sizeof(App));
  initSDL(&app,
          DISPLAY_WIDTH * display_scale, DISPLAY_HEIGHT * display_scale,
          DISPLAY_WIDTH, DISPLAY_HEIGHT);

  Chip8 ch8;
  initialize(&ch8);
  load_ROM(&ch8, rom_filename);

  int display_pitch = sizeof(ch8.display[0]) * DISPLAY_WIDTH;

  clock_t cur_time = clock();
  clock_t last_cycle_time = cur_time;
  float diff_time;
  while (1) {
    // poll for input
    do_input();

    cur_time = clock();
    /* diff_time = (float)(cur_time - last_cycle_time) / CLOCKS_PER_SEC; */
    diff_time = (float)(cur_time - last_cycle_time);
    if (diff_time > delay) {
      cycle(&ch8);
      update_display(&app, &ch8.display, display_pitch);
      last_cycle_time = cur_time;
    }
  }

  cleanup(&app);
  return 0;
}
