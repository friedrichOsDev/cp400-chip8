#pragma once

#include <stdint.h>
#include <stddef.h>
#include <sdk/os/gui.hpp>

class ROMLoader : public GUIDialog {
public:
    int selectedROM;

    ROMLoader();

    virtual int OnEvent(GUIDialog_Wrapped *dialog, GUIDialog_OnEvent_Data *event) {
        if (event->GetEventID() == ROM_MENU_EVENT_ID && (event->type & 0xF) == 0xD) {
            selectedROM = event->data;
        }

        return GUIDialog::OnEvent(dialog, event);
    }

private:
    const uint16_t ROM_MENU_EVENT_ID = 1;
	GUIDropDownMenu romMenu;
	const uint16_t LOAD_BTN_EVENT_ID = GUIDialog::DialogResultOK;
	GUIButton loadBtn;
	const uint16_t CANCEL_BTN_EVENT_ID = GUIDialog::DialogResultCancel;
	GUIButton cancelBtn;
};


class Chip8 {
public:
    Chip8();
	void loadROM(const char * filename);
    void cycle();

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

	void fetch();
	void decode();
	void execute();
};
