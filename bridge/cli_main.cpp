//
//  cli_main.cpp — master CLI 入口，解析 argv 后委托 zsignRun()
//

#include "zsign.hpp"
#include "i18n/zlog_i18n.h"
#include "common.h"

#include <cstdlib>
#include <cstring>
#include <getopt.h>
#include <string>
#include <vector>

#ifndef ZSIGN_VERSION
#define ZSIGN_VERSION 0.0.0-dev
#endif
#define ZSIGN_STR_(x) #x
#define ZSIGN_STR(x) ZSIGN_STR_(x)
#define ZSIGN_VERSION_STR ZSIGN_STR(ZSIGN_VERSION)

static const struct option kOptions[] = {
	{"debug", no_argument, NULL, 'd'},
	{"force", no_argument, NULL, 'f'},
	{"version", no_argument, NULL, 'v'},
	{"adhoc", no_argument, NULL, 'a'},
	{"cert", required_argument, NULL, 'c'},
	{"pkey", required_argument, NULL, 'k'},
	{"prov", required_argument, NULL, 'm'},
	{"password", required_argument, NULL, 'p'},
	{"bundle_id", required_argument, NULL, 'b'},
	{"bundle_name", required_argument, NULL, 'n'},
	{"bundle_version", required_argument, NULL, 'r'},
	{"entitlements", required_argument, NULL, 'e'},
	{"icon", required_argument, NULL, 'I'},
	{"output", required_argument, NULL, 'o'},
	{"zip_level", required_argument, NULL, 'z'},
	{"dylib", required_argument, NULL, 'l'},
	{"rm_dylib", required_argument, NULL, 'D'},
	{"weak", no_argument, NULL, 'w'},
	{"temp_folder", required_argument, NULL, 't'},
	{"sha256_only", no_argument, NULL, '2'},
	{"legacy_sha1", no_argument, NULL, 'L'},
	{"install", no_argument, NULL, 'i'},
	{"check", no_argument, NULL, 'C'},
	{"quiet", no_argument, NULL, 'q'},
	{"metadata", required_argument, NULL, 'x'},
	{"rm_provision", no_argument, NULL, 'R'},
	{"enable_docs", no_argument, NULL, 'S'},
	{"min_version", required_argument, NULL, 'M'},
	{"rm_extensions", no_argument, NULL, 'E'},
	{"rm_watch", no_argument, NULL, 'W'},
	{"rm_uisd", no_argument, NULL, 'U'},
	{"inject_extensions", no_argument, NULL, 'P'},
	{"help", no_argument, NULL, 'h'},
	{}
};

static bool EnvZhEnabled()
{
	const char* lang = getenv("ZSIGN_LANG");
	if (!lang || !*lang) {
		lang = getenv("LANG");
	}
	if (!lang) {
		return false;
	}
	return (strncmp(lang, "zh", 2) == 0);
}

static int Usage(void)
{
	puts(ZsignHelpText());
	return -1;
}

