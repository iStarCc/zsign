#include "zsign.hpp"

#include "archive_overlay.h"
#include "archive_cancel.h"
#include "fs_ipa_junk.h"

#include "common.h"
#include "timer.h"

#include <cstdlib>
#include <string>

namespace {

struct ZsignLangScope {
	bool on_;
	std::string prev_;
	bool hadPrev_;
	explicit ZsignLangScope(bool zh) : on_(zh), hadPrev_(false)
	{
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
	~ZsignLangScope()
	{
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

static const char* NsUtf8(NSString* _Nullable s)
{
	return s ? [s UTF8String] : "";
}

static ZsignRunOptions MakeIpaxRunOptions(
	NSString* prov,
	NSString* key,
	NSString* pass,
	NSString* entitlement,
	NSString* bundleid,
	NSString* displayname,
	NSString* bundleversion,
	bool adhoc,
	bool removeProvision,
	bool removeUISupportedDevices,
	bool removeWatchApp,
	bool enableDocuments,
	NSString* _Nullable minOSVersion,
	bool removeExtensions,
	bool zh,
	const char* outputPath,
	const char* tempFolder,
	int zipLevel)
{
	ZsignRunOptions opts = {};
	opts.pkey = NsUtf8(key);
	opts.password = NsUtf8(pass);
	opts.entitlements = NsUtf8(entitlement);
	opts.bundleId = NsUtf8(bundleid);
	opts.bundleName = NsUtf8(displayname);
	opts.bundleVersion = NsUtf8(bundleversion);
	opts.output = outputPath ? outputPath : "";
	opts.tempFolder = tempFolder ? tempFolder : "";
	opts.minVersion = minOSVersion ? [minOSVersion UTF8String] : "";
	opts.provisionPaths = NULL;
	opts.provisionPathCount = 0;
	opts.zipLevel = zipLevel;
	opts.adhoc = adhoc;
	opts.force = true;
	opts.sha256Only = true;
	opts.check = true;
	opts.removeProvision = removeProvision;
	opts.enableDocs = enableDocuments;
	opts.removeExtensions = removeExtensions;
	opts.removeWatch = removeWatchApp;
	opts.removeUISD = removeUISupportedDevices;
	opts.zh = zh;
	return opts;
}

static void AttachProvisionPath(
	ZsignRunOptions& opts,
	const char* provisionPath,
	const char* provisionPathEntries[1])
{
	if (provisionPath && *provisionPath) {
		provisionPathEntries[0] = provisionPath;
		opts.provisionPaths = provisionPathEntries;
		opts.provisionPathCount = 1;
	} else {
		provisionPathEntries[0] = NULL;
		opts.provisionPaths = NULL;
		opts.provisionPathCount = 0;
	}
}

static bool ZsignPathHasIpaExtension(NSString* ipaPath)
{
	if (!ipaPath || [ipaPath length] == 0) {
		return false;
	}
	return [[[ipaPath pathExtension] lowercaseString] isEqualToString:@"ipa"];
}

static bool ZsignRenameExistingPayloadIfNeeded(const char* outDirUTF8)
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

} // namespace

extern "C" {

int zsign(
	NSString* app,
	NSString* prov,
	NSString* key,
	NSString* pass,
	NSString* entitlement,
	NSString* bundleid,
	NSString* displayname,
	NSString* bundleversion,
	bool adhoc,
	bool dontGenerateEmbeddedMobileProvision,
	bool removeUISupportedDevices,
	bool removeWatchApp,
	bool enableDocuments,
	NSString* _Nullable minOSVersion,
	bool removeExtensions,
	bool zh,
	void (^ _Nullable completionHandler)(BOOL success, NSError* _Nullable error))
{
	@autoreleasepool {
		if (!app || [app length] == 0) {
			if (completionHandler) {
				completionHandler(NO, [NSError errorWithDomain:@"Zsign" code:-1 userInfo:@{NSLocalizedDescriptionKey: @"App path is required"}]);
			}
			return -1;
		}

		const char* provPath = NsUtf8(prov);
		const char* provPathEntries[1] = { NULL };
		ZsignRunOptions opts = MakeIpaxRunOptions(
			prov, key, pass, entitlement, bundleid, displayname, bundleversion,
			adhoc, dontGenerateEmbeddedMobileProvision, removeUISupportedDevices,
			removeWatchApp, enableDocuments, minOSVersion, removeExtensions, zh,
			"", "", 0);
		AttachProvisionPath(opts, provPath, provPathEntries);

		return zsignRun(NsUtf8(app), &opts, completionHandler);
	}
}

int zsignIPA(
	NSString* inputPath,
	NSString* outputPath,
	NSString* prov,
	NSString* key,
	NSString* pass,
	NSString* entitlement,
	NSString* bundleid,
	NSString* displayname,
	NSString* bundleversion,
	bool adhoc,
	bool dontGenerateEmbeddedMobileProvision,
	bool removeUISupportedDevices,
	bool removeWatchApp,
	bool enableDocuments,
	NSString* _Nullable minOSVersion,
	bool removeExtensions,
	int zipLevel,
	NSString* _Nullable tempFolder,
	bool zh,
	void (^ _Nullable completionHandler)(BOOL success, NSError* _Nullable error))
{
	@autoreleasepool {
		if (!inputPath || [inputPath length] == 0) {
			if (completionHandler) {
				completionHandler(NO, [NSError errorWithDomain:@"Zsign" code:-1 userInfo:@{NSLocalizedDescriptionKey: @"Input path is required"}]);
			}
			return -1;
		}
		if (!outputPath || [outputPath length] == 0) {
			if (completionHandler) {
				completionHandler(NO, [NSError errorWithDomain:@"Zsign" code:-1 userInfo:@{NSLocalizedDescriptionKey: @"Output path is required"}]);
			}
			return -1;
		}

		const char* provPath = NsUtf8(prov);
		const char* provPathEntries[1] = { NULL };
		int zl = (zipLevel >= 0 && zipLevel <= 9) ? zipLevel : 6;
		ZsignRunOptions opts = MakeIpaxRunOptions(
			prov, key, pass, entitlement, bundleid, displayname, bundleversion,
			adhoc, dontGenerateEmbeddedMobileProvision, removeUISupportedDevices,
			removeWatchApp, enableDocuments, minOSVersion, removeExtensions, zh,
			NsUtf8(outputPath), NsUtf8(tempFolder), zl);
		AttachProvisionPath(opts, provPath, provPathEntries);

		return zsignRun(NsUtf8(inputPath), &opts, completionHandler);
	}
}

int zsignArchiveFolderToIPA(
	NSString* folderPath,
	NSString* outputPath,
	int zipLevel,
	bool zh,
	void (^ _Nullable completionHandler)(BOOL success, NSError* _Nullable error))
{
	ZsignLangScope langScope(zh);

	ZTimer atimer;
	ZTimer gtimer;

	int nZipLevel = (zipLevel >= 0 && zipLevel <= 9) ? zipLevel : 6;

	if (!folderPath || [folderPath length] == 0) {
		ZLog::Error(">>> Archive: folder path is required.\n");
		if (completionHandler) {
			completionHandler(NO, [NSError errorWithDomain:@"Zsign" code:-1 userInfo:@{NSLocalizedDescriptionKey: @"Folder path is required"}]);
		}
		return -1;
	}
	if (!outputPath || [outputPath length] == 0) {
		ZLog::Error(">>> Archive: output path is required.\n");
		if (completionHandler) {
			completionHandler(NO, [NSError errorWithDomain:@"Zsign" code:-1 userInfo:@{NSLocalizedDescriptionKey: @"Output path is required"}]);
		}
		return -1;
	}

	NSString* standardized = [folderPath stringByStandardizingPath];
	BOOL isDir = NO;
	NSFileManager* fm = [NSFileManager defaultManager];
	if (![fm fileExistsAtPath:standardized isDirectory:&isDir] || !isDir) {
		ZLog::ErrorV(">>> Archive: invalid folder: %s\n", [standardized UTF8String]);
		if (completionHandler) {
			completionHandler(NO, [NSError errorWithDomain:@"Zsign" code:-1 userInfo:@{NSLocalizedDescriptionKey: @"Invalid folder path"}]);
		}
		return -1;
	}

	if ([[standardized lastPathComponent] compare:@"Payload" options:NSCaseInsensitiveSearch] != NSOrderedSame) {
		ZLog::Error(">>> Archive: path must be the Payload directory (…/Payload).\n");
		if (completionHandler) {
			completionHandler(NO, [NSError errorWithDomain:@"Zsign" code:-1 userInfo:@{NSLocalizedDescriptionKey: @"Path must be the Payload directory (ending with /Payload)"}]);
		}
		return -1;
	}

	NSError* enumErr = nil;
	NSArray<NSString*>* contents = [fm contentsOfDirectoryAtPath:standardized error:&enumErr];
	if (!contents) {
		ZLog::ErrorV(">>> Archive: cannot list Payload: %s\n", enumErr ? [[enumErr localizedDescription] UTF8String] : "unknown");
		if (completionHandler) {
			completionHandler(NO, [NSError errorWithDomain:@"Zsign" code:-1 userInfo:@{NSLocalizedDescriptionKey: @"Cannot read Payload directory"}]);
		}
		return -1;
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
		if (completionHandler) {
			completionHandler(NO, [NSError errorWithDomain:@"Zsign" code:-1 userInfo:@{NSLocalizedDescriptionKey: @"Payload must contain exactly one .app bundle (none found)"}]);
		}
		return -1;
	}
	if (appCount > 1) {
		ZLog::Error(">>> Archive: Payload must contain exactly one .app bundle (multiple .app found).\n");
		if (completionHandler) {
			completionHandler(NO, [NSError errorWithDomain:@"Zsign" code:-1 userInfo:@{NSLocalizedDescriptionKey: @"Payload must contain exactly one .app bundle (multiple found)"}]);
		}
		return -1;
	}

	string strPayloadFolder = [standardized UTF8String];
	string strOutput = [outputPath UTF8String];

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

int zsignExtractIPA(
	NSString* ipaPath,
	NSString* outputFolderPath,
	bool zh,
	void (^ _Nullable completionHandler)(BOOL success, NSError* _Nullable error))
{
	ZsignLangScope langScope(zh);

	ZTimer atimer;
	ZTimer gtimer;

	if (!ipaPath || [ipaPath length] == 0) {
		ZLog::Error(">>> Extract: input path is required.\n");
		if (completionHandler) {
			completionHandler(NO, [NSError errorWithDomain:@"Zsign" code:-1 userInfo:@{NSLocalizedDescriptionKey: @"Input path is required"}]);
		}
		return -1;
	}
	if (!outputFolderPath || [outputFolderPath length] == 0) {
		ZLog::Error(">>> Extract: output folder is required.\n");
		if (completionHandler) {
			completionHandler(NO, [NSError errorWithDomain:@"Zsign" code:-1 userInfo:@{NSLocalizedDescriptionKey: @"Output folder is required"}]);
		}
		return -1;
	}

	string strPath = [ipaPath cStringUsingEncoding:NSUTF8StringEncoding];
	string strOut = [outputFolderPath cStringUsingEncoding:NSUTF8StringEncoding];

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
	if (!ZsignPathHasIpaExtension(ipaPath)) {
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
	if (!ZsignRenameExistingPayloadIfNeeded(strOut.c_str())) {
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

void ZsignRequestZipArchiveCancel(void)
{
	ZsignRequestZipCancel();
}

bool ZsignZipArchiveLastFailureWasUserCancel(void)
{
	return ZsignZipLastFailureWasUserCancel();
}

}
