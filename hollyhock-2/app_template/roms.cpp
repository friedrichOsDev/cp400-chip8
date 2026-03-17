#include <sdk/os/file.hpp>
#include <sdk/os/mem.hpp>
#include "roms.hpp"

class Find {
public:
	Find() : m_opened(false), m_findHandle(-1) {

	}

	~Find() {
		if (m_opened) {
			findClose(m_findHandle);
		}
	}

	int findFirst(const wchar_t *path, wchar_t *name, struct findInfo *findInfoBuf) {
		int ret = ::findFirst(path, &m_findHandle, name, findInfoBuf);
		m_opened = true;
		return ret;
	}

	int findNext(wchar_t *name, struct findInfo *findInfoBuf) {
		return ::findNext(m_findHandle, name, findInfoBuf);
	}

private:
	bool m_opened;
	int m_findHandle;
};

namespace Roms {
    const char ROM_FOLDER[] = "\\\\fls0\\chip8\\roms\\";
    const char FILE_MASK[] = "*.ch8";

    romlist_t romList;

    void loadRomList() {
        romList.roms = (rom_t *)malloc(sizeof(rom_t) * MAX_ROMS);
        romList.count = 0;

        Find finder;
        wchar_t wName[256];
        wchar_t wPath[256];
        struct findInfo info;

        int i = 0;
        while (ROM_FOLDER[i] != 0) {
            wPath[i] = (wchar_t)ROM_FOLDER[i];
            i++;
        }
        int j = 0;
        while (FILE_MASK[j] != 0) {
            wPath[i] = (wchar_t)FILE_MASK[j];
            i++; j++;
        }
        wPath[i] = 0;

        int ret = finder.findFirst(wPath, wName, &info);
        while (ret >= 0) {
            if (info.type == info.EntryTypeFile) {
                rom_t *currentRom = &romList.roms[romList.count];

                int k = 0;
                while (wName[k] != 0 && k < 255) {
                    currentRom->name[k] = (char)wName[k];
                    k++;
                }
                currentRom->name[k] = 0;

                int folderLen = 0;
                while (ROM_FOLDER[folderLen] != 0 && folderLen < 255) {
                    currentRom->path[folderLen] = ROM_FOLDER[folderLen];
                    folderLen++;
                }

                int nameIdx = 0;
                while (currentRom->name[nameIdx] != 0 && (folderLen + nameIdx) < 255) {
                    currentRom->path[folderLen + nameIdx] = currentRom->name[nameIdx];
                    nameIdx++;
                }
                currentRom->path[folderLen + nameIdx] = 0;

                romList.count++;
                if (romList.count >= MAX_ROMS) break;
            }
            ret = finder.findNext(wName, &info);
        }
    }

    romlist_t getRomList() {
        return romList;
    }
}