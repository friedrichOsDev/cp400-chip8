#include <appdef.hpp>
#include <sdk/calc/calc.hpp>
#include <sdk/os/lcd.hpp>
#include <sdk/os/debug.hpp>

APP_NAME("CHIP-8 Emulator")
APP_DESCRIPTION("A simple CHIP-8 Emulator")
APP_AUTHOR("FriedrichOsDev")
APP_VERSION("1.0.0")

extern "C"
void main() {
	calcInit(); // backup screen and init some variables
	calcEnd(); // restore screen and do stuff
}