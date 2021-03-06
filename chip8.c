struct chip8 {
  unsigned short V[16]; // V registers (V0-VF)
  unsigned char memory[4096]; // memory (4K)
  unsigned short I; // index register
  unsigned short pc; // program counter
  unsigned short stack[16]; // stack (16 levels)
  unsigned short sp; // stack pointer
  unsigned char delay_timer;
  unsigned char sound_timer;
  unsigned char keypad[16];
  unsigned char display[64 * 32];
  unsigned short opcode;
};

// load application into chip 8
void load_ROM(struct chip8* ch8, char const* filename) {

}
