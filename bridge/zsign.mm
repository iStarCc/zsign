//
//  zsign.mm
//  Swift bridge — 复刻 master/src/zsign.cpp main()
//

#include "zsign.hpp"
#include "log_overlay.h"
#include "i18n/zlog_i18n.h"
#include "common.h"
#include "macho.h"
#include "bundle.h"
#include "openssl.h"
#include "timer.h"
#include "archive_overlay.h"
#include "fs_ipa_junk.h"
#include "verify_signed_bundle.h"
#include "metadata.h"
#include "certcheck.h"

#include <cstdlib>
#include <list>
#include <set>
#include <string>
#include <vector>

#if !defined(_WIN32) && TARGET_OS_OSX
#include <sys/wait.h>
#endif

#ifndef ZSIGN_VERSION
#define ZSIGN_VERSION 614caa8
#endif
#define ZSIGN_STR_(x) #x
#define ZSIGN_STR(x) ZSIGN_STR_(x)
#define ZSIGN_VERSION_STR ZSIGN_STR(ZSIGN_VERSION)

namespace {

struct ZsignLangScope {
	bool on_;
	string prev_;
	bool hadPrev_;
	explicit ZsignLangScope(bool zh) : on_(zh), hadPrev_(false) {
		if (!on_) {
			return;
		}
		const char* p = getenv("ZSIGN_LANG");
		if (p) {
			prev_.assign(p);
			hadPrev_ = true;
		}
		setenv("ZSIGN_LANG", "zh", 1);
	}
	~ZsignLangScope() {
		if (!on_) {
			return;
		}
		if (!hadPrev_) {
			unsetenv("ZSIGN_LANG");
		} else {
			setenv("ZSIGN_LANG", prev_.c_str(), 1);
		}
	}
};

static const char* kHelpText =
	"zsign (v" ZSIGN_VERSION_STR ") is a codesign alternative for iOS12+ on macOS, Linux and Windows.\n"
	"Visit https://github.com/zhlynn/zsign for more information.\n\n"
	"Usage: zsign [-options] [-k privkey.pem] [-m dev.prov] [-o output.ipa] file|folder\n"
	"options:\n"
	"-k, --pkey\t\tPath to private key or p12 file. (PEM or DER format)\n"
	"-m, --prov\t\tPath to mobile provisioning profile.\n"
	"-c, --cert\t\tPath to certificate file. (PEM or DER format)\n"
	"-a, --adhoc\t\tPerform ad-hoc signature only.\n"
	"-d, --debug\t\tGenerate debug output files. (.zsign_debug folder)\n"
	"-f, --force\t\tForce sign without cache when signing folder.\n"
	"-o, --output\t\tPath to output ipa file.\n"
	"-p, --password\t\tPassword for private key or p12 file.\n"
	"-b, --bundle_id\t\tNew bundle id to change.\n"
	"-n, --bundle_name\tNew bundle name to change.\n"
	"-r, --bundle_version\tNew bundle version to change.\n"
	"-e, --entitlements\tNew entitlements to change.\n"
	"-I, --icon\t\tPath to new app icon to replace the primary icon. (PNG format)\n"
	"-z, --zip_level\t\tCompressed level when output the ipa file. (0-9)\n"
	"-l, --dylib\t\tPath to inject dylib file. Use -l multiple time to inject multiple dylib files at once.\n"
	"-D, --rm_dylib\t\tName of dylib to remove. Use -D multiple times to remove multiple dylibs at once.\n"
	"-w, --weak\t\tInject dylib as LC_LOAD_WEAK_DYLIB.\n"
	"-i, --install\t\tInstall ipa file using ideviceinstaller command for test.\n"
	"-t, --temp_folder\tPath to temporary folder for intermediate files.\n"
	"-2, --sha256_only\t(Deprecated, now the default.) Kept for backward compatibility.\n"
	"-L, --legacy_sha1\tEmit a dual SHA1+SHA256 CodeDirectory for iOS <= 10 compatibility.\n"
	"-C, --check\t\tCheck certificate validity and OCSP revocation status.\n"
	"-q, --quiet\t\tQuiet operation.\n"
	"-x, --metadata\t\tExtract metadata and icon to the specified directory.\n"
	"-R, --rm_provision\tRemove mobileprovision file after signing.\n"
	"-S, --enable_docs\tEnable UISupportsDocumentBrowser and UIFileSharingEnabled.\n"
	"-M, --min_version\tSet MinimumOSVersion in Info.plist.\n"
	"-E, --rm_extensions\tRemove all app extensions (PlugIns/Extensions).\n"
	"-W, --rm_watch\t\tRemove watch app from the bundle.\n"
	"-U, --rm_uisd\t\tRemove UISupportedDevices from Info.plist.\n"
	"-P, --inject_extensions\tAlso inject -l dylibs into app extensions (PlugIns/Extensions).\n"
	"-v, --version\t\tShows version.\n"
	"-h, --help\t\tShows help (this message).\n";

static string OptStr(const char* _Nullable value)
{
	return value ? string(value) : string();
}

static vector<string> OptStrArray(const char* _Nullable const* _Nullable values, size_t count)
{
	vector<string> result;
	if (!values) {
		return result;
	}
	for (size_t i = 0; i < count; i++) {
		if (values[i]) {
			result.emplace_back(values[i]);
		}
	}
	return result;
}

static bool InstallSignedIpa(const string& strOutputFile)
{
	if (strOutputFile.empty()) {
		return false;
	}

#if TARGET_OS_OSX
	pid_t pid = fork();
	if (pid < 0) {
		ZLog::ErrorV(">>> Install failed to fork ideviceinstaller! %s\n", strOutputFile.c_str());
		return false;
	}
	if (0 == pid) {
		execlp("ideviceinstaller", "ideviceinstaller", "install", strOutputFile.c_str(), (char*)NULL);
		_exit(127);
	}

	int nStatus = 0;
	if (waitpid(pid, &nStatus, 0) < 0) {
		ZLog::ErrorV(">>> Install failed to wait ideviceinstaller! %s\n", strOutputFile.c_str());
		return false;
	}
	if (!WIFEXITED(nStatus) || 0 != WEXITSTATUS(nStatus)) {
		ZLog::ErrorV(">>> ideviceinstaller install failed! %s\n", strOutputFile.c_str());
		return false;
	}
	return true;
#else
	ZLog::Error(">>> Install via ideviceinstaller is only supported on macOS.\n");
	return false;
#endif
}

static void FinishRun(bool bRet, ZsignCompletionHandler completionHandler)
{
	NSError* error = nil;
	if (!bRet) {
		if (ZipLastFailureWasUserCancel()) {
			error = [NSError errorWithDomain:NSURLErrorDomain
										code:NSURLErrorCancelled
									userInfo:@{NSLocalizedDescriptionKey: @"Operation cancelled."}];
		} else {
			error = [NSError errorWithDomain:@"Zsign"
										code:-1
									userInfo:@{NSLocalizedDescriptionKey: @"Operation failed"}];
		}
	}
	if (completionHandler) {
		completionHandler(bRet ? YES : NO, error);
	}
}

static int RunEngine(const string& strPath, const ZsignRunOptions* opts)
{
	ZTimer atimer;
	ZTimer gtimer;

	bool bForce = opts->force;
	bool bInstall = opts->install;
	bool bWeakInject = opts->weakInject;
	bool bAdhoc = opts->adhoc;
	bool bSHA256Only = opts->legacySHA1 ? false : opts->sha256Only;
	bool bCheckSignature = opts->check;
	bool bRemoveProvision = opts->removeProvision;
	bool bEnableDocuments = opts->enableDocs;
	string strMinVersion = OptStr(opts->minVersion);
	bool bRemoveExtensions = opts->removeExtensions;
	bool bRemoveWatchApp = opts->removeWatch;
	bool bRemoveUISupportedDevices = opts->removeUISD;
	bool bInjectExtensions = opts->injectExtensions;
	uint32_t uZipLevel = (uint32_t)opts->zipLevel;

	string strCertFile = OptStr(opts->cert);
	string strPKeyFile = OptStr(opts->pkey);
	string strPassword = OptStr(opts->password);
	string strBundleId = OptStr(opts->bundleId);
	string strBundleVersion = OptStr(opts->bundleVersion);
	string strOutputFile = OptStr(opts->output);
	string strDisplayName = OptStr(opts->bundleName);
	string strEntitleFile = OptStr(opts->entitlements);
	string strIconFile = OptStr(opts->icon);
	vector<string> arrDylibFiles = OptStrArray(opts->dylibs, opts->dylibCount);
	vector<string> arrRemoveDylibNames = OptStrArray(opts->removeDylibs, opts->removeDylibCount);
	string strMetadataDir = OptStr(opts->metadata);
	string strTempFolder = OptStr(opts->tempFolder);
	vector<string> arrProvFiles = OptStrArray(opts->provisionPaths, opts->provisionPathCount);

	if (strTempFolder.empty()) {
		strTempFolder = ZFile::GetTempFolder();
	}

	string strProvFile;
	if (!arrProvFiles.empty()) {
		strProvFile = arrProvFiles.front();
	}

	if (opts->debug) {
		ZLog::SetLogLever(ZLog::E_DEBUG);
	} else if (opts->quiet) {
		ZLog::SetLogLever(ZLog::E_NONE);
	}

	if (!ZFile::IsFolder(strTempFolder.c_str())) {
		ZLog::ErrorV(">>> Invalid temp folder! %s\n", strTempFolder.c_str());
		return -1;
	}

	if (!ZFile::IsFileExists(strPath.c_str())) {
		ZLog::ErrorV(">>> Invalid path! %s\n", strPath.c_str());
		return -1;
	}

	if (uZipLevel > 9) {
		ZLog::ErrorV(">>> Invalid zip level! Please input 0 - 9.\n");
		return -1;
	}

	for (const string& strDylibFile : arrDylibFiles) {
		if (!ZFile::IsFileExists(strDylibFile.c_str())) {
			ZLog::ErrorV(">>> Dylib file not found! %s\n", strDylibFile.c_str());
			return -1;
		}
		ZMachO dylibMachO;
		if (!dylibMachO.Init(strDylibFile.c_str())) {
			ZLog::ErrorV(">>> Invalid dylib file! Not a valid Mach-O format. %s\n", strDylibFile.c_str());
			return -1;
		}
	}

	if (!strIconFile.empty()) {
		string strIconData;
		if (!ZFile::ReadFile(strIconFile.c_str(), strIconData) || strIconData.size() < 8 ||
			0 != memcmp(strIconData.data(), "\x89PNG\r\n\x1a\n", 8)) {
			ZLog::ErrorV(">>> Invalid icon file! Only PNG format is supported. %s\n", strIconFile.c_str());
			return -1;
		}
	}

	if (ZLog::IsDebug()) {
		ZFile::CreateFolder("./.zsign_debug");
		ZLog::DebugV(">>> Argument:\t%s\n", strPath.c_str());
	}

	if (bCheckSignature && strPKeyFile.empty() && strProvFile.empty()) {
		return CheckCertificate(strPath, strPassword);
	}

	bool bZipFile = ZFile::IsZipFile(strPath.c_str());
	if (!bZipFile && !ZFile::IsFolder(strPath.c_str())) {
		ZMachO* macho = new ZMachO();
		if (!macho->Init(strPath.c_str())) {
			ZLog::ErrorV(">>> Invalid mach-o file! %s\n", strPath.c_str());
			delete macho;
			return -1;
		}

		if (!bAdhoc && arrDylibFiles.empty() && arrRemoveDylibNames.empty() &&
			(strPKeyFile.empty() || strProvFile.empty())) {
			macho->PrintInfo();
			delete macho;
			return 0;
		}

		ZSignAsset zsa;
		if (!zsa.Init(strCertFile, strPKeyFile, strProvFile, strEntitleFile, strPassword, bAdhoc, bSHA256Only, true)) {
			delete macho;
			return -1;
		}

		if (!arrDylibFiles.empty()) {
			for (const string& dyLibFile : arrDylibFiles) {
				if (!macho->InjectDylib(bWeakInject, dyLibFile.c_str())) {
					delete macho;
					return -1;
				}
			}
		}

		if (!arrRemoveDylibNames.empty()) {
			set<string> setDylibs;
			for (const string& name : arrRemoveDylibNames) {
				if (name.find('/') != string::npos) {
					setDylibs.insert(name);
				} else {
					setDylibs.insert("@executable_path/" + name);
				}
			}
			macho->RemoveDylibs(setDylibs);
		}

		atimer.Reset();
		ZLog::PrintV(">>> Signing:\t%s %s\n", strPath.c_str(), (bAdhoc ? " (Ad-hoc)" : ""));
		string strInfoSHA1;
		string strInfoSHA256;
		string strCodeResourcesData;
		bool bRet = macho->Sign(&zsa, bForce, strBundleId, strInfoSHA1, strInfoSHA256, strCodeResourcesData);
		atimer.PrintResult(bRet, ">>> Signed %s!", bRet ? "OK" : "Failed");
		delete macho;
		gtimer.Print(">>> Done.");
		return bRet ? 0 : -1;
	}

	bool bTempOutputFile = false;
	if (strOutputFile.empty()) {
		if (bInstall) {
			bTempOutputFile = true;
			strOutputFile = ZFile::GetRealPathV("%s/zsign_temp_%llu.ipa", strTempFolder.c_str(), ZUtil::GetMicroSecond());
		} else if (bZipFile) {
			ZLog::ErrorV(">>> Use -o option to specify the output file.\n");
			return -1;
		}
	}

	ZSignAsset zsa;
	if (!zsa.Init(strCertFile, strPKeyFile, strProvFile, strEntitleFile, strPassword, bAdhoc, bSHA256Only, false)) {
		return -1;
	}

	bool bTempFolder = false;
	bool bEnableCache = true;
	string strFolder = strPath;
	if (bZipFile) {
		bForce = true;
		bTempFolder = true;
		bEnableCache = false;
		strFolder = ZFile::GetRealPathV("%s/zsign_folder_%llu", strTempFolder.c_str(), atimer.Reset());
		ZLog::PrintV(">>> Unzip:\t%s (%s) ... \n", ZUtil::GetBaseName(strPath.c_str()), ZFile::GetFileSizeString(strPath.c_str()).c_str());
		ZipBeginZipOperation();
		if (!Zip::ExtractWithProgress(strPath.c_str(), strFolder.c_str())) {
			if (ZipLastFailureWasUserCancel()) {
				ZLog::Print(">>> Unzip cancelled by user.\n");
			} else {
				ZLog::ErrorV(">>> Unzip failed!\n");
			}
			return -1;
		}
		atimer.PrintResult(true, ">>> Unzip OK!");
		ZLog::Print("\n");
	}

	(void)RemoveIPAPackagingJunkFromFolder(
		strFolder.c_str(),
		opts->removePaths,
		opts->removePathCount);

	atimer.Reset();
	ZBundle bundle;
	bundle.m_bEnableDocuments = bEnableDocuments;
	bundle.m_strMinVersion = strMinVersion;
	bundle.m_strIconFile = strIconFile;
	bundle.m_bRemoveExtensions = bRemoveExtensions;
	bundle.m_bRemoveWatchApp = bRemoveWatchApp;
	bundle.m_bRemoveUISupportedDevices = bRemoveUISupportedDevices;
	bundle.m_bInjectExtensions = bInjectExtensions;

	bool bRet;
	if (arrProvFiles.size() > 1) {
		list<ZSignAsset> zsaList;
		for (const string& provFile : arrProvFiles) {
			zsaList.push_back(ZSignAsset());
			if (!zsaList.back().Init(strCertFile, strPKeyFile, provFile, strEntitleFile, strPassword, bAdhoc, bSHA256Only, false)) {
				ZLog::ErrorV(">>> Failed to init provision: %s\n", provFile.c_str());
				zsaList.pop_back();
			}
		}
		bRet = bundle.SignFolder(&zsaList, strFolder, strBundleId, strBundleVersion, strDisplayName, arrDylibFiles,
								 arrRemoveDylibNames, bForce, bWeakInject, bEnableCache, bRemoveProvision);
	} else {
		bRet = bundle.SignFolder(&zsa, strFolder, strBundleId, strBundleVersion, strDisplayName, arrDylibFiles,
								 arrRemoveDylibNames, bForce, bWeakInject, bEnableCache, bRemoveProvision);
	}
	atimer.PrintResult(bRet, ">>> Signed %s!", bRet ? "OK" : "Failed");

	if (bRet && bCheckSignature && !bundle.m_strAppFolder.empty()) {
		if (!VerifySignedBundle(bundle.m_strAppFolder.c_str(), !bAdhoc)) {
			ZLog::Error(">>> Post-sign verification FAILED!\n");
			bRet = false;
		}
		if (bRet) {
			CheckSignedBinary(bundle.m_strAppFolder);
		}
	}

	if (bRet && !strOutputFile.empty()) {
		ZLog::Print("\n");
		size_t pos = bundle.m_strAppFolder.rfind("Payload");
		if (string::npos != pos && pos > 0) {
			atimer.Reset();
			ZLog::PrintV(">>> Archiving: \t%s ... \n", ZUtil::GetBaseName(strOutputFile.c_str()));
			string strBaseFolder = bundle.m_strAppFolder.substr(0, pos - 1);
			ZipBeginZipOperation();
			if (!Zip::Archive(strBaseFolder.c_str(), strOutputFile.c_str(), uZipLevel)) {
				if (ZipLastFailureWasUserCancel()) {
					ZLog::Print(">>> Archive cancelled by user.\n");
				} else {
					ZLog::Error(">>> Archive failed!\n");
				}
				bRet = false;
			} else {
				atimer.PrintResult(true, ">>> Archive OK! (%s)", ZFile::GetFileSizeString(strOutputFile.c_str()).c_str());
				if (bRet && !strMetadataDir.empty()) {
					ZFile::CreateFolder(strMetadataDir.c_str());
					GetMetadata(bundle.m_strAppFolder, strMetadataDir, strOutputFile);
				}
			}
		} else {
			ZLog::Error(">>> Can't find payload directory!\n");
			bRet = false;
		}
	}

	if (bRet && bInstall) {
		bRet = InstallSignedIpa(strOutputFile);
	}

	if (bTempFolder) {
		ZFile::RemoveFolder(strFolder.c_str());
	}

	if (bTempOutputFile) {
		ZFile::RemoveFile(strOutputFile.c_str());
	}

	gtimer.Print(">>> Done.");
	return bRet ? 0 : -1;
}

static void (^ _Nullable gLogHandler)(NSString* _Nullable line) = nil;

static void ZsignLogSinkTrampoline(const char* utf8Line, int color, void* userdata)
{
	(void)color;
	(void)userdata;
	void (^ _Nullable handler)(NSString* _Nullable) = gLogHandler;
	if (!handler || !utf8Line) {
		return;
	}
	NSString* line = [NSString stringWithUTF8String:utf8Line];
	if (line) {
		handler(line);
	}
}

} // namespace

