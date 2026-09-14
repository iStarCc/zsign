#ifndef zsign_hpp
#define zsign_hpp

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	const char* pkey;
	const char* cert;
	const char* password;
	const char* bundleId;
	const char* bundleName;
	const char* bundleVersion;
	const char* entitlements;
	const char* icon;
	const char* output;
	const char* tempFolder;
	const char* metadata;
	const char* minVersion;
	const char* const* provisionPaths;
	size_t provisionPathCount;
	const char* const* dylibs;
	size_t dylibCount;
	const char* const* removeDylibs;
	size_t removeDylibCount;
	const char* const* removePaths;
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

typedef void (*ZsignCompletionFn)(bool success, const char* errorMessage);

int zsignRun(
	const char* inputPath,
	const ZsignRunOptions* options,
	ZsignCompletionFn completionHandler
);

const char* ZsignVersionString(void);
const char* ZsignHelpText(void);

typedef void (*ZsignLogHandlerFn)(const char* line);
void ZsignSetLogHandler(ZsignLogHandlerFn handler);

void ZsignRequestZipCancel(void);
bool ZsignZipLastFailureWasUserCancel(void);

bool ZsignRemoveIPAPackagingJunkFromFolder(
	const char* szRoot,
	const char* const* removePaths,
	size_t removePathCount
);

bool ZsignVerifySignedBundle(const char* appFolder, bool bCheckCMS);

#ifdef __cplusplus
}
#endif

#endif /* zsign_hpp */
