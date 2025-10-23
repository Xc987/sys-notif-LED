#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <switch.h>

// Size of the inner heap
#define INNER_HEAP_SIZE 0x80000
#define MAX_PADS 8

// Global variables
static HidsysUniquePadId connectedPads[MAX_PADS];
static int numConnectedPads = 0;
static HidsysNotificationLedPattern dimPattern;
static bool sysmoduleRunning = true;

#ifdef __cplusplus
extern "C" {
#endif

u32 __nx_applet_type = AppletType_None;
u32 __nx_fs_num_sessions = 1;

void __libnx_initheap(void) {
    static u8 inner_heap[INNER_HEAP_SIZE];
    extern void* fake_heap_start;
    extern void* fake_heap_end;

    fake_heap_start = inner_heap;
    fake_heap_end = inner_heap + sizeof(inner_heap);
}

void __appInit(void) {
    Result rc;

    rc = smInitialize();
    if (R_FAILED(rc))
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_InitFail_SM));

    rc = setsysInitialize();
    if (R_SUCCEEDED(rc)) {
        SetSysFirmwareVersion fw;
        rc = setsysGetFirmwareVersion(&fw);
        if (R_SUCCEEDED(rc))
            hosversionSet(MAKEHOSVERSION(fw.major, fw.minor, fw.micro));
        setsysExit();
    }

    rc = hidInitialize();
    if (R_FAILED(rc))
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_InitFail_HID));

    rc = hidsysInitialize();
    if (R_FAILED(rc))
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_InitFail_HID));

    rc = fsInitialize();
    if (R_FAILED(rc))
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_InitFail_FS));

    fsdevMountSdmc();
    smExit();
}

void __appExit(void) {
    sysmoduleRunning = false;
    fsdevUnmountAll();
    hidsysExit();
    hidExit();
    fsExit();
}

#ifdef __cplusplus
}
#endif

// Initialize the dim LED pattern
void initializeDimPattern() {
    memset(&dimPattern, 0, sizeof(dimPattern));
    dimPattern.baseMiniCycleDuration = 0x8;
    dimPattern.totalMiniCycles = 0x2;
    dimPattern.startIntensity = 0x2;
    dimPattern.miniCycles[0].ledIntensity = 0xF;
    dimPattern.miniCycles[0].transitionSteps = 0xF;
    dimPattern.miniCycles[1].ledIntensity = 0x2;
    dimPattern.miniCycles[1].transitionSteps = 0xF;
}

// Check if a controller is already in our connected list
bool isControllerConnected(HidsysUniquePadId* padId) {
    for (int i = 0; i < numConnectedPads; i++) {
        if (memcmp(&connectedPads[i], padId, sizeof(HidsysUniquePadId)) == 0) {
            return true;
        }
    }
    return false;
}

// Remove a controller from the connected list
void removeController(HidsysUniquePadId* padId) {
    for (int i = 0; i < numConnectedPads; i++) {
        if (memcmp(&connectedPads[i], padId, sizeof(HidsysUniquePadId)) == 0) {
            // Shift remaining controllers
            for (int j = i; j < numConnectedPads - 1; j++) {
                connectedPads[j] = connectedPads[j + 1];
            }
            numConnectedPads--;
            break;
        }
    }
}

// Set dim LED pattern on a controller
void setDimLed(HidsysUniquePadId* padId) {
    Result rc = hidsysSetNotificationLedPattern(&dimPattern, *padId);
    if (R_FAILED(rc)) {
        // If setting LED fails, remove the controller from our list
        removeController(padId);
    }
}

// Scan for newly connected controllers and dim their LEDs
void scanForNewControllers() {
    HidNpadIdType controllerTypes[MAX_PADS] = {
        HidNpadIdType_Handheld,
        HidNpadIdType_No1, HidNpadIdType_No2, HidNpadIdType_No3, HidNpadIdType_No4,
        HidNpadIdType_No5, HidNpadIdType_No6, HidNpadIdType_No7
    };

    for (int i = 0; i < MAX_PADS; i++) {
        HidsysUniquePadId padIds[MAX_PADS];
        s32 total_entries = 0;

        Result rc = hidsysGetUniquePadsFromNpad(controllerTypes[i], padIds, MAX_PADS, &total_entries);

        if (R_SUCCEEDED(rc) && total_entries > 0) {
            for (int j = 0; j < total_entries; j++) {
                if (!isControllerConnected(&padIds[j])) {
                    // New controller found - add to list and dim LED
                    if (numConnectedPads < MAX_PADS) {
                        connectedPads[numConnectedPads++] = padIds[j];
                        setDimLed(&padIds[j]);
                        
                        // Log the new connection (optional)
                        // You could write to a log file here
                    }
                }
            }
        }
    }
}

// Verify existing controllers are still connected
void verifyConnectedControllers() {
    for (int i = 0; i < numConnectedPads; i++) {
        // Try to set the LED pattern to verify the controller is still connected
        Result rc = hidsysSetNotificationLedPattern(&dimPattern, connectedPads[i]);
        if (R_FAILED(rc)) {
            // Controller is no longer responsive, remove it
            removeController(&connectedPads[i]);
            i--; // Adjust index after removal
        }
    }
}

// Main program entrypoint
int main(int argc, char* argv[]) {
    // Initialize the dim LED pattern
    initializeDimPattern();
    
    // Initial scan to get currently connected controllers
    scanForNewControllers();
    
    // Main service loop
    while (sysmoduleRunning) {
        // Scan for new controllers every second
        scanForNewControllers();
        
        // Verify existing controllers every 5 seconds
        static int verifyCounter = 0;
        if (verifyCounter++ >= 5) {
            verifyConnectedControllers();
            verifyCounter = 0;
        }
        
        // Sleep for 1 second
        svcSleepThread(500000000ULL); // 1 second in nanoseconds
    }
    
    return 0;
}