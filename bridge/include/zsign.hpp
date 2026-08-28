#ifndef zsign_hpp
#define zsign_hpp

#include <stddef.h>
#include <stdbool.h>
#import <Foundation/Foundation.h>

#ifdef __cplusplus
extern "C" {
#endif

NS_ASSUME_NONNULL_BEGIN

typedef struct {
	const char* _Nullable pkey;
	const char* _Nullable cert;
	const char* _Nullable password;
	const char* _Nullable bundleId;
	const char* _Nullable bundleName;
	const char* _Nullable bundleVersion;
	const char* _Nullable entitlements;
	const char* _Nullable icon;
	const char* _Nullable output;
	const char* _Nullable tempFolder;
	const char* _Nullable metadata;
	const char* _Nullable minVersion;
	const char* _Nullable const* _Nullable provisionPaths;
	size_t provisionPathCount;
	const char* _Nullable const* _Nullable dylibs;
	size_t dylibCount;
	const char* _Nullable const* _Nullable removeDylibs;
	size_t removeDylibCount;
	const char* _Nullable const* _Nullable removePaths;
	size_t removePathCount;
	int zipLevel;
	bool adhoc;
	bool debug;
	bool force;
	bool weakInject;
	bool install;
	bool sha256Only;
	bool legacySHA1;
	bool check;
	bool removeProvision;
	bool enableDocs;
	bool removeExtensions;
	bool removeWatch;
	bool removeUISD;
	bool injectExtensions;
	bool quiet;
	bool zh;
} ZsignRunOptions;

typedef void (^ZsignCompletionHandler)(BOOL success, NSError* _Nullable error);

/// 完整复刻 master `zsign.cpp` main() 流程；返回 0 成功，-1 失败。
int zsignRun(
	const char* inputPath,
	const ZsignRunOptions* options,
	void (^ _Nullable completionHandler)(BOOL success, NSError* _Nullable error)
);

const char* ZsignVersionString(void);
const char* ZsignHelpText(void);

void ZsignSetLogHandler(void (^ _Nullable handler)(NSString* _Nullable line));

void ZsignRequestZipCancel(void);
bool ZsignZipLastFailureWasUserCancel(void);

/** zsign-ipax 兼容别名 */
void ZsignRequestZipArchiveCancel(void);
bool ZsignZipArchiveLastFailureWasUserCancel(void);

bool ZsignRemoveIPAPackagingJunkFromFolder(
	const char* _Nonnull szRoot,
	const char* _Nullable const* _Nullable removePaths,
	size_t removePathCount);

bool ZsignVerifySignedBundle(const char* _Nonnull appFolder, bool bCheckCMS);

/* Mach-O 细粒度 API（zsign-ipax 兼容） */
bool CheckIfSigned(NSString* filePath);
bool InjectDyLib(NSString* filePath, NSString* dylibPath, bool weakInject);
bool UninstallDylibs(NSString* filePath, NSArray<NSString*>* dylibPathsArray);
NSArray<NSString*>* _Nullable ListDylibs(NSString* filePath);
bool ChangeDylibPath(NSString* filePath, NSString* oldPath, NSString* newPath);

/* 高层签名 / 打包 API（zsign-ipax 兼容） */
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
	void (^ _Nullable completionHandler)(BOOL success, NSError* _Nullable error)
);

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
	void (^ _Nullable completionHandler)(BOOL success, NSError* _Nullable error)
);

int zsignArchiveFolderToIPA(
	NSString* folderPath,
	NSString* outputPath,
	int zipLevel,
	bool zh,
	void (^ _Nullable completionHandler)(BOOL success, NSError* _Nullable error)
);

int zsignExtractIPA(
	NSString* ipaPath,
	NSString* outputFolderPath,
	bool zh,
	void (^ _Nullable completionHandler)(BOOL success, NSError* _Nullable error)
);

int checkCert(
	NSString* prov,
	NSString* key,
	NSString* pass,
	void (^completionHandler)(int status, NSDate* _Nullable expirationDate, NSString* _Nullable error)
);

NS_ASSUME_NONNULL_END

#ifdef __cplusplus
}
#endif

#endif /* zsign_hpp */
