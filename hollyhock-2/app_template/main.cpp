#include <appdef.hpp>
#include <sdk/calc/calc.hpp>
#include <sdk/os/lcd.hpp>
#include <sdk/os/mem.hpp>
#include <sdk/os/file.hpp>
#include <sdk/os/string.hpp>
#include <sdk/os/debug.hpp>
#include "main.hpp"

APP_NAME("CHIP-8 Emulator")
APP_DESCRIPTION("A simple CHIP-8 Emulator")
APP_AUTHOR("FriedrichOsDev")
APP_VERSION("1.0.0")

Chip8::Chip8() {
	for (int i = 0; i < 16; i++) {
		v[i] = 0;
	}
	
	mem = (uint8_t *)malloc(4096);
	mem_size = 4096;
	memset(mem, 0, mem_size);
	
	display = (uint8_t *)malloc(64 * 32);
	display_size = 64 * 32;
	memset(display, 0, display_size);

	for (int i = 0; i < 16; i++) {
		stack[i] = 0;
	}
	
	sp = 0;
	i = 0;
	pc = 0x200;
	delay_timer = 0;
}

void Chip8::loadROM(const char * filename) {
	(void)filename;
}

void Chip8::cycle() {
	fetch();
	decode();
	execute();
}

void Chip8::fetch() {

}

void Chip8::decode() {

}

void Chip8::execute() {

}

extern "C"
void main() {
	calcInit(); // backup screen and init some variables

	// load ROM Dialog
	GUIDialog loadROMDialog(
		GUIDialog::Height35,
		GUIDialog::AlignCenter,
		"Select ROM",
		GUIDialog::KeyboardStateNone
	);

	const uint16_t ROM_MENU_EVENT_ID = 1;
	GUIDropDownMenu romMenu(
		loadROMDialog.GetLeftX() + 10,
		loadROMDialog.GetTopY() + 10,
		loadROMDialog.GetRightX() - 10,
		loadROMDialog.GetBottomY() - 10,
		ROM_MENU_EVENT_ID
	);

	const uint16_t LOAD_BTN_EVENT_ID = GUIDialog::DialogResultOK;
	GUIButton loadBtn(
		loadROMDialog.GetLeftX() + 10,
		loadROMDialog.GetTopY() + 45,
		loadROMDialog.GetLeftX() + 10 + 100,
		loadROMDialog.GetBottomY() - 10,
		"Load",
		LOAD_BTN_EVENT_ID
	);

	const uint16_t CANCEL_BTN_EVENT_ID = GUIDialog::DialogResultCancel;
	GUIButton cancelBtn(
		loadROMDialog.GetRightX() - 10 - 100,
		loadROMDialog.GetTopY() + 45,
		loadROMDialog.GetRightX() - 10,
		loadROMDialog.GetBottomY() - 10,
		"Cancel",
		CANCEL_BTN_EVENT_ID
	);

	loadROMDialog.AddElement(romMenu);
	loadROMDialog.AddElement(loadBtn);
	loadROMDialog.AddElement(cancelBtn);

	GUIDialog::DialogResult result = loadROMDialog.ShowDialog();

	if (result != GUIDialog::DialogResultOK) {
		calcEnd();
		return;
	}

	Debug_Printf(10, 10, true, 0, "Selected Load Btn!");
	LCD_Refresh();

	Chip8 chip8;

	while (true) {
		chip8.cycle();
		uint32_t key1, key2;
		getKey(&key1, &key2);
		if (testKey(key1, key2, KEY_CLEAR)) {
			break;
		}
	}

	calcEnd(); // restore screen and do stuff
}