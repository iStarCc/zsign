#include "common.h"
#include "log_keys.h"
#include <list>
#include "macho.h"
#include "bundle.h"
#include "openssl.h"
#include "timer.h"
#include "archive.h"
#include "metadata.h"
#include "certcheck.h"
#include "json.h"

#ifdef _WIN32
#include "common_win32.h"
#endif

#define _ZSIGN_STR(x) #x
#define _ZSIGN_XSTR(x) _ZSIGN_STR(x)

#ifndef ZSIGN_VERSION
#define ZSIGN_VERSION 0.9.6
#endif

#define ZSIGN_VERSION_STR _ZSIGN_XSTR(ZSIGN_VERSION)

const struct option options[] = {
	{"debug", no_argument, NULL, 'd'},
	{"force", no_argument, NULL, 'f'},
	{"verbose", no_argument, NULL, 'V'},
	{"adhoc", no_argument, NULL, 'a'},
	{"cert", required_argument, NULL, 'c'},
	{"pkey", required_argument, NULL, 'k'},
	{"prov", required_argument, NULL, 'm'},
	{"password", required_argument, NULL, 'p'},
	{"bundle_id", required_argument, NULL, 'b'},
	{"bundle_name", required_argument, NULL, 'n'},
	{"bundle_version", required_argument, NULL, 'r'},
	{"entitlements", required_argument, NULL, 'e'},
	{"output", required_argument, NULL, 'o'},
	{"zip_level", required_argument, NULL, 'z'},
	{"dylib", required_argument, NULL, 'l'},
	{"rm_dylib", required_argument, NULL, 'D'},
	{"weak", no_argument, NULL, 'w'},
	{"temp_folder", required_argument, NULL, 't'},
	{"sha256_only", no_argument, NULL, '2'},
	{"install", no_argument, NULL, 'i'},
	{"check", no_argument, NULL, 'C'},
	{"quiet", no_argument, NULL, 'q'},
	{"metadata", required_argument, NULL, 'x'},
	{"rm_provision", no_argument, NULL, 'R'},
	{"enable_docs", no_argument, NULL, 'S'},
	{"min_version", required_argument, NULL, 'M'},
	{"rm_extensions", no_argument, NULL, 'E'},
	{"rm_watch", no_argument, NULL, 'W'},
	{"rm_uisd", no_argument, NULL, 'U'},
	{"list_dylibs", no_argument, NULL, 'L'},
	{"inject_only", no_argument, NULL, 'I'},
	{"remove_only", no_argument, NULL, 'J'},
	{"no_remove_files", no_argument, NULL, 256},
	{"locale", required_argument, NULL, 'y'},
	{"help", no_argument, NULL, 'h'},
	{}
};

