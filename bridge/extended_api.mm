#include "zsign_extended.h"

#include "archive_overlay.h"
#include "archive_cancel.h"
#include "check_cert_loader.h"
#include "fs_ipa_junk.h"
#include "macho_dylib_overlay.h"

#include "common.h"
#include "macho.h"
#include "timer.h"

#include <cstdlib>
#include <cstring>
#include <set>
#include <string>
#include <vector>

namespace {

static bool PathHasIpaExtension(const char* path)
{
	if (!path || !*path) {
		return false;
	}
	@autoreleasepool {
		NSString* nsPath = [NSString stringWithUTF8String:path];
		return [[[nsPath pathExtension] lowercaseString] isEqualToString:@"ipa"];
	}
}

static bool RenameExistingPayloadIfNeeded(const char* outDirUTF8)
{
	@autoreleasepool {
		NSFileManager* fm = [NSFileManager defaultManager];
		NSString* out = [NSString stringWithUTF8String:outDirUTF8];
		if (!out) {
			return false;
		}
		NSString* payload = [out stringByAppendingPathComponent:@"Payload"];
		BOOL isDir = NO;
		if (![fm fileExistsAtPath:payload isDirectory:&isDir] || !isDir) {
			return true;
		}
		for (int n = 1; n < 10000; n++) {
			NSString* name = [NSString stringWithFormat:@"Payload%d", n];
			NSString* candidate = [out stringByAppendingPathComponent:name];
			if (![fm fileExistsAtPath:candidate]) {
				NSError* err = nil;
				if (![fm moveItemAtPath:payload toPath:candidate error:&err]) {
					ZLog::ErrorV(">>> Extract: failed to rename Payload: %s\n", err ? [[err localizedDescription] UTF8String] : "unknown");
					return false;
				}
				ZLog::PrintV(">>> Unzip:\tRenamed existing Payload to %s ... \n", [name UTF8String]);
				return true;
			}
		}
		ZLog::Error(">>> Extract: too many Payload backups.\n");
		return false;
	}
}

static bool ValidatePayloadFolder(const char* payloadFolderUTF8, NSString** standardizedOut)
{
	if (!payloadFolderUTF8 || !*payloadFolderUTF8) {
		ZLog::Error(">>> Archive: folder path is required.\n");
		return false;
	}

	@autoreleasepool {
		NSString* standardized = [[NSString stringWithUTF8String:payloadFolderUTF8] stringByStandardizingPath];
		BOOL isDir = NO;
		NSFileManager* fm = [NSFileManager defaultManager];
		if (![fm fileExistsAtPath:standardized isDirectory:&isDir] || !isDir) {
			ZLog::ErrorV(">>> Archive: invalid folder: %s\n", [standardized UTF8String]);
			return false;
		}
		if ([[standardized lastPathComponent] compare:@"Payload" options:NSCaseInsensitiveSearch] != NSOrderedSame) {
			ZLog::Error(">>> Archive: path must be the Payload directory (…/Payload).\n");
			return false;
		}

		NSError* enumErr = nil;
		NSArray<NSString*>* contents = [fm contentsOfDirectoryAtPath:standardized error:&enumErr];
		if (!contents) {
			ZLog::ErrorV(">>> Archive: cannot list Payload: %s\n", enumErr ? [[enumErr localizedDescription] UTF8String] : "unknown");
			return false;
		}

		NSUInteger appCount = 0;
		for (NSString* name in contents) {
			if ([[name pathExtension] compare:@"app" options:NSCaseInsensitiveSearch] != NSOrderedSame) {
				continue;
			}
			NSString* full = [standardized stringByAppendingPathComponent:name];
			isDir = NO;
			if ([fm fileExistsAtPath:full isDirectory:&isDir] && isDir) {
				appCount++;
			}
		}
		if (appCount == 0) {
			ZLog::Error(">>> Archive: Payload must contain exactly one .app bundle (none found).\n");
			return false;
		}
		if (appCount > 1) {
			ZLog::Error(">>> Archive: Payload must contain exactly one .app bundle (multiple .app found).\n");
			return false;
		}

		if (standardizedOut) {
			*standardizedOut = standardized;
		}
		return true;
	}
}

} // namespace

