#include "ipax_bridge.h"

#include "zsign_extended.h"

extern "C" {

bool CheckIfSigned(NSString* filePath)
{
	@autoreleasepool {
		if (!filePath) {
			return false;
		}
		return ZsignExtendedMachoCheckSigned([filePath UTF8String]);
	}
}

bool InjectDyLib(NSString* filePath, NSString* dylibPath, bool weakInject)
{
	@autoreleasepool {
		if (!filePath || !dylibPath) {
			return false;
		}
		return ZsignExtendedMachoInjectDylib([filePath UTF8String], [dylibPath UTF8String], weakInject);
	}
}

bool UninstallDylibs(NSString* filePath, NSArray<NSString*>* dylibPathsArray)
{
	@autoreleasepool {
		if (!filePath || !dylibPathsArray || [dylibPathsArray count] == 0) {
			return false;
		}

		NSMutableArray<NSString*>* paths = [dylibPathsArray mutableCopy];
		size_t count = [paths count];
		NSMutableData* storage = [NSMutableData dataWithLength:sizeof(const char*) * count];
		const char** cPaths = (const char**)storage.mutableBytes;
		for (size_t i = 0; i < count; i++) {
			cPaths[i] = [[paths objectAtIndex:i] UTF8String];
		}
		return ZsignExtendedMachoRemoveDylibs([filePath UTF8String], cPaths, count);
	}
}

NSArray<NSString*>* _Nullable ListDylibs(NSString* filePath)
{
	@autoreleasepool {
		if (!filePath) {
			return nil;
		}
		NSMutableArray<NSString*>* dylibPathsArray = [NSMutableArray array];
		if (!ZsignExtendedMachoListDylibs([filePath UTF8String], ^(const char* dylibPath) {
			[dylibPathsArray addObject:[NSString stringWithUTF8String:dylibPath]];
		})) {
			return nil;
		}
		return [dylibPathsArray copy];
	}
}

bool ChangeDylibPath(NSString* filePath, NSString* oldPath, NSString* newPath)
{
	@autoreleasepool {
		if (!filePath || !oldPath || !newPath) {
			return false;
		}
		return ZsignExtendedMachoChangeDylibPath(
			[filePath UTF8String],
			[oldPath UTF8String],
			[newPath UTF8String]);
	}
}

}