int usage()
{
	ZLog::PrintV("zsign (v%s) is a codesign alternative for iOS12+ on macOS, Linux and Windows. \nVisit https://github.com/zhlynn/zsign for more information.\n\n", ZSIGN_VERSION_STR);
	ZLog::Print("Usage: zsign [-options] [-k privkey.pem] [-m dev.prov] [-o output.ipa] file|folder\n");
	ZLog::Print("options:\n");
	ZLog::Print("-k, --pkey\t\tPath to private key or p12 file. (PEM or DER format)\n");
	ZLog::Print("-m, --prov\t\tPath to mobile provisioning profile.\n");
	ZLog::Print("-c, --cert\t\tPath to certificate file. (PEM or DER format)\n");
	ZLog::Print("-a, --adhoc\t\tPerform ad-hoc signature only.\n");
	ZLog::Print("-d, --debug\t\tGenerate debug output files. (.zsign_debug folder)\n");
	ZLog::Print("-f, --force\t\tForce sign without cache when signing folder.\n");
	ZLog::Print("-o, --output\t\tPath to output ipa file.\n");
	ZLog::Print("-p, --password\t\tPassword for private key or p12 file.\n");
	ZLog::Print("-b, --bundle_id\t\tNew bundle id to change.\n");
	ZLog::Print("-n, --bundle_name\tNew bundle name to change.\n");
	ZLog::Print("-r, --bundle_version\tNew bundle version to change.\n");
	ZLog::Print("-e, --entitlements\tNew entitlements to change.\n");
	ZLog::Print("-z, --zip_level\t\tCompressed level when output the ipa file. (0-9)\n");
	ZLog::Print("-l, --dylib\t\tInject dylib. Format: -l \"install_path\" or -l \"install_path=source_path\" (= optional). Use -l multiple times.\n");
	ZLog::Print("-D, --rm_dylib\t\tName or path of dylib to remove (e.g. OldLib.dylib or @rpath/OldLib.dylib). Use -D multiple times.\n");
	ZLog::Print("-w, --weak\t\tInject dylib as LC_LOAD_WEAK_DYLIB (default when not adhoc).\n");
	ZLog::Print("-i, --install\t\tInstall ipa file using ideviceinstaller command for test.\n");
	ZLog::Print("-t, --temp_folder\tPath to temporary folder for intermediate files.\n");
	ZLog::Print("-2, --sha256_only\tSerialize a single code directory that uses SHA256.\n");
	ZLog::Print("-C, --check\t\tCheck certificate validity and OCSP revocation status.\n");
	ZLog::Print("-q, --quiet\t\tQuiet operation.\n");
	ZLog::Print("-x, --metadata\t\tExtract metadata and icon to the specified directory.\n");
	ZLog::Print("-R, --rm_provision\tRemove mobileprovision file after signing.\n");
	ZLog::Print("-S, --enable_docs\tEnable UISupportsDocumentBrowser and UIFileSharingEnabled.\n");
	ZLog::Print("-M, --min_version\tSet MinimumOSVersion in Info.plist.\n");
	ZLog::Print("-E, --rm_extensions\tRemove all app extensions (PlugIns/Extensions).\n");
	ZLog::Print("-W, --rm_watch\t\tRemove watch app from the bundle.\n");
	ZLog::Print("-U, --rm_uisd\t\tRemove UISupportedDevices from Info.plist.\n");
	ZLog::Print("-L, --list_dylibs\tList dylibs injected into the main executable.\n");
	ZLog::Print("-I, --inject_only\tInject dylib only (no signing). Use -l \"install_path\" or -l \"install_path=source_path\".\n");
	ZLog::Print("-J, --remove_only\tRemove dylib only (no signing). Use -D for dylib name or full path (e.g. @rpath/OldLib.dylib).\n");
	ZLog::Print("    --no_remove_files\tWhen removing dylibs, only remove load commands, do not delete dylib files.\n");
	ZLog::Print("-y, --locale\t\tLanguage: zh (简体中文) or en (English). Default: en\n");
	ZLog::Print("-v, --version\t\tShows version.\n");
	ZLog::Print("-h, --help\t\tShows help (this message).\n");

	return -1;
}

static bool ResolveExecutablePath(const string& strPath, const string& strTempFolder, string& strExecPath, string* pStrFolderToClean = NULL)
{
	string strResolvePath = strPath;
	if (ZFile::IsZipFile(strPath.c_str())) {
		string strFolder = ZFile::GetRealPathV("%s/zsign_list_%llu", strTempFolder.c_str(), ZUtil::GetMicroSecond());
		if (!ZFile::CreateFolder(strFolder.c_str())) return false;
		if (pStrFolderToClean) *pStrFolderToClean = strFolder;
		if (!Zip::Extract(strPath.c_str(), strFolder.c_str())) return false;
		strResolvePath = strFolder;
	}
	if (ZFile::IsFolder(strResolvePath.c_str())) {
		string strAppFolder;
		if (ZFile::IsPathSuffix(strResolvePath, ".app") || ZFile::IsPathSuffix(strResolvePath, ".appex")) {
			strAppFolder = strResolvePath;
		} else {
			ZFile::EnumFolder(strResolvePath.c_str(), true, [](bool, const string&) { return false; },
				[&](bool bFolder, const string& strP) {
					if (bFolder && (ZFile::IsPathSuffix(strP, ".app") || ZFile::IsPathSuffix(strP, ".appex"))) {
						strAppFolder = strP;
						return true;
					}
					return false;
				});
		}
		if (strAppFolder.empty()) return false;
		string strInfoData;
		if (!ZFile::ReadFile((strAppFolder + "/Info.plist").c_str(), strInfoData)) return false;
		jvalue jvInfo;
		if (!jvInfo.read_plist(strInfoData)) return false;
		string strExe = jvInfo["CFBundleExecutable"].as_cstr();
		if (strExe.empty()) return false;
		strExecPath = strAppFolder + "/" + strExe;
		return ZFile::IsFileExists(strExecPath.c_str());
	}
	strExecPath = strResolvePath;
	return true;
}

