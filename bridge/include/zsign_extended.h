#ifndef zsign_extended_h
#define zsign_extended_h

#include <stddef.h>
#include <stdbool.h>
#import <Foundation/Foundation.h>

#ifdef __cplusplus
extern "C" {
#endif

NS_ASSUME_NONNULL_BEGIN

typedef struct {
	void* _Nullable cert;
	void* _Nullable issuer;
} ZsignCertLoaded;

void ZsignZipBeginOperation(void);

int ZsignArchivePayloadDirectoryToIPA(
	const char* _Nonnull payloadFolderUTF8,
	const char* _Nonnull outputPathUTF8,
	int zipLevel,
	void (^ _Nullable completionHandler)(BOOL success, NSError* _Nullable error));

int ZsignExtractIPAIntoDirectory(
	const char* _Nonnull ipaPathUTF8,
	const char* _Nonnull outputFolderUTF8,
	void (^ _Nullable completionHandler)(BOOL success, NSError* _Nullable error));

bool ZsignExtendedMachoCheckSigned(const char* _Nonnull path);
bool ZsignExtendedMachoInjectDylib(const char* _Nonnull path, const char* _Nonnull dylibPath, bool weakInject);
bool ZsignExtendedMachoRemoveDylibs(const char* _Nonnull path, const char* _Nonnull const* _Nonnull dylibs, size_t dylibCount);
bool ZsignExtendedMachoListDylibs(const char* _Nonnull path, void (^ _Nonnull append)(const char* _Nonnull dylibPath));
bool ZsignExtendedMachoChangeDylibPath(const char* _Nonnull path, const char* _Nonnull oldPath, const char* _Nonnull newPath);

bool ZsignLoadCheckCertAssets(
	const char* _Nonnull pkeyPath,
	const char* _Nonnull provPath,
	const char* _Nonnull password,
	ZsignCertLoaded* _Nonnull out,
	char* _Nullable errorOut,
	size_t errorOutLen);

void ZsignFreeCheckCertLoaded(ZsignCertLoaded* _Nonnull loaded);

NS_ASSUME_NONNULL_END

#ifdef __cplusplus
}
#endif

#endif /* zsign_extended_h */
