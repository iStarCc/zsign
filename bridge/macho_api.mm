#include "zsign.hpp"

#include "macho_dylib_overlay.h"

#include "common.h"
#include "macho.h"
#include "timer.h"

#include <set>
#include <string>
#include <vector>

extern "C" {

bool CheckIfSigned(NSString* filePath)
{
	ZTimer gtimer;
	@autoreleasepool {
		std::string filePathStr = [filePath UTF8String];

		ZMachO machO;
		if (!machO.Init(filePathStr.c_str())) {
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
}

bool InjectDyLib(NSString* filePath, NSString* dylibPath, bool weakInject)
{
	ZTimer gtimer;
	@autoreleasepool {
		std::string filePathStr = [filePath UTF8String];
		std::string dylibPathStr = [dylibPath UTF8String];

		ZMachO machO;
		if (!machO.Init(filePathStr.c_str())) {
			gtimer.Print(">>> Failed to initialize ZMachO.");
			return false;
		}

		bool success = machO.InjectDylib(weakInject, dylibPathStr.c_str());
		machO.Free();

		if (success) {
			gtimer.Print(">>> Dylib injected successfully!");
			return true;
		}
		gtimer.Print(">>> Failed to inject dylib.");
		return false;
	}
}

bool UninstallDylibs(NSString* filePath, NSArray<NSString*>* dylibPathsArray)
{
	ZTimer gtimer;
	@autoreleasepool {
		std::string filePathStr = [filePath UTF8String];
		std::set<std::string> dylibsToRemove;

		for (NSString* dylibPath in dylibPathsArray) {
			dylibsToRemove.insert([dylibPath UTF8String]);
		}

		ZMachO machO;
		if (!machO.Init(filePathStr.c_str())) {
			gtimer.Print(">>> Failed to initialize ZMachO.");
			return false;
		}

		machO.RemoveDylibs(dylibsToRemove);
		machO.Free();

		gtimer.Print(">>> Dylibs uninstalled successfully!");
		return true;
	}
}

NSArray<NSString*>* _Nullable ListDylibs(NSString* filePath)
{
	ZTimer gtimer;
	@autoreleasepool {
		NSMutableArray<NSString*>* dylibPathsArray = [NSMutableArray array];
		std::string filePathStr = [filePath UTF8String];

		if (!ZFile::IsFileExists(filePathStr.c_str())) {
			gtimer.Print(">>> Failed to initialize ZMachO.");
			return nil;
		}

		std::vector<std::string> dylibPaths;
		if (!MachoBridgeListDylibs(filePathStr.c_str(), dylibPaths)) {
			gtimer.Print(">>> Failed to list dylibs.");
			return nil;
		}
		if (!dylibPaths.empty()) {
			gtimer.Print(">>> List of dylibs in the Mach-O file:");
			for (const std::string& dylibPath : dylibPaths) {
				[dylibPathsArray addObject:[NSString stringWithUTF8String:dylibPath.c_str()]];
			}
		} else {
			gtimer.Print(">>> No dylibs found in the Mach-O file.");
		}

		return [dylibPathsArray copy];
	}
}

bool ChangeDylibPath(NSString* filePath, NSString* oldPath, NSString* newPath)
{
	ZTimer gtimer;
	@autoreleasepool {
		std::string filePathStr = [filePath UTF8String];
		std::string oldPathStr = [oldPath UTF8String];
		std::string newPathStr = [newPath UTF8String];

		if (!ZFile::IsFileExists(filePathStr.c_str())) {
			gtimer.Print(">>> Failed to initialize ZMachO.");
			return false;
		}

		bool success = MachoBridgeChangeDylibPath(filePathStr.c_str(), oldPathStr.c_str(), newPathStr.c_str());

		if (success) {
			gtimer.Print(">>> Dylib path changed successfully!");
			return true;
		}
		gtimer.Print(">>> Failed to change dylib path.");
		return false;
	}
}

}