extern "C" {

void ZsignZipBeginOperation(void)
{
	ZipBeginZipOperation();
}

int ZsignArchivePayloadDirectoryToIPA(
	const char* payloadFolderUTF8,
	const char* outputPathUTF8,
	int zipLevel,
	void (^completionHandler)(BOOL success, NSError* error))
{
	ZTimer atimer;
	ZTimer gtimer;

	int nZipLevel = (zipLevel >= 0 && zipLevel <= 9) ? zipLevel : 6;

	if (!outputPathUTF8 || !*outputPathUTF8) {
		ZLog::Error(">>> Archive: output path is required.\n");
		if (completionHandler) {
			completionHandler(NO, [NSError errorWithDomain:@"Zsign" code:-1 userInfo:@{NSLocalizedDescriptionKey: @"Output path is required"}]);
		}
		return -1;
	}

	NSString* standardized = nil;
	if (!ValidatePayloadFolder(payloadFolderUTF8, &standardized)) {
		if (completionHandler) {
			completionHandler(NO, [NSError errorWithDomain:@"Zsign" code:-1 userInfo:@{NSLocalizedDescriptionKey: @"Invalid Payload folder"}]);
		}
		return -1;
	}

	string strPayloadFolder = [standardized UTF8String];
	string strOutput = outputPathUTF8;

	atimer.Reset();
	ZLog::PrintV(">>> Archiving: \t%s ... \n", ZUtil::GetBaseName(strOutput.c_str()));
	(void)RemoveIPAPackagingJunkFromFolder(strPayloadFolder.c_str(), NULL, 0);
	ZipBeginZipOperation();
	bool bRet = Zip::ArchivePayloadFolderForIPA(strPayloadFolder, strOutput, nZipLevel);
	if (!bRet) {
		if (ZipLastFailureWasUserCancel()) {
			ZLog::Print(">>> Archive cancelled by user.\n");
		} else {
			ZLog::Error(">>> Archive failed!\n");
		}
	} else {
		atimer.PrintResult(true, ">>> Archive OK! (%s)", ZFile::GetFileSizeString(strOutput.c_str()).c_str());
	}

	NSError* archiveError = nil;
	if (!bRet) {
		if (ZipLastFailureWasUserCancel()) {
			archiveError = [NSError errorWithDomain:NSURLErrorDomain code:NSURLErrorCancelled userInfo:@{NSLocalizedDescriptionKey: @"Archive cancelled."}];
		} else {
			archiveError = [NSError errorWithDomain:@"Zsign" code:-1 userInfo:@{NSLocalizedDescriptionKey: @"Archive failed"}];
		}
	}

	if (completionHandler) {
		completionHandler(bRet, archiveError);
	}

	gtimer.Print(">>> Done.");
	return bRet ? 0 : -1;
}

int ZsignExtractIPAIntoDirectory(
	const char* ipaPathUTF8,
	const char* outputFolderUTF8,
	void (^completionHandler)(BOOL success, NSError* error))
{
	ZTimer atimer;
	ZTimer gtimer;

	if (!ipaPathUTF8 || !*ipaPathUTF8) {
		ZLog::Error(">>> Extract: input path is required.\n");
		if (completionHandler) {
			completionHandler(NO, [NSError errorWithDomain:@"Zsign" code:-1 userInfo:@{NSLocalizedDescriptionKey: @"Input path is required"}]);
		}
		return -1;
	}
	if (!outputFolderUTF8 || !*outputFolderUTF8) {
		ZLog::Error(">>> Extract: output folder is required.\n");
		if (completionHandler) {
			completionHandler(NO, [NSError errorWithDomain:@"Zsign" code:-1 userInfo:@{NSLocalizedDescriptionKey: @"Output folder is required"}]);
		}
		return -1;
	}

	string strPath = ipaPathUTF8;
	string strOut = outputFolderUTF8;

	if (!ZFile::IsFileExists(strPath.c_str())) {
		ZLog::ErrorV(">>> Invalid path! %s\n", strPath.c_str());
		if (completionHandler) {
			completionHandler(NO, [NSError errorWithDomain:@"Zsign" code:-1 userInfo:@{NSLocalizedDescriptionKey: @"Invalid input path"}]);
		}
		return -1;
	}
	if (!ZFile::IsZipFile(strPath.c_str())) {
		ZLog::Error(">>> Extract: not a zip file.\n");
		if (completionHandler) {
			completionHandler(NO, [NSError errorWithDomain:@"Zsign" code:-1 userInfo:@{NSLocalizedDescriptionKey: @"Not a zip file"}]);
		}
		return -1;
	}
	if (!PathHasIpaExtension(ipaPathUTF8)) {
		ZLog::Error(">>> Extract: input must be an .ipa file.\n");
		if (completionHandler) {
			completionHandler(NO, [NSError errorWithDomain:@"Zsign" code:-1 userInfo:@{NSLocalizedDescriptionKey: @"Input must be a .ipa file"}]);
		}
		return -1;
	}
	if (!Zip::HasIpaLayout(strPath.c_str())) {
		ZLog::Error(">>> Extract: not a valid IPA (missing Payload/xxx.app).\n");
		if (completionHandler) {
			completionHandler(NO, [NSError errorWithDomain:@"Zsign" code:-1 userInfo:@{NSLocalizedDescriptionKey: @"Not a valid IPA (expected Payload/xxx.app in archive)"}]);
		}
		return -1;
	}
	if (ZFile::IsFileExists(strOut.c_str()) && !ZFile::IsFolder(strOut.c_str())) {
		ZLog::Error(">>> Extract: output path must be a directory.\n");
		if (completionHandler) {
			completionHandler(NO, [NSError errorWithDomain:@"Zsign" code:-1 userInfo:@{NSLocalizedDescriptionKey: @"Output path must be a directory"}]);
		}
		return -1;
	}
	if (!ZFile::CreateFolder(strOut.c_str())) {
		ZLog::Error(">>> Extract: output directory could not be created.\n");
		if (completionHandler) {
			completionHandler(NO, [NSError errorWithDomain:@"Zsign" code:-1 userInfo:@{NSLocalizedDescriptionKey: @"Output directory could not be created"}]);
		}
		return -1;
	}
	if (!RenameExistingPayloadIfNeeded(strOut.c_str())) {
		if (completionHandler) {
			completionHandler(NO, [NSError errorWithDomain:@"Zsign" code:-1 userInfo:@{NSLocalizedDescriptionKey: @"Failed to rename existing Payload"}]);
		}
		return -1;
	}

	atimer.Reset();
	ZLog::PrintV(">>> Unzip:\t%s (%s) ... \n", ZUtil::GetBaseName(strPath.c_str()), ZFile::GetFileSizeString(strPath.c_str()).c_str());
	ZipBeginZipOperation();
	bool bRet = Zip::ExtractWithProgressIntoExisting(strPath.c_str(), strOut.c_str());
	if (!bRet) {
		ZLog::ErrorV(">>> Unzip failed!\n");
	} else {
		atimer.PrintResult(true, ">>> Unzip OK!");
	}

	NSError* extractError = nil;
	if (!bRet) {
		extractError = [NSError errorWithDomain:@"Zsign" code:-1 userInfo:@{NSLocalizedDescriptionKey: @"Unzip failed"}];
	}

	if (completionHandler) {
		completionHandler(bRet, extractError);
	}

	gtimer.Print(">>> Done.");
	return bRet ? 0 : -1;
}

bool ZsignExtendedMachoCheckSigned(const char* path)
{
	ZTimer gtimer;
	if (!path || !*path) {
		return false;
	}

	ZMachO machO;
	if (!machO.Init(path)) {
		gtimer.Print(">>> Failed to initialize ZMachO.");
		return false;
	}

	bool success = machO.CheckSignature();
	machO.Free();

	if (success) {
		gtimer.Print(">>> MachO is signed!");
		return true;
	}
	gtimer.Print(">>> MachO is not signed.");
	return false;
}

bool ZsignExtendedMachoInjectDylib(const char* path, const char* dylibPath, bool weakInject)
{
	ZTimer gtimer;
	if (!path || !*path || !dylibPath || !*dylibPath) {
		return false;
	}

	ZMachO machO;
	if (!machO.Init(path)) {
		gtimer.Print(">>> Failed to initialize ZMachO.");
		return false;
	}

	bool success = machO.InjectDylib(weakInject, dylibPath);
	machO.Free();

	if (success) {
		gtimer.Print(">>> Dylib injected successfully!");
		return true;
	}
	gtimer.Print(">>> Failed to inject dylib.");
	return false;
}

bool ZsignExtendedMachoRemoveDylibs(const char* path, const char* const* dylibs, size_t dylibCount)
{
	ZTimer gtimer;
	if (!path || !*path || !dylibs || dylibCount == 0) {
		return false;
	}

	std::set<std::string> dylibsToRemove;
	for (size_t i = 0; i < dylibCount; i++) {
		if (dylibs[i]) {
			dylibsToRemove.insert(dylibs[i]);
		}
	}

	ZMachO machO;
	if (!machO.Init(path)) {
		gtimer.Print(">>> Failed to initialize ZMachO.");
		return false;
	}

	machO.RemoveDylibs(dylibsToRemove);
	machO.Free();

	gtimer.Print(">>> Dylibs uninstalled successfully!");
	return true;
}

bool ZsignExtendedMachoListDylibs(const char* path, void (^append)(const char* dylibPath))
{
	ZTimer gtimer;
	if (!path || !*path || !append) {
		return false;
	}

	if (!ZFile::IsFileExists(path)) {
		gtimer.Print(">>> Failed to initialize ZMachO.");
		return false;
	}

	std::vector<std::string> dylibPaths;
	if (!MachoBridgeListDylibs(path, dylibPaths)) {
		gtimer.Print(">>> Failed to list dylibs.");
		return false;
	}
	if (!dylibPaths.empty()) {
		gtimer.Print(">>> List of dylibs in the Mach-O file:");
		for (const std::string& dylibPath : dylibPaths) {
			append(dylibPath.c_str());
		}
	} else {
		gtimer.Print(">>> No dylibs found in the Mach-O file.");
	}

	return true;
}

bool ZsignExtendedMachoChangeDylibPath(const char* path, const char* oldPath, const char* newPath)
{
	ZTimer gtimer;
	if (!path || !*path || !oldPath || !*oldPath || !newPath || !*newPath) {
		return false;
	}

	if (!ZFile::IsFileExists(path)) {
		gtimer.Print(">>> Failed to initialize ZMachO.");
		return false;
	}

	bool success = MachoBridgeChangeDylibPath(path, oldPath, newPath);
	if (success) {
		gtimer.Print(">>> Dylib path changed successfully!");
		return true;
	}
	gtimer.Print(">>> Failed to change dylib path.");
	return false;
}

bool ZsignLoadCheckCertAssets(
	const char* pkeyPath,
	const char* provPath,
	const char* password,
	ZsignCertLoaded* out,
	char* errorOut,
	size_t errorOutLen)
{
	if (!out) {
		return false;
	}
	out->cert = NULL;
	out->issuer = NULL;

	CheckCertLoaded loaded = {NULL, NULL};
	string loadError;
	bool ok = LoadCheckCertAssets(
		pkeyPath ? string(pkeyPath) : string(),
		provPath ? string(provPath) : string(),
		password ? string(password) : string(),
		loaded,
		loadError);
	if (!ok) {
		if (errorOut && errorOutLen > 0) {
			strncpy(errorOut, loadError.c_str(), errorOutLen - 1);
			errorOut[errorOutLen - 1] = '\0';
		}
		return false;
	}

	out->cert = loaded.cert;
	out->issuer = loaded.issuer;
	return true;
}

void ZsignFreeCheckCertLoaded(ZsignCertLoaded* loaded)
{
	if (!loaded) {
		return;
	}
	CheckCertLoaded native = {loaded->cert, loaded->issuer};
	FreeCheckCertLoaded(native);
	loaded->cert = NULL;
	loaded->issuer = NULL;
}

}
