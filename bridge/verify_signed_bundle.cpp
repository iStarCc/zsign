#include "common.h"

#include "verify_signed_bundle.h"

#include "verify_macho_map.h"

#include "json.h"
#include "mach-o.h"

#include <cstdio>
#include <cstring>
#include <vector>

bool VerifySignedBundle(const char* appFolder, bool bCheckCMS)
{
	if (NULL == appFolder || !*appFolder) {
		ZLog::Error(">>> Verify: app folder not set\n");
		return false;
	}

	string strAppFolder = appFolder;
	vector<string> arrMachOFiles;
	ZFile::EnumFolder(strAppFolder.c_str(), true, NULL, [&](bool bFolder, const string& strPath) {
		if (bFolder) {
			return false;
		}
		FILE* fp = fopen(strPath.c_str(), "rb");
		if (fp) {
			uint32_t magic = 0;
			if (1 == fread(&magic, sizeof(magic), 1, fp)) {
				if (magic == MH_MAGIC || magic == MH_CIGAM || magic == MH_MAGIC_64 || magic == MH_CIGAM_64
					|| magic == FAT_MAGIC || magic == FAT_CIGAM) {
					arrMachOFiles.push_back(strPath);
				}
			}
			fclose(fp);
		}
		return false;
	});

	if (arrMachOFiles.empty()) {
		ZLog::Error(">>> Verify: no Mach-O files found\n");
		return false;
	}

	ZLog::PrintV(">>> Verifying all Mach-O embedded signatures (%zu files) ...\n", arrMachOFiles.size());

	uint32_t uPassCount = 0;
	for (const string& strFile : arrMachOFiles) {
		if (!VerifyMachOFileCodeSlots(strFile.c_str())) {
			ZLog::ErrorV(">>> Verify FAILED: %s\n", strFile.c_str());
			return false;
		}
		uPassCount++;
	}

	ZLog::PrintV(">>> All Mach-O embedded verified OK (%u files)\n", uPassCount);

	string strInfoPath = strAppFolder + "/Info.plist";
	string strInfoPlist;
	if (!ZFile::ReadFile(strInfoPath.c_str(), strInfoPlist) || strInfoPlist.empty()) {
		ZLog::Error(">>> Verify: can't read main Info.plist\n");
		return false;
	}

	jvalue jvInfo;
	if (!jvInfo.read_plist(strInfoPlist)) {
		ZLog::Error(">>> Verify: can't parse main Info.plist\n");
		return false;
	}

	string strExecName = jvInfo["CFBundleExecutable"].as_cstr();
	if (strExecName.empty()) {
		ZLog::Error(">>> Verify: missing CFBundleExecutable\n");
		return false;
	}

	string strMainBinary = strAppFolder + "/" + strExecName;
	if (!VerifyMachOFileEmbeddedSignature(strMainBinary.c_str(), bCheckCMS)) {
		ZLog::ErrorV(">>> Verify: main binary embedded signature check FAILED: %s\n", strExecName.c_str());
		return false;
	}

	ZLog::PrintV(">>> Main binary embedded signature verified OK: %s\n", strExecName.c_str());
	return true;
}
