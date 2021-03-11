#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "display.h"


#define START_ADDRESS 0x200
#define FONTSET_SIZE 80
#define FONTSET_START_ADDRESS 0x50

#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32


const uint8_t fontset[FONTSET_SIZE] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, //0
    0x20, 0x60, 0x20, 0x20, 0x70, //1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
    0x90, 0x90, 0xF0, 0x10, 0x10, //4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
    0xF0, 0x10, 0x20, 0x40, 0x40, //7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
    0xF0, 0x90, 0xF0, 0x90, 0x90, //A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
    0xF0, 0x80, 0x80, 0x80, 0xF0, //C
    0xE0, 0x90, 0x90, 0x90, 0xE0, //D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
    0xF0, 0x80, 0xF0, 0x80, 0x80  //F
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
void OP_NULL(Chip8* ch8, uint16_t opcode) {}

// table of function pointers
// available leading: 0, 8, E, F
// other unique codes:
// $1nnn
// $2nnn
// $3xkk
// $4xkk
// $5xy0
// $6xkk
// $7xkk
// $9xy0
// $Annn
// $Bnnn
// $Cxkk
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
/*
void (*table8[0xE + 1]) () = {OP_NULL};
// leading E:
// $ExA1
// $Ex9E
void (*tableE[0xE + 1]) () = {OP_NULL};
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
void (*tableF[0x65 + 1]) () = {OP_NULL};
*/

// point to leading table
// & 0x000F: off first 3 bits
void Table0(Chip8* ch8, uint16_t opcode) {
  table0[opcode & 0x000F](ch8, opcode);
}

/*
void Table8(uint16_t opcode) {
  return table8[opcode & 0x000F]();
}

void TableE(uint16_t opcode) {
  return tableE[opcode & 0x000F]();
}

void TableF(uint16_t opcode) {
  return tableF[opcode & 0x000F]();
}
*/

// clear the screen
void OP_00E0(Chip8* ch8, uint16_t opcode) {
  memset(ch8->display, 0, sizeof(ch8->display));
}

void OP_6xkk(Chip8* ch8, uint16_t opcode) {
}

// initialize the Chip8
void initialize(Chip8* ch8) {
  ch8->pc = START_ADDRESS;

  // load fonts
  for (uint8_t i = 0; i < FONTSET_SIZE; ++i) {
    ch8->memory[FONTSET_START_ADDRESS + i] = fontset[i];
  }

  // do nothing for unknown opcode
  for (uint16_t i = 0; i <= sizeof(table); ++i) {
    table[i] = OP_NULL;
  }
  for (uint16_t i = 0; i <= sizeof(table0); ++i) {
    table0[i] = OP_NULL;
  }

  table[0x0] = Table0;
  /* table[0x1] = OP_1nnn; */
  /* table[0x2] = OP_2nn; */
  /* table[0x3] = OP_3xkk; */
  /* table[0x4] = OP_4xkk; */
  table[0x6] = OP_6xkk;
  /* table[0x8] = Table8; */
  /* table[0xE] = TableE; */
  /* table[0xF] = TableF; */

  table0[0x0] = OP_00E0;
  /* table0[0xE] = OP_00EE; */

  /* table8[0x0] = OP_8xy0; */
  /* table8[0x1] = OP_8xy1; */
  /* table8[0xE] = OP_8xyE; */

  /* tableE[0x1] = OP_ExA1; */
  /* tableE[0xE] = OP_Ex9E; */

  /* tableF[0x07] = OP_Fx07; */
  /* tableF[0x15] = OP_Fx15; */
  /* tableF[0x65] = OP_Fx65; */
}

// load application into chip 8
void load_ROM(Chip8* ch8, char const* filename) {
  FILE *file = fopen(filename, "r");
  long i = 0;
  char c;
  while ((c = fgetc(file)) != EOF) {
    ch8->memory[START_ADDRESS + i] = c;
    i++;
  }
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

  time_t last_cycle_time, cur_time;
  float diff_time;
  time(&last_cycle_time);
  while (1) {
    // poll for input
    doInput();

    time(&cur_time);
    diff_time = difftime(cur_time, last_cycle_time);
    if (diff_time > delay) {
      last_cycle_time = cur_time;
      cycle(&ch8);
      update_display(&app, &ch8.display, display_pitch);
    }
  }

  cleanup(&app);
  return 0;
}
