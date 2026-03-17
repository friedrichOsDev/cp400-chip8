#include <stdint.h>
#include <stddef.h>
#include <appdef.hpp>
#include <sdk/calc/calc.hpp>
#include <sdk/os/lcd.hpp>
#include <sdk/os/mem.hpp>
#include <sdk/os/file.hpp>
#include <sdk/os/string.hpp>
#include <sdk/os/debug.hpp>
#include <sdk/os/gui.hpp>
#include "roms.hpp"

APP_NAME("CHIP-8 Emulator")
APP_DESCRIPTION("A simple CHIP-8 Emulator")
APP_AUTHOR("FriedrichOsDev")
APP_VERSION("1.0.0")

class ROMLoader : public GUIDialog {
public:
    ROMLoader() : GUIDialog(
		GUIDialog::Height60,
		GUIDialog::AlignCenter,
		"Select ROM",
		GUIDialog::KeyboardStateNone
	), romMenu(
		GetLeftX() + 10,
		GetTopY() + 10,
		GetRightX() - 10,
		GetBottomY() - 10 - 30 - 10,
		ROM_MENU_EVENT_ID
	), loadBtn(
		GetLeftX() + 10,
		GetBottomY() - 10 - 30,
		GetLeftX() + 10 + 100,
		GetBottomY() - 10,
		"Load",
		LOAD_BTN_EVENT_ID
	), cancelBtn(
		GetRightX() - 10 - 100,
		GetBottomY() - 10 - 30,
		GetRightX() - 10,
		GetBottomY() - 10,
		"Cancel",
		CANCEL_BTN_EVENT_ID
	) {
		selectedROM = 0;

		Roms::loadRomList();
		Roms::romlist_t romList = Roms::getRomList();

		for (int i = 0; i < romList.count; i++) {
			romMenu.AddMenuItem(*(new GUIDropDownMenuItem(romList.roms[i].name, i + 1, GUIDropDownMenuItem::FlagEnabled | GUIDropDownMenuItem::FlagTextAlignLeft)));
		}
		
		romMenu.SetScrollBarVisibility(GUIDropDownMenu::ScrollBarVisibleWhenRequired);

		AddElement(romMenu);
		AddElement(loadBtn);
		AddElement(cancelBtn);
	}

    virtual int OnEvent(GUIDialog_Wrapped *dialog, GUIDialog_OnEvent_Data *event) {
        if (event->GetEventID() == ROM_MENU_EVENT_ID && (event->type & 0xF) == 0xD) {
            selectedROM = event->data - 1;
        }

        return GUIDialog::OnEvent(dialog, event);
    }

	Roms::rom_t GetSelectedROM() {
		Roms::romlist_t romList = Roms::getRomList();
		return romList.roms[selectedROM];
	}

private:
    int selectedROM;
    const uint16_t ROM_MENU_EVENT_ID = 1;
	GUIDropDownMenu romMenu;
	const uint16_t LOAD_BTN_EVENT_ID = GUIDialog::DialogResultOK;
	GUIButton loadBtn;
	const uint16_t CANCEL_BTN_EVENT_ID = GUIDialog::DialogResultCancel;
	GUIButton cancelBtn;
};

class Chip8 {
public:
    Chip8() {
		// for (int i = 0; i < 16; i++) {
		// 	v[i] = 0;
		// }
		
		// mem = (uint8_t *)malloc(4096);
		// mem_size = 4096;
		// memset(mem, 0, mem_size);
		
		// display = (uint8_t *)malloc(64 * 32);
		// display_size = 64 * 32;
		// memset(display, 0, display_size);

		// for (int i = 0; i < 16; i++) {
		// 	stack[i] = 0;
		// }
		
		// sp = 0;
		// i = 0;
		// pc = 0x200;
		// delay_timer = 0;
	}

	void loadROM(const char * filename) {
		(void)filename;
	}

    void cycle() {
		fetch();
		decode();
		execute();
	}

private:
    uint8_t v[16]; // V0 - VF
    uint8_t * mem; // pointer to mem
    size_t mem_size; // 4096
    uint8_t * display; // pointer to display
    size_t display_size; // 64x32 pixels
    uint16_t stack[16]; 
    uint16_t sp;
    uint16_t i; // index register
    uint16_t pc; // program counter
    uint8_t delay_timer;

	void fetch() {
		return;
	}

	void decode() {
		return;
	}

	void execute() {
		return;
	}
};

extern "C"
void main() {
	calcInit(); // backup screen and init some variables

	ROMLoader loader;
	GUIDialog::DialogResult result = loader.ShowDialog();

	if (result != GUIDialog::DialogResultOK) {
		calcEnd();
		return;
	}

	Roms::rom_t selectedROM = loader.GetSelectedROM();

	Debug_Printf(0, 0, true, 0, "Loading ROM: %s", selectedROM.path);
	LCD_Refresh();

	Chip8 chip8;

	int ticks = 0;
	while (true) {
		Debug_Printf(0, 1, true, 0, "Ticks: %d", ticks);
		LCD_Refresh();
		chip8.cycle();
		uint32_t key1, key2;
		getKey(&key1, &key2);
		if (testKey(key1, key2, KEY_CLEAR)) {
			break;
		}
		ticks++;
	}

	calcEnd(); // restore screen and do stuff
}