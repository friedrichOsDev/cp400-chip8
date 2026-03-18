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

	const Roms::rom_t * GetSelectedROM() {
		Roms::romlist_t romList = Roms::getRomList();
		return &romList.roms[selectedROM];
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
	bool init_success;

    Chip8() {
		init_success = true;

		for (int i = 0; i < 16; i++) {
			registers[i] = 0;
		}
		
		mem = (uint8_t *)malloc(4096);
		mem_size = 4096;
		if (mem != nullptr) {
			memset(mem, 0, mem_size);
		} else {
			init_success = false;
			return;
		}		

		index = 0;
		pc = START_ADDRESS;

		for (int i = 0; i < 16; i++) {
			stack[i] = 0;
		}
		sp = 0;
		
		delay_timer = 0;

		for (int i = 0; i < 16; i++) {
			keypad[i] = 0;
		}

		display = (uint8_t *)malloc(64 * 32);
		display_size = 64 * 32;
		if (display != nullptr) {
			memset(display, 0, display_size);
		} else {
			init_success = false;
			return;
		}

		opcode = 0;
	}

	void FreeMem() {
		if (mem != nullptr) {
			free(mem);
			mem = nullptr;
		}
		if (display != nullptr) {
			free(display);
			display = nullptr;
		}
	}

	int LoadROM(const char * path) {
		uint16_t size;
		Roms::loadRom(path, mem + START_ADDRESS, mem_size - START_ADDRESS, &size);
		Debug_Printf(0, 1, true, 0, "Loaded size: %d", size);
		LCD_Refresh();
		if (size == 0) {
			return -1;
		} else {
			return 0;
		}	
	}

    void Cycle() {
		fetch();
		decode();
		execute();
	}

private:
	const uint16_t START_ADDRESS = 0x200;

    uint8_t registers[16]; // V0 - VF
    uint8_t * mem; // pointer to mem
    uint16_t mem_size; // 4096
    uint16_t index; // index register
    uint16_t pc; // program counter
    uint16_t stack[16]; // 16-level stack
    uint8_t sp; // stack pointer
    uint8_t delay_timer;
	uint8_t keypad[16]; // keypad with 16 keys (1 to 9; A to F)
    uint8_t * display; // pointer to display
    uint16_t display_size; // 64x32 pixels
	uint16_t opcode;

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
		Roms::freeRomList();
		calcEnd();
		return;
	}

	const Roms::rom_t * selectedROM = loader.GetSelectedROM();

	Chip8 chip8;

	if (!chip8.init_success) {
		Roms::freeRomList();
		chip8.FreeMem();
		Debug_Printf(0, 10, true, 0, "Failed to initialize Chip8");
		LCD_Refresh();
		while (true) {
			uint32_t key1, key2;
			getKey(&key1, &key2);
			if (testKey(key1, key2, KEY_CLEAR)) {
				break;
			}
		}
		calcEnd();
		return;
	}

	Debug_Printf(0, 0, true, 0, "Loading ROM: %s", selectedROM->path);
	LCD_Refresh();

	int ret = chip8.LoadROM(selectedROM->path);
	if (ret < 0) {
		Roms::freeRomList();
		chip8.FreeMem();
		Debug_Printf(0, 11, true, 0, "Failed to load ROM: %s", selectedROM->path);
		LCD_Refresh();
		while (true) {
			uint32_t key1, key2;
			getKey(&key1, &key2);
			if (testKey(key1, key2, KEY_CLEAR)) {
				break;
			}
		}
		calcEnd();
		return;
	}

	while (true) {
		chip8.Cycle();
		uint32_t key1, key2;
		getKey(&key1, &key2);
		if (testKey(key1, key2, KEY_CLEAR)) {
			break;
		}
	}

	Roms::freeRomList();
	chip8.FreeMem();
	calcEnd(); // restore screen and do stuff
}