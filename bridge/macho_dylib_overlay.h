#pragma once

#include "common.h"

#include <vector>

bool MachoBridgeListDylibs(const char* path, std::vector<std::string>& out);
bool MachoBridgeChangeDylibPath(const char* path, const char* oldPath, const char* newPath);
