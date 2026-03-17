#include "timer.h"
#include "l10n.h"
#include "log_keys.h"
#include <stdio.h>

ZTimer::ZTimer()
{
	Reset();
}

uint64_t ZTimer::Reset()
{
	m_uBeginTime = ZUtil::GetMicroSecond();
	return m_uBeginTime;
}

static void FormatElapsed(uint64_t uElapse, char* buf, size_t bufSize)
{
	uint64_t h = uElapse / 3600000000ULL;
	uint64_t m = (uElapse % 3600000000ULL) / 60000000ULL;
	double s = (uElapse % 60000000ULL) / 1000000.0;
	if (uElapse < 60000000ULL) {
		snprintf(buf, bufSize, ZL10n::GetFmt(ZL10nKeys::TIME_FMT_S), s);
	} else if (uElapse < 3600000000ULL) {
		snprintf(buf, bufSize, ZL10n::GetFmt(ZL10nKeys::TIME_FMT_M_S), (unsigned long long)m, s);
	} else if (m == 0) {
		snprintf(buf, bufSize, ZL10n::GetFmt(ZL10nKeys::TIME_FMT_H_S), (unsigned long long)h, s);
	} else {
		snprintf(buf, bufSize, ZL10n::GetFmt(ZL10nKeys::TIME_FMT_H_M_S), (unsigned long long)h, (unsigned long long)m, s);
	}
}

uint64_t ZTimer::Print(const char* szFormatArgs, ...)
{
	FORMAT_V(szFormatArgs, szFormat);
	uint64_t uElapse = ZUtil::GetMicroSecond() - m_uBeginTime;
	char timeBuf[64];
	FormatElapsed(uElapse, timeBuf, sizeof(timeBuf));
	ZLog::PrintV(ZL10n::GetFmt(ZL10nKeys::TIME_FMT_WRAP), szFormat, timeBuf, (unsigned long long)uElapse);
	return Reset();
}

uint64_t ZTimer::PrintResult(bool bSuccess, const char* szFormatArgs, ...)
{
	FORMAT_V(szFormatArgs, szFormat);
	uint64_t uElapse = ZUtil::GetMicroSecond() - m_uBeginTime;
	char timeBuf[64];
	FormatElapsed(uElapse, timeBuf, sizeof(timeBuf));
	ZLog::PrintResultV(bSuccess, ZL10n::GetFmt(ZL10nKeys::TIME_FMT_WRAP), szFormat, timeBuf, (unsigned long long)uElapse);
	return Reset();
}
