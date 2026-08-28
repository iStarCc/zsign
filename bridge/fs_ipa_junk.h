#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool RemoveIPAPackagingJunkFromFolder(
	const char* szRoot,
	const char* const* customRemovePaths,
	size_t customRemovePathCount);

#ifdef __cplusplus
}
#endif
