#pragma once

void ZipBeginZipOperation();
void ZipRequestCancel();
bool ZipIsCancelRequested();
bool ZipLastFailureWasUserCancel();
void ZipMarkStoppedByUserCancel();
