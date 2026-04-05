#pragma once

#include <stdbool.h>

#if defined(TARGET_PSP)
bool psp_me_init(void);
void psp_me_shutdown(void);
bool psp_me_run_audio_job(void);
#else
static inline bool psp_me_init(void) {
    return false;
}

static inline void psp_me_shutdown(void) {
}

static inline bool psp_me_run_audio_job(void) {
    return false;
}
#endif
