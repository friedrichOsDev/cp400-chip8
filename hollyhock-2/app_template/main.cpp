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
#include <sdk/os/input.hpp>
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
			GUIDropDownMenuItem item(romList.roms[i].name, i + 1, GUIDropDownMenuItem::FlagEnabled | GUIDropDownMenuItem::FlagTextAlignLeft);
			romMenu.AddMenuItem(item);
		}
		
		romMenu.SetScrollBarVisibility(GUIDropDownMenu::ScrollBarVisibleWhenRequired);

		AddElement(romMenu);
		AddElement(loadBtn);
		AddElement(cancelBtn);
	}

	virtual ~ROMLoader() {}

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
	int ticks_to_wait_per_loop = 100000;

    Chip8() : mem(nullptr), display(nullptr) {
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

		for (unsigned int i = 0; i < FONTSET_SIZE; i++) {
			mem[FONTSET_START_ADDRESS + i] = fontset[i];
		}

		// Set up function pointer table
		table[0x0] = &Chip8::Table0;
		table[0x1] = &Chip8::OP_1nnn;
		table[0x2] = &Chip8::OP_2nnn;
		table[0x3] = &Chip8::OP_3xkk;
		table[0x4] = &Chip8::OP_4xkk;
		table[0x5] = &Chip8::OP_5xy0;
		table[0x6] = &Chip8::OP_6xkk;
		table[0x7] = &Chip8::OP_7xkk;
		table[0x8] = &Chip8::Table8;
		table[0x9] = &Chip8::OP_9xy0;
		table[0xA] = &Chip8::OP_Annn;
		table[0xB] = &Chip8::OP_Bnnn;
		table[0xC] = &Chip8::OP_Cxkk;
		table[0xD] = &Chip8::OP_Dxyn;
		table[0xE] = &Chip8::TableE;
		table[0xF] = &Chip8::TableF;

		for (size_t i = 0; i <= 0xE; i++) {
			table0[i] = &Chip8::OP_NULL;
			table8[i] = &Chip8::OP_NULL;
			tableE[i] = &Chip8::OP_NULL;
		}

		table0[0x0] = &Chip8::OP_00E0;
		table0[0xE] = &Chip8::OP_00EE;

		table8[0x0] = &Chip8::OP_8xy0;
		table8[0x1] = &Chip8::OP_8xy1;
		table8[0x2] = &Chip8::OP_8xy2;
		table8[0x3] = &Chip8::OP_8xy3;
		table8[0x4] = &Chip8::OP_8xy4;
		table8[0x5] = &Chip8::OP_8xy5;
		table8[0x6] = &Chip8::OP_8xy6;
		table8[0x7] = &Chip8::OP_8xy7;
		table8[0xE] = &Chip8::OP_8xyE;

		tableE[0x1] = &Chip8::OP_ExA1;
		tableE[0xE] = &Chip8::OP_Ex9E;

		for (size_t i = 0; i <= 0x65; i++) {
			tableF[i] = &Chip8::OP_NULL;
		}

		tableF[0x07] = &Chip8::OP_Fx07;
		tableF[0x0A] = &Chip8::OP_Fx0A;
		tableF[0x15] = &Chip8::OP_Fx15;
		tableF[0x18] = &Chip8::OP_Fx18;
		tableF[0x1E] = &Chip8::OP_Fx1E;
		tableF[0x29] = &Chip8::OP_Fx29;
		tableF[0x33] = &Chip8::OP_Fx33;
		tableF[0x55] = &Chip8::OP_Fx55;
		tableF[0x65] = &Chip8::OP_Fx65;

		LCD_GetSize(&LCD_WIDTH, &LCD_HEIGHT);
		ResetDirty();
	}

	~Chip8() {
		FreeMem();
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
		if (size == 0) {
			return -1;
		} else {
			return 0;
		}	
	}

	void UpdateTimers() {
		if (delay_timer > 0) {
			--delay_timer;
		}
	}

    void Cycle() {
		// Fetch
		if (pc >= mem_size - 1) {
			pc = START_ADDRESS;
		}

		opcode = (mem[pc] << 8u) | mem[pc + 1];
		pc += 2;

		// Decode & Execute
		((*this).*(table[(opcode & 0xF000u) >> 12u]))();
	}

	uint8_t GetRandByte() {
		SEED ^= SEED << 13;
		SEED ^= SEED >> 17;
		SEED ^= SEED << 5;
		return (uint8_t)(SEED & 0xFF);
	}

	int SyncWithPlatform() {
		// keys
		uint32_t key1, key2;
		getKey(&key1, &key2);

		keypad[0x1] = testKey(key1, key2, KEY_7);
		keypad[0x2] = testKey(key1, key2, KEY_8);
		keypad[0x3] = testKey(key1, key2, KEY_9);
		keypad[0xC] = testKey(key1, key2, KEY_MULTIPLY);

		keypad[0x4] = testKey(key1, key2, KEY_4);
		keypad[0x5] = testKey(key1, key2, KEY_5);
		keypad[0x6] = testKey(key1, key2, KEY_6);
		keypad[0xD] = testKey(key1, key2, KEY_SUBTRACT);

		keypad[0x7] = testKey(key1, key2, KEY_1);
		keypad[0x8] = testKey(key1, key2, KEY_2);
		keypad[0x9] = testKey(key1, key2, KEY_3);
		keypad[0xE] = testKey(key1, key2, KEY_ADD);

		keypad[0xA] = testKey(key1, key2, KEY_0);
		keypad[0x0] = testKey(key1, key2, KEY_DOT);
		keypad[0xB] = testKey(key1, key2, KEY_EXP);
		keypad[0xF] = testKey(key1, key2, KEY_EXE);

		// display
		if (dirty_flag) {
			for (uint8_t y = display_dirty_y1; y <= display_dirty_y2; y++) {
				for (uint8_t x = display_dirty_x1; x <= display_dirty_x2; x++) {
					uint8_t pixel = display[y * 64 + x];
					uint16_t color = (pixel == 0xFF) ? 0xFFFF : 0x0000;
					
					for (int sy = 0; sy < LCD_PIXEL_SCALE; sy++) {
						if ((y * LCD_PIXEL_SCALE + sy) >= LCD_HEIGHT) continue;
						memset((void*)((uintptr_t)vram + (y * LCD_PIXEL_SCALE + sy) * LCD_WIDTH * 2 + (x * LCD_PIXEL_SCALE) * 2), color, LCD_PIXEL_SCALE * 2);
					}
				}
			}
			LCD_Refresh();
			ResetDirty();
		}

		// exit, if KEY_CLEAR is pressed
		if (testKey(key1, key2, KEY_CLEAR)) return -1;
		return 0;
	}