int main(int argc, char* argv[])
{
	ZTimer atimer;
	ZTimer gtimer;

	bool bForce = false;
	bool bInstall = false;
	bool bWeakInject = true;  // 默认 LC_LOAD_WEAK_DYLIB；-a (adhoc) 时改为 LC_LOAD_DYLIB
	bool bAdhoc = false;
	bool bSHA256Only = false;
	bool bCheckSignature = false;
	bool bRemoveProvision = false;
	bool bEnableDocuments = false;
	string strMinVersion;
	bool bRemoveExtensions = false;
	bool bRemoveWatchApp = false;
	bool bRemoveUISupportedDevices = false;
	bool bListDylibs = false;
	uint32_t uZipLevel = 0;

	string strCertFile;
	string strPKeyFile;
	string strProvFile;
	vector<string> arrProvFiles;
	string strPassword;
	string strBundleId;
	string strBundleVersion;
	string strOutputFile;
	string strOutputFileDisplay;  // 用户传入的原始路径，用于日志显示（避免输出绝对路径）
	string strDisplayName;
	string strEntitleFile;
	vector<string> arrDylibFiles;
	vector<string> arrRemoveDylibNames;
	string strMetadataDir;
	string strTempFolder = ZFile::GetTempFolder();

	int opt = 0;
	int argslot = -1;
	bool bInjectOnly = false;
	bool bRemoveOnly = false;
	bool bRemoveDylibFiles = true;
	while (-1 != (opt = getopt_long(argc, argv, "dfva2hiqwCRSIJc:k:m:o:p:e:b:n:z:l:D:t:r:x:M:E:W:ULy:",
		options, &argslot))) {
		switch (opt) {
		case 'd':
			ZLog::SetLogLever(ZLog::E_DEBUG);
			break;
		case 'f':
			bForce = true;
			break;
		case 'c':
			strCertFile = ZFile::GetFullPath(optarg);
			break;
		case 'k':
			strPKeyFile = ZFile::GetFullPath(optarg);
			break;
		case 'm':
			strProvFile = ZFile::GetFullPath(optarg);
			arrProvFiles.push_back(strProvFile);
			break;
		case 'a':
			bAdhoc = true;
			break;
		case 'p':
			strPassword = optarg;
			break;
		case 'b':
			strBundleId = optarg;
			break;
		case 'r':
			strBundleVersion = optarg;
			break;
		case 'n':
			strDisplayName = optarg;
			break;
		case 'e':
			strEntitleFile = ZFile::GetFullPath(optarg);
			break;
		case 'l':
			// 格式: "install_path" 或 "install_path=source_path"（仅支持 @executable_path/ 或 @rpath/）
			if (optarg[0] == '@') {
				arrDylibFiles.push_back(optarg);
			} else {
				ZLog::Error("zsign: -l 仅支持 @executable_path/ 或 @rpath/ 格式，如 -l \"@rpath/MyLib.dylib\" 或 -l \"@rpath/MyLib.dylib=/path/to/MyLib.dylib\"\n");
				return -1;
			}
			break;
		case 'D':
			arrRemoveDylibNames.push_back(optarg);
			break;
		case 'i':
			bInstall = true;
			break;
		case 'o':
			strOutputFile = ZFile::GetFullPath(optarg);
			strOutputFileDisplay = optarg;
			break;
		case 'z':
			uZipLevel = atoi(optarg);
			break;
		case 'w':
			bWeakInject = true;
			break;
		case 't':
			strTempFolder = ZFile::GetFullPath(optarg);
			break;
		case '2':
			bSHA256Only = true;
			break;
		case 'C':
			bCheckSignature = true;
			break;
		case 'q':
			ZLog::SetLogLever(ZLog::E_NONE);
			break;
		case 'x':
			strMetadataDir = ZFile::GetFullPath(optarg);
			break;
		case 'R':
			bRemoveProvision = true;
			break;
		case 'S':
			bEnableDocuments = true;
		case 'M':
			strMinVersion = optarg;
		case 'E':
			bRemoveExtensions = true;
		case 'W':
			bRemoveWatchApp = true;
		case 'U':
			bRemoveUISupportedDevices = true;
			break;
		case 'L':
			bListDylibs = true;
			break;
		case 'I':
			bInjectOnly = true;
			break;
		case 'J':
			bRemoveOnly = true;
			break;
		case 256:
			bRemoveDylibFiles = false;
			break;
		case 'y':
			ZL10n::SetLocale(optarg);
			break;
		case 'v': {
			printf("version: %s\n", ZSIGN_VERSION_STR);
			return 0;
			}
			break;
		case 'h':
		case '?':
			return usage();
			break;
		}

		ZLog::DebugV(ZL10n::GetFmt(ZL10nKeys::DEBUG_OPTION), opt, optarg ? optarg : "");
	}

	if (optind >= argc) {
		return usage();
	}

	if (!ZFile::IsFolder(strTempFolder.c_str())) {
		ZLog::ErrorV(ZL10n::GetFmt(ZL10nKeys::INVALID_TEMP_FOLDER), strTempFolder.c_str());
		return -1;
	}

	string strPath = ZFile::GetFullPath(argv[optind]);
	if (!ZFile::IsFileExists(strPath.c_str())) {
		ZLog::ErrorV(ZL10n::GetFmt(ZL10nKeys::INVALID_PATH), strPath.c_str());
		return -1;
	}

	if (uZipLevel < 0 || uZipLevel > 9) {
		ZLog::Error(ZL10n::GetFmt(ZL10nKeys::INVALID_ZIP_LEVEL));
		return -1;
	}

	if (ZLog::IsDebug()) {
		ZFile::CreateFolder("./.zsign_debug");
		for (int i = optind; i < argc; i++) {
			ZLog::DebugV(ZL10n::GetFmt(ZL10nKeys::DEBUG_ARGUMENT), argv[i]);
		}
	}

	if (bCheckSignature && strPKeyFile.empty() && strProvFile.empty()) {
		return CheckCertificate(strPath, strPassword);
	}

	// Standalone inject-only (参考 zsign-swift injectDyLib / injectDylibs)
	if (bInjectOnly) {
		if (arrDylibFiles.empty()) {
			ZLog::Error("zsign: -I/--inject_only requires -l. Use -l \"@rpath/MyLib.dylib\" or -l \"@rpath/MyLib.dylib=/path/to/MyLib.dylib\".\n");
			return -1;
		}
		bool bZipFile = ZFile::IsZipFile(strPath.c_str());
		if (bZipFile && strOutputFile.empty()) {
			ZLog::Error(ZL10n::GetFmt(ZL10nKeys::OUTPUT_PATH_REQUIRED));
			return -1;
		}
		string strExecPath;
		string strFolderToClean;
		string strWorkFolder = strPath;
		if (bZipFile) {
			strWorkFolder = ZFile::GetRealPathV("%s/zsign_inject_%llu", strTempFolder.c_str(), ZUtil::GetMicroSecond());
			if (!ZFile::CreateFolder(strWorkFolder.c_str())) return -1;
			strFolderToClean = strWorkFolder;
			if (!Zip::Extract(strPath.c_str(), strWorkFolder.c_str())) {
				ZLog::Error(ZL10n::GetFmt(ZL10nKeys::UNZIP_FAILED));
				ZFile::RemoveFolder(strWorkFolder.c_str());
				return -1;
			}
		}
		if (!ResolveExecutablePath(strWorkFolder, strTempFolder, strExecPath, NULL)) {
			ZLog::ErrorV(ZL10n::GetFmt(ZL10nKeys::FAILED_RESOLVE_EXEC_PATH), strPath.c_str());
			if (!strFolderToClean.empty()) ZFile::RemoveFolder(strFolderToClean.c_str());
			return -1;
		}
		// -a: LC_LOAD_DYLIB; -w: LC_LOAD_WEAK_DYLIB; -a 优先于 -w
		bool bEffectiveWeakInject = bWeakInject && !bAdhoc;
		string strAppFolder = strExecPath.substr(0, strExecPath.rfind('/'));
		ZMachO macho;
		if (!macho.Init(strExecPath.c_str())) {
			ZLog::ErrorV(ZL10n::GetFmt(ZL10nKeys::INVALID_MACHO_PATH), strExecPath.c_str());
			if (!strFolderToClean.empty()) ZFile::RemoveFolder(strFolderToClean.c_str());
			return -1;
		}
		const size_t lenExec = 17, lenRpath = 7;
		for (const string& dyLibSpec : arrDylibFiles) {
			string strLoadPath;
			string strSourceFile;
			size_t eqPos = dyLibSpec.find('=');
			if (eqPos != string::npos && eqPos > 1) {
				// 字典格式: install_path=source_path (与 zsign-swift injectDylibs 一致)
				strLoadPath = dyLibSpec.substr(0, eqPos);
				strSourceFile = ZFile::GetFullPath(dyLibSpec.substr(eqPos + 1).c_str());
			} else {
				strLoadPath = dyLibSpec;
			}
			if (!strSourceFile.empty()) {
				string relPath;
				if (strLoadPath.find("@executable_path/") == 0) {
					relPath = strLoadPath.substr(lenExec);
				} else if (strLoadPath.find("@rpath/") == 0) {
					string afterRpath = strLoadPath.substr(lenRpath);
					relPath = (afterRpath.find("Frameworks/") == 0) ? afterRpath : string("Frameworks/") + afterRpath;
				}
				if (!relPath.empty()) {
					string strDestFile = strAppFolder + "/" + relPath;
					size_t lastSlash = strDestFile.rfind('/');
					if (lastSlash != string::npos && lastSlash > strAppFolder.size()) {
						ZFile::CreateFolder(strDestFile.substr(0, lastSlash).c_str());
					}
					if (!ZFile::CopyFileV(strSourceFile.c_str(), "%s/%s", strAppFolder.c_str(), relPath.c_str())) {
						ZLog::ErrorV(ZL10n::GetFmt(ZL10nKeys::FAILED_COPY_APP), strSourceFile.c_str());
						macho.Free();
						if (!strFolderToClean.empty()) ZFile::RemoveFolder(strFolderToClean.c_str());
						return -1;
					}
				}
			}
			if (!macho.InjectDylib(bEffectiveWeakInject, strLoadPath.c_str())) {
				macho.Free();
				if (!strFolderToClean.empty()) ZFile::RemoveFolder(strFolderToClean.c_str());
				return -1;
			}
		}
		macho.Free();
		if (bZipFile) {
			if (!Zip::Archive(strWorkFolder.c_str(), strOutputFile.c_str(), uZipLevel)) {
				ZLog::Error(ZL10n::GetFmt(ZL10nKeys::ARCHIVE_FAILED));
				ZFile::RemoveFolder(strFolderToClean.c_str());
				return -1;
			}
			ZFile::RemoveFolder(strFolderToClean.c_str());
		}
		ZLog::Print(ZL10n::Get(ZL10nKeys::DONE));
		return 0;
	}

	// Standalone remove-only (参考 zsign-swift removeDylibs)
	if (bRemoveOnly) {
		if (arrRemoveDylibNames.empty()) {
			ZLog::Error("zsign: -J/--remove_only requires -D. Use -D \"@rpath/OldLib.dylib\" or -D OldLib.dylib.\n");
			return -1;
		}
		bool bZipFile = ZFile::IsZipFile(strPath.c_str());
		if (bZipFile && strOutputFile.empty()) {
			ZLog::Error(ZL10n::GetFmt(ZL10nKeys::OUTPUT_PATH_REQUIRED));
			return -1;
		}
		string strExecPath;
		string strFolderToClean;
		string strWorkFolder = strPath;
		if (bZipFile) {
			strWorkFolder = ZFile::GetRealPathV("%s/zsign_remove_%llu", strTempFolder.c_str(), ZUtil::GetMicroSecond());
			if (!ZFile::CreateFolder(strWorkFolder.c_str())) return -1;
			strFolderToClean = strWorkFolder;
			if (!Zip::Extract(strPath.c_str(), strWorkFolder.c_str())) {
				ZLog::Error(ZL10n::GetFmt(ZL10nKeys::UNZIP_FAILED));
				ZFile::RemoveFolder(strWorkFolder.c_str());
				return -1;
			}
		}
		if (!ResolveExecutablePath(strWorkFolder, strTempFolder, strExecPath, NULL)) {
			ZLog::ErrorV(ZL10n::GetFmt(ZL10nKeys::FAILED_RESOLVE_EXEC_PATH), strPath.c_str());
			if (!strFolderToClean.empty()) ZFile::RemoveFolder(strFolderToClean.c_str());
			return -1;
		}
		string strAppFolder = strExecPath.substr(0, strExecPath.rfind('/'));
		set<string> setRemoveDylibs;
		for (const string& name : arrRemoveDylibNames) {
			if (name[0] == '@') {
				setRemoveDylibs.insert(name);
			} else {
				setRemoveDylibs.insert("@executable_path/" + name);
			}
		}
		ZMachO macho;
		if (!macho.Init(strExecPath.c_str())) {
			ZLog::ErrorV(ZL10n::GetFmt(ZL10nKeys::INVALID_MACHO_PATH), strExecPath.c_str());
			if (!strFolderToClean.empty()) ZFile::RemoveFolder(strFolderToClean.c_str());
			return -1;
		}
		macho.RemoveDylibs(setRemoveDylibs);
		if (bRemoveDylibFiles) {
			const size_t lenExec = 17, lenRpath = 7;
			for (const string& name : setRemoveDylibs) {
				string baseName;
				if (name.find("@executable_path/") == 0) {
					baseName = name.substr(lenExec);
				} else if (name.find("@rpath/") == 0) {
					string afterRpath = name.substr(lenRpath);
					baseName = (afterRpath.find("Frameworks/") == 0) ? afterRpath : string("Frameworks/") + afterRpath;
				} else {
					continue;
				}
				if (!baseName.empty()) {
					ZFile::RemoveFileV("%s/%s", strAppFolder.c_str(), baseName.c_str());
				}
			}
		}
		macho.Free();
		if (bZipFile) {
			if (!Zip::Archive(strWorkFolder.c_str(), strOutputFile.c_str(), uZipLevel)) {
				ZLog::Error(ZL10n::GetFmt(ZL10nKeys::ARCHIVE_FAILED));
				ZFile::RemoveFolder(strFolderToClean.c_str());
				return -1;
			}
			ZFile::RemoveFolder(strFolderToClean.c_str());
		}
		ZLog::Print(ZL10n::Get(ZL10nKeys::DONE));
		return 0;
	}

	if (bListDylibs) {
		string strExecPath;
		string strFolderToClean;
		if (!ResolveExecutablePath(strPath, strTempFolder, strExecPath, &strFolderToClean)) {
			ZLog::ErrorV(ZL10n::GetFmt(ZL10nKeys::FAILED_RESOLVE_EXEC_PATH), strPath.c_str());
			return -1;
		}
		ZMachO macho;
		if (!macho.Init(strExecPath.c_str())) {
			ZLog::ErrorV(ZL10n::GetFmt(ZL10nKeys::INVALID_MACHO_PATH), strExecPath.c_str());
			if (!strFolderToClean.empty()) ZFile::RemoveFolder(strFolderToClean.c_str());
			return -1;
		}
		vector<string> dylibs = macho.ListDylibs();
		ZLog::PrintV(ZL10n::GetFmt(ZL10nKeys::DYLIBS_IN), strExecPath.c_str());
		for (const string& d : dylibs) {
			ZLog::PrintV(ZL10n::GetFmt(ZL10nKeys::DYLIB_LIST_ITEM), d.c_str());
		}
		macho.Free();
		if (!strFolderToClean.empty()) ZFile::RemoveFolder(strFolderToClean.c_str());
		return 0;
	}

	bool bZipFile = ZFile::IsZipFile(strPath.c_str());
	if (!bZipFile && !ZFile::IsFolder(strPath.c_str())) { // macho file
		ZMachO* macho = new ZMachO();
		if (!macho->Init(strPath.c_str())) {
			ZLog::ErrorV(ZL10n::GetFmt(ZL10nKeys::INVALID_MACHO_PATH), strPath.c_str());
			return -1;
		}

		if (!bAdhoc && arrDylibFiles.empty() && (strPKeyFile.empty() || strProvFile.empty())) {
			macho->PrintInfo();
			return 0;
		}

		ZSignAsset zsa;
		if (!zsa.Init(strCertFile, strPKeyFile, strProvFile, strEntitleFile, strPassword, bAdhoc, bSHA256Only, true)) {
			return -1;
		}

		if (!arrDylibFiles.empty()) {
			bool bEffectiveWeakInject = bWeakInject && !bAdhoc;
			for (const string& dyLibFile : arrDylibFiles) {
				if (!macho->InjectDylib(bEffectiveWeakInject, dyLibFile.c_str())) {
					return -1;
				}
			}
		}

		atimer.Reset();
		ZLog::PrintV(ZL10n::GetFmt(ZL10nKeys::SIGNING), strPath.c_str(), (bAdhoc ? ZL10n::Get(ZL10nKeys::ADHOC) : ""));
		string strInfoSHA1;
		string strInfoSHA256;
		string strCodeResourcesData;
		bool bRet = macho->Sign(&zsa, bForce, strBundleId, strInfoSHA1, strInfoSHA256, strCodeResourcesData);
		atimer.PrintResult(bRet, ZL10n::GetFmt(ZL10nKeys::SIGNED_FMT), bRet ? ZL10n::Get(ZL10nKeys::SIGNED_OK) : ZL10n::Get(ZL10nKeys::SIGNED_FAILED));
		return bRet ? 0 : -1;
	}

	bool bTempOutputFile = false;
	if (strOutputFile.empty()) {
		if (bInstall) {
			bTempOutputFile = true;
			strOutputFile = ZFile::GetRealPathV("%s/zsign_temp_%llu.ipa", strTempFolder.c_str(), ZUtil::GetMicroSecond());
		} else if (bZipFile) {
			ZLog::Error(ZL10n::GetFmt(ZL10nKeys::OUTPUT_PATH_REQUIRED));
			return -1;
		}
	}

	//init
	ZSignAsset zsa;
	if (!zsa.Init(strCertFile, strPKeyFile, strProvFile, strEntitleFile, strPassword, bAdhoc, bSHA256Only, false)) {
		return -1;
	}

	//extract
	bool bTempFolder = false;
	bool bEnableCache = true;
	string strFolder = strPath;
	if (bZipFile) {
		bForce = true;
		bTempFolder = true;
		bEnableCache = false;
		strFolder = ZFile::GetRealPathV("%s/zsign_folder_%llu", strTempFolder.c_str(), atimer.Reset());
		ZLog::PrintV(ZL10n::GetFmt(ZL10nKeys::UNZIP), strPath.c_str(), ZFile::GetFileSizeString(strPath.c_str()).c_str(), strFolder.c_str());
		if (!Zip::Extract(strPath.c_str(), strFolder.c_str())) {
			ZLog::Error(ZL10n::GetFmt(ZL10nKeys::UNZIP_FAILED));
			return -1;
		}
		atimer.PrintResult(true, ZL10n::Get(ZL10nKeys::UNZIP_OK));
	}

	//sign
	atimer.Reset();
	ZBundle bundle;
	bundle.m_bEnableDocuments = bEnableDocuments;
	bundle.m_strMinVersion = strMinVersion;
	bundle.m_bRemoveExtensions = bRemoveExtensions;
	bundle.m_bRemoveWatchApp = bRemoveWatchApp;
	bundle.m_bRemoveUISupportedDevices = bRemoveUISupportedDevices;

	// -a: LC_LOAD_DYLIB; -w: LC_LOAD_WEAK_DYLIB; -a 优先于 -w
	bool bEffectiveWeakInject = bWeakInject && !bAdhoc;

	bool bRet;
	if (arrProvFiles.size() > 1) {
		list<ZSignAsset> zsaList;
		for (const string& provFile : arrProvFiles) {
			zsaList.push_back(ZSignAsset());
			if (!zsaList.back().Init(strCertFile, strPKeyFile, provFile, strEntitleFile, strPassword, bAdhoc, bSHA256Only, false)) {
				ZLog::ErrorV(ZL10n::GetFmt(ZL10nKeys::FAILED_INIT_PROVISION), provFile.c_str());
				zsaList.pop_back();
			}
		}
		bRet = bundle.SignFolder(&zsaList, strFolder, strBundleId, strBundleVersion, strDisplayName, arrDylibFiles, arrRemoveDylibNames, bForce, bEffectiveWeakInject, bEnableCache, bRemoveProvision, bRemoveDylibFiles);
	} else {
		bRet = bundle.SignFolder(&zsa, strFolder, strBundleId, strBundleVersion, strDisplayName, arrDylibFiles, arrRemoveDylibNames, bForce, bEffectiveWeakInject, bEnableCache, bRemoveProvision, bRemoveDylibFiles);
	}
	atimer.PrintResult(bRet, ZL10n::GetFmt(ZL10nKeys::SIGNED_FMT), bRet ? ZL10n::Get(ZL10nKeys::SIGNED_OK) : ZL10n::Get(ZL10nKeys::SIGNED_FAILED));

	// Post-sign certificate check
	if (bRet && bCheckSignature && !bundle.m_strAppFolder.empty()) {
		CheckSignedBinary(bundle.m_strAppFolder);
	}

	//archive
	if (bRet && !strOutputFile.empty()) {
		size_t pos = bundle.m_strAppFolder.rfind("Payload");
		if (string::npos != pos && pos > 0) {
			atimer.Reset();
			string strArchivePath = strOutputFileDisplay.empty() ? strOutputFile : strOutputFileDisplay;
			size_t sep = strArchivePath.find_last_of("/\\");
			string strArchiveName = (sep != string::npos && sep + 1 < strArchivePath.size()) ? strArchivePath.substr(sep + 1) : strArchivePath;
			ZLog::PrintV(ZL10n::GetFmt(ZL10nKeys::ARCHIVING), strArchiveName.c_str());
			string strBaseFolder = bundle.m_strAppFolder.substr(0, pos - 1);
			if (!Zip::Archive(strBaseFolder.c_str(), strOutputFile.c_str(), uZipLevel)) {
				ZLog::Error(ZL10n::GetFmt(ZL10nKeys::ARCHIVE_FAILED));
				bRet = false;
			} else {
				atimer.PrintResult(true, ZL10n::GetFmt(ZL10nKeys::ARCHIVE_OK), ZFile::GetFileSizeString(strOutputFile.c_str()).c_str());
				if (bRet && !strMetadataDir.empty()) {
					ZFile::CreateFolder(strMetadataDir.c_str());
					GetMetadata(bundle.m_strAppFolder, strMetadataDir, strOutputFile);
				}
			}
		} else {
			ZLog::Error(ZL10n::GetFmt(ZL10nKeys::CANT_FIND_PAYLOAD));
			bRet = false;
		}
	}

	//install
	if (bRet && bInstall) {
		bRet = ZUtil::SystemExecV("ideviceinstaller install  \"%s\"", strOutputFile.c_str());
	}

	//clean
	if (bTempFolder) {
		ZFile::RemoveFolder(strFolder.c_str());
	}

	if (bTempOutputFile) {
		ZFile::RemoveFile(strOutputFile.c_str());
	}

	gtimer.Print(ZL10n::Get(ZL10nKeys::DONE));
	return bRet ? 0 : -1;
}
