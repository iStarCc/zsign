#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ZLogExternalSink)(const char* utf8Line, int color, void* userdata);
void ZLog_SetExternalSink(ZLogExternalSink sink, void* userdata);
void ZLog_ClearExternalSink(void);

#ifdef __cplusplus
}
#endif
