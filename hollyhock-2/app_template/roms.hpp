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

    const int MAX_ROMS = 64;

    void loadRomList();
    romlist_t getRomList();
}