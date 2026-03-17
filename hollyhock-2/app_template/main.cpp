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

ROMLoader::ROMLoader() : GUIDialog(
	GUIDialog::Height35,
	GUIDialog::AlignCenter,
	"Select ROM",
	GUIDialog::KeyboardStateNone
), romMenu(
	GetLeftX() + 10,
	GetTopY() + 10,
	GetRightX() - 10,
	GetBottomY() - 10,
	ROM_MENU_EVENT_ID
), loadBtn(
	GetLeftX() + 10,
	GetTopY() + 45,
	GetLeftX() + 10 + 100,
	GetBottomY() - 10,
	"Load",
	LOAD_BTN_EVENT_ID
), cancelBtn(
	GetRightX() - 10 - 100,
	GetTopY() + 45,
	GetRightX() - 10,
	GetBottomY() - 10,
	"Cancel",
	CANCEL_BTN_EVENT_ID
) {
	selectedROM = 1;

	romMenu.AddMenuItem(*(new GUIDropDownMenuItem("Test 1", 1, GUIDropDownMenuItem::FlagEnabled | GUIDropDownMenuItem::FlagTextAlignLeft)));
	romMenu.AddMenuItem(*(new GUIDropDownMenuItem("Test 2", 2, GUIDropDownMenuItem::FlagEnabled | GUIDropDownMenuItem::FlagTextAlignLeft)));
	romMenu.AddMenuItem(*(new GUIDropDownMenuItem("Test 3", 3, GUIDropDownMenuItem::FlagEnabled | GUIDropDownMenuItem::FlagTextAlignLeft)));
	romMenu.AddMenuItem(*(new GUIDropDownMenuItem("Test 4", 4, GUIDropDownMenuItem::FlagEnabled | GUIDropDownMenuItem::FlagTextAlignLeft)));

	romMenu.SetScrollBarVisibility(GUIDropDownMenu::ScrollBarVisibleWhenRequired);

	AddElement(romMenu);
	AddElement(loadBtn);
	AddElement(cancelBtn);
}

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

	ROMLoader loader;

	GUIDialog::DialogResult result = loader.ShowDialog();

	if (result != GUIDialog::DialogResultOK) {
		calcEnd();
		return;
	}

	Debug_Printf(10, 10, true, 0, "Selected ROM: %d", loader.selectedROM);
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