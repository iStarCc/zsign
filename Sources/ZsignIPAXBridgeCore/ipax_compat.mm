#include "ipax_bridge.h"

#include "zsign.hpp"
#include "zsign_extended.h"

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
	@autoreleasepool {
		if (!folderPath || [folderPath length] == 0) {
			if (completionHandler) {
				completionHandler(NO, [NSError errorWithDomain:@"Zsign" code:-1 userInfo:@{NSLocalizedDescriptionKey: @"Folder path is required"}]);
			}
			return -1;
		}

		NSString* standardized = [folderPath stringByStandardizingPath];
		return ZsignArchivePayloadDirectoryToIPA(
			[standardized UTF8String],
			NsUtf8(outputPath),
			zipLevel,
			completionHandler);
	}
}

int zsignExtractIPA(
	NSString* ipaPath,
	NSString* outputFolderPath,
	bool zh,
	void (^ _Nullable completionHandler)(BOOL success, NSError* _Nullable error))
{
	ZsignLangScope langScope(zh);
	return ZsignExtractIPAIntoDirectory(
		NsUtf8(ipaPath),
		NsUtf8(outputFolderPath),
		completionHandler);
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
