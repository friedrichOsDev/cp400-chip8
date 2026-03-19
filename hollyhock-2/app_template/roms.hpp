#pragma once

namespace Roms {
    typedef struct {
        char name[256];
        char path[256];
    } rom_t;

    typedef struct {
        rom_t *roms;
        int count;
    } romlist_t;

    const int MAX_ROMS = 256;

    void loadRomList();
    void freeRomList();
    romlist_t getRomList();
    void loadRom(const char * path, uint8_t *buffer, uint16_t max_size, uint16_t *size);
}