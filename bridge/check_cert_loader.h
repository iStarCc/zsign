#pragma once

#include "common.h"

struct CheckCertLoaded {
	void* cert;
	void* issuer;
};

bool LoadCheckCertAssets(
	const string& strPKeyFile,
	const string& strProvFile,
	const string& strPassword,
	CheckCertLoaded& out,
	string& outError);

void FreeCheckCertLoaded(CheckCertLoaded& loaded);
