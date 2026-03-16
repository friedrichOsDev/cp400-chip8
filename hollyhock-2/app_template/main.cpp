#include <appdef.hpp>
#include <sdk/calc/calc.hpp>
#include <sdk/os/lcd.hpp>
#include <sdk/os/mem.hpp>
#include "main.hpp"

APP_NAME("CHIP-8 Emulator")
APP_DESCRIPTION("A simple CHIP-8 Emulator")
APP_AUTHOR("FriedrichOsDev")
APP_VERSION("1.0.0")

extern "C"
void main() {
	calcInit(); // backup screen and init some variables
	calcEnd(); // restore screen and do stuff
}

extern "C"
void initChip8() {
  chip8_hardware_t hw;
  memset(&hw, 0, sizeof(chip8_hardware_t));
  
  hw.mem = malloc(4096);
  hw.mem_size = 4096;
  memset(hw.mem, 0, hw.mem_size);
  
  hw.display = malloc(32 * 64);
  hw.display_size = 32 * 64;
  memset(hw.display, 0, hw.display_size);
  
  hw.pc = 0x200;
}