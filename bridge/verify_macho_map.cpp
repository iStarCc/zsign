#include "common.h"

#include "verify_macho_map.h"

#include "verify_archo.h"

#include "archo.h"
#include "mach-o.h"

#include <vector>

namespace {

class ZsignMachOMap {
public:
	ZsignMachOMap() : m_pBase(NULL), m_sSize(0) {}

	~ZsignMachOMap()
	{
		FreeArchOes();
	}

	bool Open(const char* szPath)
	{
		FreeArchOes();

		m_sSize = 0;
		m_pBase = (uint8_t*)ZFile::MapFile(szPath, 0, 0, &m_sSize, false);
		if (NULL == m_pBase) {
			return false;
		}

		uint32_t magic = *((uint32_t*)m_pBase);
		if (FAT_CIGAM == magic || FAT_MAGIC == magic) {
			fat_header* pFatHeader = (fat_header*)m_pBase;
			int nFatArch = (FAT_MAGIC == magic) ? pFatHeader->nfat_arch : LE(pFatHeader->nfat_arch);
			for (int i = 0; i < nFatArch; i++) {
				fat_arch* pFatArch = (fat_arch*)(m_pBase + sizeof(fat_header) + sizeof(fat_arch) * i);
				uint8_t* pArchBase = m_pBase + ((FAT_MAGIC == magic) ? pFatArch->offset : LE(pFatArch->offset));
				uint32_t uArchLength = (FAT_MAGIC == magic) ? pFatArch->size : LE(pFatArch->size);
				if (!NewArchO(pArchBase, uArchLength)) {
					ZLog::ErrorV(">>> Invalid arch file in fat mach-o file!\n");
					return false;
				}
			}
		} else if (MH_MAGIC == magic || MH_CIGAM == magic || MH_MAGIC_64 == magic || MH_CIGAM_64 == magic) {
			if (!NewArchO(m_pBase, (uint32_t)m_sSize)) {
				ZLog::ErrorV(">>> Invalid mach-o file!\n");
				return false;
			}
		} else {
			ZLog::ErrorV(">>> Invalid mach-o file (magic: 0x%08x)!\n", magic);
			return false;
		}

		return !m_arrArchOes.empty();
	}

	const std::vector<ZArchO*>& ArchOs() const
	{
		return m_arrArchOes;
	}

private:
	bool NewArchO(uint8_t* pBase, uint32_t uLength)
	{
		ZArchO* archo = new ZArchO();
		if (archo->Init(pBase, uLength)) {
			m_arrArchOes.push_back(archo);
			return true;
		}
		delete archo;
		return false;
	}

	void FreeArchOes()
	{
		for (size_t i = 0; i < m_arrArchOes.size(); i++) {
			delete m_arrArchOes[i];
		}
		m_arrArchOes.clear();

		if (NULL != m_pBase && m_sSize > 0) {
			(void)ZFile::UnmapFile((void*)m_pBase, m_sSize);
		}
		m_pBase = NULL;
		m_sSize = 0;
	}

	uint8_t* m_pBase;
	size_t m_sSize;
	std::vector<ZArchO*> m_arrArchOes;
};

} // namespace

bool VerifyMachOFileCodeSlots(const char* path)
{
	if (NULL == path || !*path) {
		return false;
	}

	ZsignMachOMap map;
	if (!map.Open(path)) {
		return false;
	}

	const std::vector<ZArchO*>& archOs = map.ArchOs();
	if (archOs.empty()) {
		return false;
	}

	for (size_t i = 0; i < archOs.size(); i++) {
		if (!VerifyArchOCodeSlots(archOs[i], true)) {
			return false;
		}
	}
	return true;
}

bool VerifyMachOFileEmbeddedSignature(const char* path, bool bCheckCMS)
{
	if (NULL == path || !*path) {
		return false;
	}

	ZsignMachOMap map;
	if (!map.Open(path)) {
		return false;
	}

	const std::vector<ZArchO*>& archOs = map.ArchOs();
	if (archOs.empty()) {
		return false;
	}

	for (size_t i = 0; i < archOs.size(); i++) {
		if (!VerifyArchOEmbeddedSignature(archOs[i], bCheckCMS)) {
			return false;
		}
	}
	return true;
}
