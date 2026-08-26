#include "fs_ipa_junk.h"

#include "common.h"

#include <algorithm>
#include <cstring>
#include <fnmatch.h>
#include <utility>
#include <vector>

#if !defined(S_ISDIR) && defined(S_IFMT) && defined(S_IFDIR)
#define S_ISDIR(m) (((m)&S_IFMT) == S_IFDIR)
#endif

namespace {

struct CleanupContext {
	string displayRoot;
	size_t removedCount;

	explicit CleanupContext(const string& root)
		: displayRoot(root)
		, removedCount(0)
	{
	}

	static string RelativeDisplayPath(const string& root, const string& fullPath)
	{
		if (fullPath.size() >= root.size() && 0 == strncmp(fullPath.c_str(), root.c_str(), root.size())) {
			string rel = fullPath.substr(root.size());
			if (!rel.empty() && ('/' == rel[0] || '\\' == rel[0])) {
				rel = rel.substr(1);
			}
			if (!rel.empty()) {
				return rel;
			}
		}
		return ZUtil::GetBaseName(fullPath.c_str());
	}

	void LogRemoved(const string& fullPath)
	{
		ZLog::PrintV(">>> Removed: %s\n", RelativeDisplayPath(displayRoot, fullPath).c_str());
		removedCount++;
	}
};

static bool IsIPAJunkTopLevelDirName(const char* name)
{
	if (NULL == name || !*name) {
		return false;
	}
	if (0 == strcmp(name, "__MACOSX")
		|| 0 == strcmp(name, ".Spotlight-V100")
		|| 0 == strcmp(name, ".Trashes")
		|| 0 == strcmp(name, ".fseventsd")
		|| 0 == strcmp(name, ".AppleDouble")
		|| 0 == strcmp(name, ".LSOverride")
		|| 0 == strcmp(name, ".zsign_cache")) {
		return true;
	}
	if (name[0] == '.' && name[1] == '_' && name[2] != 0) {
		return true;
	}
	return false;
}

static bool IsIPAJunkFileName(const char* name)
{
	if (NULL == name || !*name) {
		return false;
	}
	if (0 == strcmp(name, ".DS_Store") || 0 == strcmp(name, ".LSOverride")) {
		return true;
	}
	if (name[0] == '.' && name[1] == '_' && name[2] != 0) {
		return true;
	}
	return false;
}

static void RemoveIPAJunkInFolderRecursive(const string& strFolder, CleanupContext& ctx)
{
#ifdef _WIN32
	string strFrom = strFolder + "\\*";
	WIN32_FIND_DATAA fd = { 0 };
	HANDLE hFind = ::FindFirstFileA(strFrom.c_str(), &fd);
	if (INVALID_HANDLE_VALUE == hFind) {
		return;
	}
	do {
		if (0 == strcmp(fd.cFileName, ".") || 0 == strcmp(fd.cFileName, "..")) {
			continue;
		}
		string strPath = strFolder + "\\" + fd.cFileName;
		bool bFolder = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
		if (bFolder) {
			if (IsIPAJunkTopLevelDirName(fd.cFileName)) {
				if (ZFile::RemoveFolder(strPath.c_str())) {
					ctx.LogRemoved(strPath);
				}
			} else {
				RemoveIPAJunkInFolderRecursive(strPath, ctx);
			}
		} else {
			if (IsIPAJunkFileName(fd.cFileName)) {
				if (ZFile::RemoveFile(strPath.c_str())) {
					ctx.LogRemoved(strPath);
				}
			}
		}
	} while (::FindNextFileA(hFind, &fd));
	::FindClose(hFind);
#else
	DIR* dir = opendir(strFolder.c_str());
	if (NULL == dir) {
		return;
	}
	dirent* ptr = readdir(dir);
	while (NULL != ptr) {
		if (0 == strcmp(ptr->d_name, ".") || 0 == strcmp(ptr->d_name, "..")) {
			ptr = readdir(dir);
			continue;
		}
		string strPath = strFolder + "/" + ptr->d_name;
		bool bFolder = false;
		if (DT_DIR == ptr->d_type) {
			bFolder = true;
		} else if (DT_UNKNOWN == ptr->d_type) {
			struct stat st = { 0 };
			if (0 == stat(strPath.c_str(), &st) && S_ISDIR(st.st_mode)) {
				bFolder = true;
			}
		}
		if (bFolder) {
			if (IsIPAJunkTopLevelDirName(ptr->d_name)) {
				if (ZFile::RemoveFolder(strPath.c_str())) {
					ctx.LogRemoved(strPath);
				}
			} else {
				RemoveIPAJunkInFolderRecursive(strPath, ctx);
			}
		} else {
			if (IsIPAJunkFileName(ptr->d_name)) {
				if (ZFile::RemoveFile(strPath.c_str())) {
					ctx.LogRemoved(strPath);
				}
			}
		}
		ptr = readdir(dir);
	}
	closedir(dir);
#endif
}

static bool IsCustomPatternValid(const char* pattern)
{
	if (NULL == pattern || !*pattern) {
		return false;
	}
	if (NULL != strchr(pattern, '/')
		|| NULL != strchr(pattern, '\\')
		|| NULL != strstr(pattern, "..")
		|| NULL != strchr(pattern, ':')) {
		return false;
	}
	return true;
}

static bool ResolveAppRoot(const string& strFolder, string& outAppRoot)
{
	if (ZFile::IsPathSuffix(strFolder, ".app")) {
		outAppRoot = strFolder;
		return true;
	}

	string strPayload = strFolder + "/Payload";
	if (!ZFile::IsFolder(strPayload.c_str())) {
		return false;
	}

	vector<string> apps;
	ZFile::EnumFolder(strPayload.c_str(), false, NULL, [&](bool bFolder, const string& strPath) {
		if (bFolder && ZFile::IsPathSuffix(strPath, ".app")) {
			apps.push_back(strPath);
		}
		return false;
	});

	if (apps.size() != 1) {
		if (apps.size() > 1) {
			ZLog::ErrorV(">>> Multiple .app bundles in Payload, skip custom remove paths.\n");
		}
		return false;
	}

	outAppRoot = apps.front();
	return true;
}

static void RemovePathIfExists(const string& strPath, bool bFolder, CleanupContext& ctx)
{
	if (bFolder) {
		if (ZFile::IsFolder(strPath.c_str()) && ZFile::RemoveFolder(strPath.c_str())) {
			ctx.LogRemoved(strPath);
		} else if (ZFile::IsFolder(strPath.c_str())) {
			ZLog::ErrorV(">>> Failed to remove folder: %s\n", strPath.c_str());
		}
	} else if (ZFile::IsFileExists(strPath.c_str()) && ZFile::RemoveFile(strPath.c_str())) {
		ctx.LogRemoved(strPath);
	} else if (ZFile::IsFileExists(strPath.c_str())) {
		ZLog::ErrorV(">>> Failed to remove file: %s\n", strPath.c_str());
	}
}

static void RemoveRootLevelCustomEntry(const string& appRoot, const char* pattern, CleanupContext& ctx)
{
	string target = appRoot + "/" + pattern;
	if (ZFile::IsFolder(target.c_str())) {
		RemovePathIfExists(target, true, ctx);
	} else if (ZFile::IsFileExists(target.c_str())) {
		RemovePathIfExists(target, false, ctx);
	}
}

static void CollectGlobMatches(
	const string& appRoot,
	const char* pattern,
	vector<pair<string, bool>>& outMatches)
{
	ZFile::EnumFolder(appRoot.c_str(), true, NULL, [&](bool bFolder, const string& strPath) {
		const char* base = ZUtil::GetBaseName(strPath.c_str());
		if (0 == fnmatch(pattern, base, 0)) {
			outMatches.emplace_back(strPath, bFolder);
		}
		return false;
	});
}

static void RemoveCustomPathsFromApp(
	const string& appRoot,
	const char* const* customRemovePaths,
	size_t customRemovePathCount,
	CleanupContext& ctx)
{
	if (NULL == customRemovePaths || 0 == customRemovePathCount) {
		return;
	}

	vector<pair<string, bool>> globMatches;

	for (size_t i = 0; i < customRemovePathCount; i++) {
		const char* pattern = customRemovePaths[i];
		if (!IsCustomPatternValid(pattern)) {
			ZLog::ErrorV(">>> Invalid custom remove pattern (skipped): %s\n", pattern ? pattern : "(null)");
			continue;
		}

		if (NULL == strchr(pattern, '*')) {
			RemoveRootLevelCustomEntry(appRoot, pattern, ctx);
			continue;
		}

		CollectGlobMatches(appRoot, pattern, globMatches);
	}

	if (globMatches.empty()) {
		return;
	}

	std::sort(globMatches.begin(), globMatches.end(), [](const pair<string, bool>& a, const pair<string, bool>& b) {
		return a.first.size() > b.first.size();
	});

	for (const pair<string, bool>& item : globMatches) {
		if (item.second) {
			if (ZFile::IsFolder(item.first.c_str())) {
				RemovePathIfExists(item.first, true, ctx);
			}
		} else if (ZFile::IsFileExists(item.first.c_str())) {
			RemovePathIfExists(item.first, false, ctx);
		}
	}
}

} // namespace

bool RemoveIPAPackagingJunkFromFolder(
	const char* szRoot,
	const char* const* customRemovePaths,
	size_t customRemovePathCount)
{
	if (NULL == szRoot || !*szRoot) {
		return false;
	}
	if (!ZFile::IsFolder(szRoot)) {
		return false;
	}

	CleanupContext ctx{string(szRoot)};
	ZLog::Print(">>> Cleaning IPA packaging junk...\n");
	RemoveIPAJunkInFolderRecursive(string(szRoot), ctx);

	string appRoot;
	if (ResolveAppRoot(string(szRoot), appRoot)) {
		RemoveCustomPathsFromApp(appRoot, customRemovePaths, customRemovePathCount, ctx);
	}

	if (ctx.removedCount > 0) {
		ZLog::PrintV(">>> Cleaned %zu item(s).\n", ctx.removedCount);
	}
	ZLog::Print("\n");

	return true;
}