private:
	int LCD_PIXEL_SCALE = 4;
	int LCD_WIDTH;
	int LCD_HEIGHT;
	uint32_t SEED = 0xACE1u;
	const uint16_t START_ADDRESS = 0x200;
	const uint16_t FONTSET_START_ADDRESS = 0x50;
	static const unsigned int FONTSET_SIZE = 80;
	static const uint8_t fontset[FONTSET_SIZE];

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

	typedef void (Chip8::*Chip8Func)();
	Chip8Func table[0xF + 1];
	Chip8Func table0[0xE + 1];
	Chip8Func table8[0xE + 1];
	Chip8Func tableE[0xE + 1];
	Chip8Func tableF[0x65 + 1];

	uint16_t display_dirty_x1, display_dirty_y1, display_dirty_x2, display_dirty_y2;
	bool dirty_flag;

	void ResetDirty() {
		display_dirty_x1 = 64;
		display_dirty_y1 = 32;
		display_dirty_x2 = 0;
		display_dirty_y2 = 0;
		dirty_flag = false;
	}

	void MarkFullScreenDirty() {
		display_dirty_x1 = 0;
		display_dirty_y1 = 0;
		display_dirty_x2 = 63;
		display_dirty_y2 = 31;
		dirty_flag = true;
	}

	void MarkDirty(uint8_t x, uint8_t y) {
		if (x >= 64 || y >= 32) return;
		if (x < display_dirty_x1) display_dirty_x1 = x;
		if (x > display_dirty_x2) display_dirty_x2 = x;
		if (y < display_dirty_y1) display_dirty_y1 = y;
		if (y > display_dirty_y2) display_dirty_y2 = y;
		dirty_flag = true;
	}

	void Table0() {
		uint8_t op = opcode & 0x000Fu;
		if (op <= 0xE) ((*this).*(table0[op]))();
	}

	void Table8() {
		uint8_t op = opcode & 0x000Fu;
		if (op <= 0xE) ((*this).*(table8[op]))();
	}

	void TableE() {
		uint8_t op = opcode & 0x000Fu;
		if (op <= 0xE) ((*this).*(tableE[op]))();
	}

	void TableF() {
		uint8_t op = opcode & 0x00FFu;
		if (op <= 0x65) ((*this).*(tableF[op]))();
	}
	
	void OP_NULL() {}

	void OP_00E0() {
		memset(display, 0, display_size);
		MarkFullScreenDirty();
	}

	void OP_00EE() {
		if (sp > 0) {
			--sp;
			pc = stack[sp];
		}
	}

	void OP_1nnn() {
		uint16_t address = opcode & 0x0FFFu;
		pc = address;
	}

	void OP_2nnn() {
		uint16_t address = opcode & 0x0FFFu;
		if (sp < 16) {
			stack[sp] = pc;
			++sp;
		}
		pc = address;
	}

	void OP_3xkk() {
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;
		uint8_t byte = opcode & 0x00FFu;
		if (registers[Vx] == byte) {
			pc += 2;
		}
	}

	void OP_4xkk() {
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;
		uint8_t byte = opcode & 0x00FFu;
		if (registers[Vx] != byte) {
			pc += 2;
		}
	}

	void OP_5xy0() {
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;
		uint8_t Vy = (opcode & 0x00F0u) >> 4u;
		if (registers[Vx] == registers[Vy]) {
			pc += 2;
		}
	}

	void OP_6xkk() {
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;
		uint8_t byte = opcode & 0x00FFu;
		registers[Vx] = byte;
	}

	void OP_7xkk() {
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;
		uint8_t byte = opcode & 0x00FFu;
		registers[Vx] += byte;
	}

	void OP_8xy0() {
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;
		uint8_t Vy = (opcode & 0x00F0u) >> 4u;
		registers[Vx] = registers[Vy];
	}

	void OP_8xy1() {
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;
		uint8_t Vy = (opcode & 0x00F0u) >> 4u;
		registers[Vx] |= registers[Vy];
	}

	void OP_8xy2() {
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;
		uint8_t Vy = (opcode & 0x00F0u) >> 4u;
		registers[Vx] &= registers[Vy];
	}

	void OP_8xy3() {
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;
		uint8_t Vy = (opcode & 0x00F0u) >> 4u;
		registers[Vx] ^= registers[Vy];
	}

	void OP_8xy4() {
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;
		uint8_t Vy = (opcode & 0x00F0u) >> 4u;
		uint16_t sum = registers[Vx] + registers[Vy];

		if (sum > 255u) {
			registers[0xF] = 1;
		} else {
			registers[0xF] = 0;
		}

		registers[Vx] = sum & 0xFFu;
	}

	void OP_8xy5() {
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;
		uint8_t Vy = (opcode & 0x00F0u) >> 4u;

		if (registers[Vx] > registers[Vy]) {
			registers[0xF] = 1;
		} else {
			registers[0xF] = 0;
		}

		registers[Vx] -= registers[Vy];
	}

	void OP_8xy6() {
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;

		// Save least significant bit in VF
		registers[0xF] = (registers[Vx] & 0x1u);

		registers[Vx] >>= 1;
	}

	void OP_8xy7() {
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;
		uint8_t Vy = (opcode & 0x00F0u) >> 4u;

		if (registers[Vy] > registers[Vx]) {
			registers[0xF] = 1;
		} else {
			registers[0xF] = 0;
		}

		registers[Vx] = registers[Vy] - registers[Vx];
	}

	void OP_8xyE() {
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;

		// Save most significant bit in VF
		registers[0xF] = (registers[Vx] & 0x80u) >> 7u;

		registers[Vx] <<= 1;
	}

	void OP_9xy0() {
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;
		uint8_t Vy = (opcode & 0x00F0u) >> 4u;

		if (registers[Vx] != registers[Vy]) {
			pc += 2;
		}
	}

	void OP_Annn() {
		uint16_t address = opcode & 0x0FFFu;
		index = address;
	}

	void OP_Bnnn() {
		uint16_t address = opcode & 0x0FFFu;
		pc = registers[0] + address;
	}

	void OP_Cxkk() {
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;
		uint8_t byte = opcode & 0x00FFu;
		registers[Vx] = GetRandByte() & byte;
	}

	void OP_Dxyn() {
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;
		uint8_t Vy = (opcode & 0x00F0u) >> 4u;
		uint8_t height = opcode & 0x000Fu;

		// Wrap if going beyond screen boundaries
		uint8_t xPos = registers[Vx] % 64;
		uint8_t yPos = registers[Vy] % 32;

		registers[0xF] = 0;

		for (unsigned int row = 0; row < height; row++) {
			if (yPos + row >= 32) continue;

			uint8_t spriteByte = (index + row < mem_size) ? mem[index + row] : 0;

			for (unsigned int col = 0; col < 8; col++) {
				if (xPos + col >= 64) continue;

				uint8_t spritePixel = spriteByte & (0x80u >> col);
				uint8_t *screenPixel = &display[(yPos + row) * 64 + (xPos + col)];

				if (spritePixel) {
					// Screen pixel collision
					if (*screenPixel == 0xFF) {
						registers[0xF] = 1;
					}

					// Effectively XOR with the sprite pixel
					*screenPixel ^= 0xFF;
					MarkDirty(xPos + col, yPos + row);
				}
			}
		}
	}

	void OP_Ex9E() {
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;
		uint8_t key = registers[Vx];

		if (key < 16 && keypad[key]) {
			pc += 2;
		}
	}

	void OP_ExA1() {
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;
		uint8_t key = registers[Vx];

		if (key < 16 && !keypad[key]) {
			pc += 2;
		}
	}

	void OP_Fx07() {
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;
		registers[Vx] = delay_timer;
	}

	void OP_Fx0A() {
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;
		bool keyPressed = false;

		for (uint8_t i = 0; i < 16; i++) {
			if (keypad[i]) {
				registers[Vx] = i;
				keyPressed = true;
				break;
			}
		}

		if (!keyPressed) {
			pc -= 2;
		}
	}

	void OP_Fx15() {
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;
		delay_timer = registers[Vx];
	}

	void OP_Fx18() {
		// Sound timer not implemented, but opcode handled
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;
		(void)Vx;
	}

	void OP_Fx1E() {
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;
		index += registers[Vx];
	}

	void OP_Fx29() {
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;
		uint8_t digit = registers[Vx];

		index = FONTSET_START_ADDRESS + (5 * digit);
	}

	void OP_Fx33() {
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;
		uint8_t value = registers[Vx];

		if (index + 2 >= mem_size) return;

		// Ones-place
		mem[index + 2] = value % 10;
		value /= 10;

		// Tens-place
		mem[index + 1] = value % 10;
		value /= 10;

		// Hundreds-place
		mem[index] = value % 10;
	}

	void OP_Fx55() {
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;

		if (index + Vx >= mem_size) return;

		for (uint8_t i = 0; i <= Vx; ++i) {
			mem[index + i] = registers[i];
		}
	}

	void OP_Fx65() {
		uint8_t Vx = (opcode & 0x0F00u) >> 8u;

		if (index + Vx >= mem_size) return;

		for (uint8_t i = 0; i <= Vx; ++i) {
			registers[i] = mem[index + i];
		}
	}
};

