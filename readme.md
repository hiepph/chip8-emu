# Chip8 Emulator

![demo](./misc/demo.png)

My Chip8 emulator written in pure C.


## How to run

+ Requirements: *SDL2* for interacting with the keyboard and rendering the display.
For Arch fellows: `pacman -S sdl2`

+ Build: `make`

+ Run: `./chip8 <display scale> <delay in second> /path/to/rom`. Example: `./chip8 10 0.005 ROM/tetris.ch8`


## Reference

+ [Chip8 Wikipedia](https://www.wikiwand.com/en/CHIP-8): Basic
  overview and reference for opcodes.

+ [How to write an emulator (CHIP-8
  interpreter)](http://www.multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/):
  My first ever resource when I was looking for how to implement a
  Chip8 emulator.

+ [Building a CHIP-8 Emulator
  [C++]](https://austinmorlan.com/posts/chip8_emulator/): The author
  explains well the architecture and has a clear implementation in
  C++. You can consider mine is a re-implementation in C.

+ [SDL2 Game
  Tutorials](https://www.parallelrealities.co.uk/tutorials/#shooter):
  I used some of SDL2's examples here.


## Why

I decided to do some fun side projects while learning C at the university.
I needed something hardcore and related to system programming. That's
when my passion for video games spoke out loud! And Chip8 is a
good start.