extern "C" {

const char* ZsignVersionString(void)
{
	return "version: " ZSIGN_VERSION_STR;
}

const char* ZsignHelpText(void)
{
	static string sHelp;
	ZLogI18n::Apply(kHelpText, sHelp);
	return sHelp.c_str();
}

void ZsignSetLogHandler(void (^ _Nullable handler)(NSString* _Nullable line))
{
	gLogHandler = [handler copy];
	if (handler) {
		ZLog_SetExternalSink(ZsignLogSinkTrampoline, NULL);
	} else {
		ZLog_ClearExternalSink();
	}
}

void ZsignRequestZipCancel(void)
{
	ZipRequestCancel();
}

bool ZsignZipLastFailureWasUserCancel(void)
{
	return ZipLastFailureWasUserCancel() ? true : false;
}

bool ZsignRemoveIPAPackagingJunkFromFolder(
	const char* szRoot,
	const char* const* removePaths,
	size_t removePathCount)
{
	return RemoveIPAPackagingJunkFromFolder(szRoot, removePaths, removePathCount) ? true : false;
}

bool ZsignVerifySignedBundle(const char* appFolder, bool bCheckCMS)
{
	return VerifySignedBundle(appFolder, bCheckCMS) ? true : false;
}

int zsignRun(
	const char* inputPath,
	const ZsignRunOptions* options,
	void (^ _Nullable completionHandler)(BOOL success, NSError* _Nullable error))
{
	@autoreleasepool {
		if (!inputPath || !options) {
			FinishRun(false, completionHandler);
			return -1;
		}

		string strPath = ZFile::GetFullPath(inputPath);
		ZsignLangScope langScope(options->zh);
		int result = RunEngine(strPath, options);
		FinishRun(result == 0, completionHandler);
		return result;
	}
}

}
