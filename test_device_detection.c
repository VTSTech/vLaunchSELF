#include <stdio.h>
#include <string.h>

// Mock sysfs functions for testing
typedef struct {
    int st_mode;
} sysFSStat;

int mock_device_exists(const char *path) {
    // Mock some devices for testing
    if (strcmp(path, "/dev_hdd0") == 0) return 1;
    if (strcmp(path, "/dev_flash") == 0) return 1;
    return 0;
}

int device_exists(const char *path) {
    sysFSStat stat;
    return mock_device_exists(path);
}

static void find_first_available_device(void) {
    const char *devices[] = {
        "/dev_bdvd",
        "/dev_hdd0", 
        "/dev_hdd1",
        "/dev_flash",
        "/dev_rewrite"
    };
    
    const size_t device_count = sizeof(devices) / sizeof(devices[0]);
    printf("Testing %zu devices...\n", device_count);
    
    for (size_t i = 0; i < device_count; i++) {
        printf("Checking device %zu: %s\n", i, devices[i]);
        if (device_exists(devices[i])) {
            printf("✓ Found device: %s\n", devices[i]);
            return;
        }
    }
    
    /* If none found, default to /dev_hdd0 */
    printf("⚠ No devices found, using default: /dev_hdd0\n");
}

int main() {
    printf("=== Device Detection Test ===\n");
    find_first_available_device();
    return 0;
}