const uint8_t Chip8::fontset[Chip8::FONTSET_SIZE] = {
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

extern "C"
const Roms::rom_t * getRomDialog() {
	ROMLoader *loader = new ROMLoader();
	GUIDialog::DialogResult result = loader->ShowDialog();

	const Roms::rom_t * selection = nullptr;
	if (result != GUIDialog::DialogResultOK) {
		selection = nullptr;
	} else {
		selection = loader->GetSelectedROM();
	}

	delete loader;
	return selection;
}

extern "C"
void main() {
	calcInit();

	const Roms::rom_t * selectedROM = getRomDialog();
	if (selectedROM == nullptr) {
		Roms::freeRomList();
		calcEnd();
		return;
	}

	fillScreen(0x0000);
	
	Chip8 *chip8 = new Chip8();
	if (chip8 == nullptr) {
		Roms::freeRomList();
		calcEnd();
		return;
	}

	if (!chip8->init_success) {
		Roms::freeRomList();
		delete chip8;
		calcEnd();
		return;
	}

	int ret = chip8->LoadROM(selectedROM->path);
	if (ret < 0) {
		Debug_Printf(0, 0, true, 0, "Failed to load ROM: %s", selectedROM->path);
		LCD_Refresh();
		Debug_WaitKey();
		Roms::freeRomList();
		delete chip8;
		calcEnd();
		return;
	}

	while (true) {
		for (int i = 0; i < 12; i++) {
			chip8->Cycle();
		}

		chip8->UpdateTimers();
		if (chip8->SyncWithPlatform() < 0) break;

		// we need to some how wait 16.6ms
		for (volatile int j = 0; j < chip8->ticks_to_wait_per_loop; j++) {
			asm volatile("nop");
		}
	}

	Roms::freeRomList();
	delete chip8;
	calcEnd();
}