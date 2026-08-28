#ifndef ipax_bridge_h
#define ipax_bridge_h

#include <stddef.h>
#include <stdbool.h>
#import <Foundation/Foundation.h>

#ifdef __cplusplus
extern "C" {
#endif

NS_ASSUME_NONNULL_BEGIN

bool CheckIfSigned(NSString* filePath);
bool InjectDyLib(NSString* filePath, NSString* dylibPath, bool weakInject);
bool UninstallDylibs(NSString* filePath, NSArray<NSString*>* dylibPathsArray);
NSArray<NSString*>* _Nullable ListDylibs(NSString* filePath);
bool ChangeDylibPath(NSString* filePath, NSString* oldPath, NSString* newPath);

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

void ZsignRequestZipArchiveCancel(void);
bool ZsignZipArchiveLastFailureWasUserCancel(void);

NS_ASSUME_NONNULL_END

#ifdef __cplusplus
}
#endif

#endif /* ipax_bridge_h */
