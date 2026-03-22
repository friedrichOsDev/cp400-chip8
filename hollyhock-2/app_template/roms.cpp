#include <sdk/os/file.hpp>
#include <sdk/os/mem.hpp>
#include "roms.hpp"

class File {
public:
	File() : m_opened(false), m_fd(-1) {

	}

	~File() {
		if (m_opened) {
			close(m_fd);
		}
	}

	int open(const char *path, int flags) {
		m_fd = ::open(path, flags);
		m_opened = true;
		return m_fd;
	}

	int getAddr(int offset, const void **addr) {
		return ::getAddr(m_fd, offset, addr);
	}

    int getSize(uint32_t *size) {
        struct stat stat_buffer;
		int ret = fstat(m_fd, &stat_buffer);
		if (ret < 0) return ret;

        *size = stat_buffer.fileSize;
        return ret;
    }

	int read(void *buf, int count) {
		return ::read(m_fd, buf, count);
	}

private:
	bool m_opened;
	int m_fd;
};

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
    const char ROM_FOLDER[] = "\\fls0\\chip8\\roms\\";
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

    void freeRomList() {
        if (romList.roms != nullptr) {
            free(romList.roms);
            romList.roms = nullptr;
        }
        romList.count = 0;
    }

    romlist_t getRomList() {
        return romList;
    }

    void loadRom(const char * path, uint8_t *buffer, uint16_t max_size, uint16_t *size) {
        File file; 
        int ret = file.open(path, OPEN_READ);
        if (ret < 0) {
            *size = 0;
            return;
        }

        uint32_t fileSize = 0;
        ret = file.getSize(&fileSize);
        if (ret < 0) {
            *size = 0;
            return;
        }

        if (fileSize > max_size) {
            *size = 0;
            return;
        }

        int readBytes = file.read(buffer, (int)fileSize);
        if (readBytes < 0) {
            *size = 0;
            return;
        }

        *size = (uint16_t)readBytes;
    }
}