int main(int argc, char* argv[])
{
	std::vector<std::string> provStorage;
	std::vector<std::string> dylibStorage;
	std::vector<std::string> removeDylibStorage;
	std::vector<const char*> provPtrs;
	std::vector<const char*> dylibPtrs;
	std::vector<const char*> removeDylibPtrs;

	std::string certFile;
	std::string pkeyFile;
	std::string password;
	std::string bundleId;
	std::string bundleName;
	std::string bundleVersion;
	std::string outputFile;
	std::string entitlements;
	std::string iconFile;
	std::string metadataDir;
	std::string tempFolder;
	std::string minVersion;

	ZsignRunOptions opts = {};
	opts.sha256Only = true;
	tempFolder = ZFile::GetTempFolder();
	opts.tempFolder = tempFolder.c_str();
	opts.zh = EnvZhEnabled();

	int opt = 0;
	int argslot = -1;
	while ((opt = getopt_long(argc, argv, "dfva2LhiqwCRSEWUPc:k:m:o:p:e:b:n:z:l:D:t:r:x:M:I:",
		kOptions, &argslot)) != -1) {
		switch (opt) {
		case 'd':
			opts.debug = true;
			break;
		case 'f':
			opts.force = true;
			break;
		case 'c':
			certFile = ZFile::GetFullPath(optarg);
			opts.cert = certFile.c_str();
			break;
		case 'k':
			pkeyFile = ZFile::GetFullPath(optarg);
			opts.pkey = pkeyFile.c_str();
			break;
		case 'm':
			provStorage.push_back(ZFile::GetFullPath(optarg));
			break;
		case 'a':
			opts.adhoc = true;
			break;
		case 'p':
			password = optarg;
			opts.password = password.c_str();
			break;
		case 'b':
			bundleId = optarg;
			opts.bundleId = bundleId.c_str();
			break;
		case 'r':
			bundleVersion = optarg;
			opts.bundleVersion = bundleVersion.c_str();
			break;
		case 'n':
			bundleName = optarg;
			opts.bundleName = bundleName.c_str();
			break;
		case 'e':
			entitlements = ZFile::GetFullPath(optarg);
			opts.entitlements = entitlements.c_str();
			break;
		case 'I':
			iconFile = ZFile::GetFullPath(optarg);
			opts.icon = iconFile.c_str();
			break;
		case 'l':
			dylibStorage.push_back(ZFile::GetFullPath(optarg));
			break;
		case 'D':
			removeDylibStorage.push_back(optarg);
			break;
		case 'i':
			opts.install = true;
			break;
		case 'o':
			outputFile = ZFile::GetFullPath(optarg);
			opts.output = outputFile.c_str();
			break;
		case 'z':
			opts.zipLevel = atoi(optarg);
			break;
		case 'w':
			opts.weakInject = true;
			break;
		case 't':
			tempFolder = ZFile::GetFullPath(optarg);
			opts.tempFolder = tempFolder.c_str();
			break;
		case '2':
			opts.sha256Only = true;
			break;
		case 'L':
			opts.legacySHA1 = true;
			opts.sha256Only = false;
			break;
		case 'C':
			opts.check = true;
			break;
		case 'q':
			opts.quiet = true;
			break;
		case 'x':
			metadataDir = ZFile::GetFullPath(optarg);
			opts.metadata = metadataDir.c_str();
			break;
		case 'R':
			opts.removeProvision = true;
			break;
		case 'S':
			opts.enableDocs = true;
			break;
		case 'M':
			minVersion = optarg;
			opts.minVersion = minVersion.c_str();
			break;
		case 'E':
			opts.removeExtensions = true;
			break;
		case 'W':
			opts.removeWatch = true;
			break;
		case 'U':
			opts.removeUISD = true;
			break;
		case 'P':
			opts.injectExtensions = true;
			break;
		case 'v':
			puts(ZsignVersionString());
			return 0;
		case 'h':
		case '?':
			return Usage();
		default:
			break;
		}
	}

	if (optind >= argc) {
		return Usage();
	}

	for (const auto& p : provStorage) {
		provPtrs.push_back(p.c_str());
	}
	for (const auto& d : dylibStorage) {
		dylibPtrs.push_back(d.c_str());
	}
	for (const auto& r : removeDylibStorage) {
		removeDylibPtrs.push_back(r.c_str());
	}
	opts.provisionPaths = provPtrs.empty() ? NULL : provPtrs.data();
	opts.provisionPathCount = provPtrs.size();
	opts.dylibs = dylibPtrs.empty() ? NULL : dylibPtrs.data();
	opts.dylibCount = dylibPtrs.size();
	opts.removeDylibs = removeDylibPtrs.empty() ? NULL : removeDylibPtrs.data();
	opts.removeDylibCount = removeDylibPtrs.size();

	return zsignRun(argv[optind], &opts, NULL);
}
