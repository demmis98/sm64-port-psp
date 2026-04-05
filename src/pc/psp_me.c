#if defined(TARGET_PSP)

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <pspkernel.h>
#include <psputils.h>

#include <me-core-mapper/me-core.h>

#include "psp_me.h"
#include "psp_me_audio.h"

enum PspMeCommand {
    PSP_ME_COMMAND_NONE = 0,
    PSP_ME_COMMAND_GENERATE_AUDIO = 1,
    PSP_ME_COMMAND_SHUTDOWN = 2,
};

struct PspMeControl {
    volatile uint32_t ready;
    volatile uint32_t command;
};

static struct PspMeControl gPspMeControlRaw __attribute__((aligned(64), section(".uncached")));
static bool gPspMeInitialized;

static inline volatile struct PspMeControl *psp_me_control(void) {
    return (volatile struct PspMeControl *)(UNCACHED_USER_MASK | (uintptr_t)&gPspMeControlRaw);
}

static inline void psp_me_cpu_sync(void) {
    asm volatile("sync" ::: "memory");
}

static inline void psp_me_prepare_shared_state(void) {
    sceKernelDcacheWritebackInvalidateAll();
    psp_me_cpu_sync();
}

static inline void psp_me_finish_shared_state(void) {
    sceKernelDcacheWritebackInvalidateAll();
    psp_me_cpu_sync();
}

void meLibOnProcess(void) {
    volatile struct PspMeControl *control = psp_me_control();

    HW_SYS_BUS_CLOCK_ENABLE = (u32)-1;
    meLibSync();
    meCoreBusClockPreserve(0x0f);

    control->ready = 1;
    control->command = PSP_ME_COMMAND_NONE;
    meLibSync();

    for (;;) {
        uint32_t command = control->command;

        if (command == PSP_ME_COMMAND_NONE) {
            meLibDelayPipeline();
            continue;
        }

        if (command == PSP_ME_COMMAND_SHUTDOWN) {
            control->ready = 0;
            control->command = PSP_ME_COMMAND_NONE;
            meLibSync();
            meLibHalt();
        }

        if (command == PSP_ME_COMMAND_GENERATE_AUDIO) {
            meLibDcacheWritebackInvalidateAll();
            run_me_audio_cpu(0);
            meLibDcacheWritebackInvalidateAll();
        }

        control->command = PSP_ME_COMMAND_NONE;
        meLibSync();
    }
}

bool psp_me_init(void) {
    volatile struct PspMeControl *control = psp_me_control();
    int tableId;
    int wait;

    if (gPspMeInitialized) {
        return true;
    }

    memset((void *)&gPspMeControlRaw, 0, sizeof(gPspMeControlRaw));
    sceKernelDcacheWritebackInvalidateRange((void *)&gPspMeControlRaw, sizeof(gPspMeControlRaw));

    tableId = meLibDefaultInit();
    if (tableId < 0) {
        return false;
    }

    for (wait = 0; wait < 2000; wait++) {
        if (control->ready != 0) {
            gPspMeInitialized = true;
            return true;
        }
        sceKernelDelayThread(1000);
    }

    return false;
}

void psp_me_shutdown(void) {
    volatile struct PspMeControl *control = psp_me_control();
    int wait;

    if (!gPspMeInitialized) {
        return;
    }

    control->command = PSP_ME_COMMAND_SHUTDOWN;
    psp_me_cpu_sync();

    for (wait = 0; wait < 200; wait++) {
        if (control->ready == 0) {
            break;
        }
        sceKernelDelayThread(1000);
    }

    gPspMeInitialized = false;
}

bool psp_me_run_audio_job(void) {
    volatile struct PspMeControl *control = psp_me_control();

    if (!gPspMeInitialized || control->ready == 0) {
        return false;
    }

    psp_me_prepare_shared_state();
    control->command = PSP_ME_COMMAND_GENERATE_AUDIO;
    psp_me_cpu_sync();

    while (control->command != PSP_ME_COMMAND_NONE && control->ready != 0) {
        psp_me_cpu_sync();
    }

    psp_me_finish_shared_state();
    return control->ready != 0;
}

#endif
