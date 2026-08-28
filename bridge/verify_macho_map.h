#pragma once

bool VerifyMachOFileCodeSlots(const char* path);
bool VerifyMachOFileEmbeddedSignature(const char* path, bool bCheckCMS);
