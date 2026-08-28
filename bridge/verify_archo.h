#pragma once

class ZArchO;

bool VerifyArchOCodeSlots(const ZArchO* archo, bool bSilent);
bool VerifyArchOEmbeddedSignature(const ZArchO* archo, bool bCheckCMS);
