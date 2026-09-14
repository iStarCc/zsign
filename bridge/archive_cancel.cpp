#include "archive_cancel.h"

#include <atomic>

namespace {

std::atomic<bool> g_zipCancelRequested{ false };
std::atomic<bool> g_zipLastFailureWasUserCancel{ false };

} // namespace

void ZipBeginZipOperation()
{
	g_zipCancelRequested.store(false, std::memory_order_release);
	g_zipLastFailureWasUserCancel.store(false, std::memory_order_release);
}

void ZipRequestCancel()
{
	g_zipCancelRequested.store(true, std::memory_order_release);
}

bool ZipIsCancelRequested()
{
	return g_zipCancelRequested.load(std::memory_order_acquire);
}

bool ZipLastFailureWasUserCancel()
{
	return g_zipLastFailureWasUserCancel.load(std::memory_order_acquire);
}

void ZipMarkStoppedByUserCancel()
{
	g_zipLastFailureWasUserCancel.store(true, std::memory_order_release);
}
