#include "macho_dylib_overlay.h"

#include "mach-o.h"

#include <cstring>
#include <functional>
#include <set>

namespace {

static uint32_t BO(uint32_t uVal, bool bBigEndian)
{
	return bBigEndian ? LE(uVal) : uVal;
}

static void ListDylibsInArch(uint8_t* pBase, uint32_t uLength, bool bBigEndian, std::vector<std::string>& out)
{
	if (uLength < sizeof(uint32_t)) {
		return;
	}

	uint32_t magic = *(uint32_t*)pBase;
	mach_header* pHeader = NULL;
	uint32_t uHeaderSize = 0;

	if (MH_MAGIC == magic || MH_CIGAM == magic) {
		pHeader = (mach_header*)pBase;
		uHeaderSize = sizeof(mach_header);
	} else if (MH_MAGIC_64 == magic || MH_CIGAM_64 == magic) {
		pHeader = (mach_header*)pBase;
		uHeaderSize = sizeof(mach_header_64);
	} else {
		return;
	}

	if (uLength < uHeaderSize) {
		return;
	}

	uint8_t* pLoadCommand = pBase + uHeaderSize;
	for (uint32_t i = 0; i < BO(pHeader->ncmds, bBigEndian); i++) {
		if ((size_t)(pLoadCommand - pBase) + sizeof(load_command) > uLength) {
			break;
		}
		load_command* plc = (load_command*)pLoadCommand;
		uint32_t cmd = BO(plc->cmd, bBigEndian);
		uint32_t cmdsize = BO(plc->cmdsize, bBigEndian);
		if (cmdsize < sizeof(load_command) || (size_t)(pLoadCommand - pBase) + cmdsize > uLength) {
			break;
		}
		if (LC_LOAD_DYLIB == cmd || LC_LOAD_WEAK_DYLIB == cmd) {
			dylib_command* dlc = (dylib_command*)pLoadCommand;
			uint32_t nameOffset = BO(dlc->dylib.name.offset, bBigEndian);
			if ((size_t)(pLoadCommand - pBase) + nameOffset < uLength) {
				const char* szDyLib = (const char*)(pLoadCommand + nameOffset);
				out.push_back(std::string(szDyLib));
			}
		}
		pLoadCommand += cmdsize;
	}
}

static bool ChangeDylibPathInArch(uint8_t* pBase, uint32_t uLength, bool bBigEndian, const char* oldPath, const char* newPath)
{
	if (uLength < sizeof(uint32_t) || !oldPath || !newPath) {
		return false;
	}

	uint32_t magic = *(uint32_t*)pBase;
	mach_header* pHeader = NULL;
	uint32_t uHeaderSize = 0;

	if (MH_MAGIC == magic || MH_CIGAM == magic) {
		pHeader = (mach_header*)pBase;
		uHeaderSize = sizeof(mach_header);
	} else if (MH_MAGIC_64 == magic || MH_CIGAM_64 == magic) {
		pHeader = (mach_header*)pBase;
		uHeaderSize = sizeof(mach_header_64);
	} else {
		return false;
	}

	if (uLength < uHeaderSize) {
		return false;
	}

	bool pathChanged = false;
	uint32_t oldPathLength = (uint32_t)strlen(oldPath);
	uint32_t newPathLength = (uint32_t)strlen(newPath);
	uint32_t newPathPadding = (8 - newPathLength % 8) % 8;

	uint8_t* pLoadCommand = pBase + uHeaderSize;
	for (uint32_t i = 0; i < BO(pHeader->ncmds, bBigEndian); i++) {
		if ((size_t)(pLoadCommand - pBase) + sizeof(load_command) > uLength) {
			break;
		}
		load_command* plc = (load_command*)pLoadCommand;
		uint32_t cmd = BO(plc->cmd, bBigEndian);
		uint32_t cmdsize = BO(plc->cmdsize, bBigEndian);
		if (cmdsize < sizeof(load_command) || (size_t)(pLoadCommand - pBase) + cmdsize > uLength) {
			break;
		}

		if (LC_LOAD_DYLIB == cmd || LC_LOAD_WEAK_DYLIB == cmd) {
			dylib_command* dlc = (dylib_command*)pLoadCommand;
			uint32_t nameOffset = BO(dlc->dylib.name.offset, bBigEndian);
			const char* szDyLib = (const char*)(pLoadCommand + nameOffset);
			if (strcmp(szDyLib, oldPath) == 0) {
				uint32_t dylibPathOffset = sizeof(dylib_command);
				uint32_t dylibPathSize = newPathLength + newPathPadding;
				if (dylibPathOffset + dylibPathSize > cmdsize) {
					ZLog::Error(">>> Insufficient space to update dylib path!\n");
					return false;
				}
				memcpy(pLoadCommand + dylibPathOffset, newPath, newPathLength);
				memset(pLoadCommand + dylibPathOffset + newPathLength, 0, newPathPadding);
				ZLog::PrintV(">>> Dylib Path Changed: %s -> %s\n", oldPath, newPath);
				pathChanged = true;
			}
		}
		pLoadCommand += cmdsize;
	}

	if (!pathChanged) {
		ZLog::PrintV(">>> Old Dylib Path Not Found: %s\n", oldPath);
	}
	return pathChanged;
}

static bool VisitMachOFile(const char* path, const std::function<bool(uint8_t*, uint32_t, bool)>& visitor)
{
	size_t sSize = 0;
	uint8_t* pBase = (uint8_t*)ZFile::MapFile(path, 0, 0, &sSize, false);
	if (!pBase || sSize < sizeof(uint32_t)) {
		return false;
	}

	bool ok = true;
	uint32_t magic = *(uint32_t*)pBase;
	if (FAT_CIGAM == magic || FAT_MAGIC == magic) {
		fat_header* pFatHeader = (fat_header*)pBase;
		int nFatArch = (FAT_MAGIC == magic) ? pFatHeader->nfat_arch : LE(pFatHeader->nfat_arch);
		for (int i = 0; i < nFatArch; i++) {
			fat_arch* pFatArch = (fat_arch*)(pBase + sizeof(fat_header) + sizeof(fat_arch) * i);
			uint8_t* pArchBase = pBase + ((FAT_MAGIC == magic) ? pFatArch->offset : LE(pFatArch->offset));
			uint32_t uArchLength = (FAT_MAGIC == magic) ? pFatArch->size : LE(pFatArch->size);
			uint32_t archMagic = *(uint32_t*)pArchBase;
			bool bBigEndian = (MH_CIGAM == archMagic || MH_CIGAM_64 == archMagic);
			if (!visitor(pArchBase, uArchLength, bBigEndian)) {
				ok = false;
			}
		}
	} else if (MH_MAGIC == magic || MH_CIGAM == magic || MH_MAGIC_64 == magic || MH_CIGAM_64 == magic) {
		bool bBigEndian = (MH_CIGAM == magic || MH_CIGAM_64 == magic);
		ok = visitor(pBase, (uint32_t)sSize, bBigEndian);
	} else {
		ok = false;
	}

	ZFile::UnmapFile(pBase, sSize);
	return ok;
}

} // namespace

bool MachoBridgeListDylibs(const char* path, std::vector<std::string>& out)
{
	out.clear();
	return VisitMachOFile(path, [&out](uint8_t* pBase, uint32_t uLength, bool bBigEndian) {
		ListDylibsInArch(pBase, uLength, bBigEndian, out);
		return true;
	});
}

bool MachoBridgeChangeDylibPath(const char* path, const char* oldPath, const char* newPath)
{
	ZLog::WarnV(">>> Change DyLib Path: %s -> %s ... \n", oldPath, newPath);
	bool pathChanged = true;
	bool visited = VisitMachOFile(path, [&](uint8_t* pBase, uint32_t uLength, bool bBigEndian) {
		if (!ChangeDylibPathInArch(pBase, uLength, bBigEndian, oldPath, newPath)) {
			ZLog::Error(">>> Failed to change path in one of the architectures!\n");
			pathChanged = false;
		}
		return true;
	});
	if (!visited) {
		return false;
	}
	if (pathChanged) {
		ZLog::Warn(">>> Successfully changed all dylib paths!\n");
	}
	return pathChanged;
}
