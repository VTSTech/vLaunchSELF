/*
 * vLaunchSELF - ELF/SELF Launcher
 * 
 * A PS3 file browser with ELF/SELF launching capability.
 * Features:
 * - Text rendering using 8x8 bitmap font
 * - Directory navigation with full path display
 * - ELF/SELF auto-launching
 * - Debounced button controls
 * - Display scaling for proper resolution handling
 * 
 * Based on PS3-Moonlight graphics and text rendering system
 * Original: https://github.com/Cruslan/PS3-Moonlight
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmain"
#pragma GCC diagnostic ignored "-Wformat-truncation"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <malloc.h>

#include <sys/process.h>
#include <sysmodule/sysmodule.h>
#include <sysutil/sysutil.h>
#include <sysutil/video.h>
#include <io/pad.h>
#include <sys/thread.h>
#include <rsx/rsx.h>
#include <tiny3d.h>
#include <libfont.h>
#include <lv2/sysfs.h>

/* Process parameters: 1001 = user ID, 0x100000 = stack size */
SYS_PROCESS_PARAM(1001, 0x100000)

/* Display dimensions (from Moonlight ui.c) */
static int width = 1280;
static int height = 720;
static float scale_x = 1.0f;
static float scale_y = 1.0f;
static float scale_font = 1.0f;

/* Display scaling macros for resolution-independent positioning */
#define SX(x) ((float)(x) * scale_x)
#define SY(y) ((float)(y) * scale_y)
#define SF(s) ((u32)(((float)(s) * scale_font < 8.0f) ? 8.0f : ((float)(s) * scale_font)))

/* Maximum items for file listing */
#define MAX_FILES   256
#define MAX_NAME    256
#define MAX_PATH    1024

/* Debouncing: number of frames to wait before accepting another input */
#define INPUT_DELAY_FRAMES 10

/* File entry structure for directory listing */
typedef struct { 
    char name[MAX_NAME]; 
    int is_dir; 
    u64 size; 
    u64 mtime; 
} file_entry_t;

/* Global state variables */
static file_entry_t files[MAX_FILES];
static int file_count = 0;
static int selected_index = 0;
static char current_path[MAX_PATH] = "/dev_hdd0";
static volatile int running = 1;

/* Font texture memory (from Moonlight ui.c) */
static void * texture_mem = NULL;

/* Input debouncing state - all buttons use frame-based delay */
static int up_counter = 0;
static int down_counter = 0;
static int cross_counter = 0;
static int circle_counter = 0;
static int triangle_counter = 0;

/**
 * 8x8 bitmap font glyph data
 * Contains ASCII characters 32-127 (printable characters)
 * Each character is 8 pixels wide and 8 pixels tall
 */
static const unsigned char font_8x8_basic[96][8] = {
    /* ASCII 32 (space) */ {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    /* ASCII 33 (!) */ {0x18,0x3c,0x3c,0x18,0x18,0x00,0x18,0x00},
    /* ASCII 34 (") */ {0x6c,0x6c,0x6c,0x00,0x00,0x00,0x00,0x00},
    /* ASCII 35 (#) */ {0x6c,0x6c,0xfe,0x6c,0xfe,0x6c,0x6c,0x00},
    /* ASCII 36 ($) */ {0x18,0x3e,0x60,0x3c,0x06,0x7c,0x18,0x00},
    /* ASCII 37 (%) */ {0x00,0xc6,0xcc,0x18,0x30,0x66,0xc6,0x00},
    /* ASCII 38 (&) */ {0x38,0x6c,0x38,0x76,0xdc,0xcc,0x76,0x00},
    /* ASCII 39 (') */ {0x18,0x18,0x30,0x00,0x00,0x00,0x00,0x00},
    /* ASCII 40 (() */ {0x0c,0x18,0x30,0x30,0x30,0x18,0x0c,0x00},
    /* ASCII 41 ()) */ {0x30,0x18,0x0c,0x0c,0x0c,0x18,0x30,0x00},
    /* ASCII 42 (*) */ {0x00,0x66,0x3c,0xff,0x3c,0x66,0x00,0x00},
    /* ASCII 43 (+) */ {0x00,0x18,0x18,0x7e,0x18,0x18,0x00,0x00},
    /* ASCII 44 (,) */ {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00},
    /* ASCII 45 (-) */ {0x00,0x00,0x00,0xfe,0x00,0x00,0x00,0x00},
    /* ASCII 46 (.) */ {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00},
    /* ASCII 47 (/) */ {0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x00},
    /* ASCII 48 (0) */ {0x3c,0x66,0x6e,0x7e,0x76,0x66,0x3c,0x00},
    /* ASCII 49 (1) */ {0x18,0x38,0x18,0x18,0x18,0x18,0x3c,0x00},
    /* ASCII 50 (2) */ {0x3c,0x66,0x06,0x0c,0x18,0x30,0x7e,0x00},
    /* ASCII 51 (3) */ {0x3c,0x66,0x06,0x1c,0x06,0x66,0x3c,0x00},
    /* ASCII 52 (4) */ {0x0c,0x1c,0x3c,0x6c,0xfe,0x0c,0x0c,0x00},
    /* ASCII 53 (5) */ {0x7e,0x60,0x7c,0x06,0x06,0x66,0x3c,0x00},
    /* ASCII 54 (6) */ {0x1c,0x30,0x60,0x7c,0x66,0x66,0x3c,0x00},
    /* ASCII 55 (7) */ {0x7e,0x06,0x0c,0x18,0x30,0x30,0x30,0x00},
    /* ASCII 56 (8) */ {0x3c,0x66,0x66,0x3c,0x66,0x66,0x3c,0x00},
    /* ASCII 57 (9) */ {0x3c,0x66,0x66,0x3e,0x06,0x0c,0x38,0x00},
    /* ASCII 58 (:) */ {0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x00},
    /* ASCII 59 (;) */ {0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x30},
    /* ASCII 60 (<) */ {0x0c,0x18,0x30,0x60,0x30,0x18,0x0c,0x00},
    /* ASCII 61 (=) */ {0x00,0x00,0x7e,0x00,0x7e,0x00,0x00,0x00},
    /* ASCII 62 (>) */ {0x30,0x18,0x0c,0x06,0x0c,0x18,0x30,0x00},
    /* ASCII 63 (?) */ {0x3c,0x66,0x06,0x0c,0x18,0x00,0x18,0x00},
    /* ASCII 64 (@) */ {0x3c,0x66,0x6e,0x6e,0x60,0x3e,0x00,0x00},
    /* ASCII 65 (A) */ {0x18,0x3c,0x66,0x66,0x7e,0x66,0x66,0x00},
    /* ASCII 66 (B) */ {0x7c,0x66,0x66,0x7c,0x66,0x66,0x7c,0x00},
    /* ASCII 67 (C) */ {0x3c,0x66,0x60,0x60,0x60,0x66,0x3c,0x00},
    /* ASCII 68 (D) */ {0x78,0x6c,0x66,0x66,0x66,0x6c,0x78,0x00},
    /* ASCII 69 (E) */ {0x7e,0x60,0x60,0x7c,0x60,0x60,0x7e,0x00},
    /* ASCII 70 (F) */ {0x7e,0x60,0x60,0x7c,0x60,0x60,0x60,0x00},
    /* ASCII 71 (G) */ {0x3c,0x66,0x60,0x6e,0x66,0x66,0x3c,0x00},
    /* ASCII 72 (H) */ {0x66,0x66,0x66,0x7e,0x66,0x66,0x66,0x00},
    /* ASCII 73 (I) */ {0x3c,0x18,0x18,0x18,0x18,0x18,0x3c,0x00},
    /* ASCII 74 (J) */ {0x1e,0x0c,0x0c,0x0c,0x0c,0x6c,0x38,0x00},
    /* ASCII 75 (K) */ {0x66,0x6c,0x78,0x70,0x78,0x6c,0x66,0x00},
    /* ASCII 76 (L) */ {0x60,0x60,0x60,0x60,0x60,0x60,0x7e,0x00},
    /* ASCII 77 (M) */ {0x63,0x77,0x7f,0x6b,0x63,0x63,0x63,0x00},
    /* ASCII 78 (N) */ {0x66,0x66,0x76,0x7e,0x6e,0x66,0x66,0x00},
    /* ASCII 79 (O) */ {0x3c,0x66,0x66,0x66,0x66,0x66,0x3c,0x00},
    /* ASCII 80 (P) */ {0x7c,0x66,0x66,0x7c,0x60,0x60,0x60,0x00},
    /* ASCII 81 (Q) */ {0x3c,0x66,0x66,0x76,0x6e,0x66,0x3e,0x00},
    /* ASCII 82 (R) */ {0x7c,0x66,0x66,0x7c,0x78,0x6c,0x66,0x00},
    /* ASCII 83 (S) */ {0x3c,0x66,0x30,0x18,0x0c,0x66,0x3c,0x00},
    /* ASCII 84 (T) */ {0x7e,0x18,0x18,0x18,0x18,0x18,0x18,0x00},
    /* ASCII 85 (U) */ {0x66,0x66,0x66,0x66,0x66,0x66,0x3c,0x00},
    /* ASCII 86 (V) */ {0x66,0x66,0x66,0x66,0x66,0x3c,0x18,0x00},
    /* ASCII 87 (W) */ {0x63,0x6b,0x7f,0x7f,0x36,0x00,0x00,0x00},
    /* ASCII 88 (X) */ {0x66,0x66,0x3c,0x18,0x3c,0x66,0x66,0x00},
    /* ASCII 89 (Y) */ {0x66,0x66,0x66,0x3c,0x18,0x18,0x18,0x00},
    /* ASCII 90 (Z) */ {0x7e,0x06,0x0c,0x18,0x30,0x60,0x7e,0x00},
    /* ASCII 91 ([) */ {0x3c,0x30,0x30,0x30,0x30,0x30,0x3c,0x00},
    /* ASCII 92 (\) */ {0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x00},
    /* ASCII 93 (]) */ {0x3c,0x0c,0x0c,0x0c,0x0c,0x0c,0x3c,0x00},
    /* ASCII 94 (^) */ {0x10,0x38,0x6c,0xc6,0x00,0x00,0x00,0x00},
    /* ASCII 95 (_) */ {0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x00},
    /* ASCII 96 (`) */ {0x30,0x18,0x0c,0x00,0x00,0x00,0x00,0x00},
    /* ASCII 97 (a) */ {0x00,0x00,0x3c,0x06,0x3e,0x66,0x3e,0x00},
    /* ASCII 98 (b) */ {0x60,0x60,0x7c,0x66,0x66,0x66,0x7c,0x00},
    /* ASCII 99 (c) */ {0x00,0x00,0x3c,0x60,0x60,0x66,0x3c,0x00},
    /* ASCII 100 (d) */ {0x06,0x06,0x3e,0x66,0x66,0x66,0x3e,0x00},
    /* ASCII 101 (e) */ {0x00,0x00,0x3c,0x66,0x7e,0x60,0x3c,0x00},
    /* ASCII 102 (f) */ {0x1c,0x30,0x7c,0x30,0x30,0x30,0x30,0x00},
    /* ASCII 103 (g) */ {0x00,0x00,0x3e,0x66,0x66,0x3e,0x06,0x3c},
    /* ASCII 104 (h) */ {0x60,0x60,0x7c,0x66,0x66,0x66,0x66,0x00},
    /* ASCII 105 (i) */ {0x18,0x00,0x38,0x18,0x18,0x18,0x3c,0x00},
    /* ASCII 106 (j) */ {0x06,0x00,0x1e,0x06,0x06,0x66,0x3c,0x00},
    /* ASCII 107 (k) */ {0x60,0x60,0x66,0x6c,0x78,0x6c,0x66,0x00},
    /* ASCII 108 (l) */ {0x30,0x30,0x30,0x30,0x30,0x30,0x1c,0x00},
    /* ASCII 109 (m) */ {0x00,0x00,0x66,0x7f,0x7f,0x6b,0x63,0x00},
    /* ASCII 110 (n) */ {0x00,0x00,0x7c,0x66,0x66,0x66,0x66,0x00},
    /* ASCII 111 (o) */ {0x00,0x00,0x3c,0x66,0x66,0x66,0x3c,0x00},
    /* ASCII 112 (p) */ {0x00,0x00,0x7c,0x66,0x66,0x7c,0x60,0x60},
    /* ASCII 113 (q) */ {0x00,0x00,0x3e,0x66,0x66,0x3e,0x06,0x06},
    /* ASCII 114 (r) */ {0x00,0x00,0x7c,0x66,0x60,0x60,0x60,0x00},
    /* ASCII 115 (s) */ {0x00,0x00,0x3e,0x60,0x3c,0x06,0x7c,0x00},
    /* ASCII 116 (t) */ {0x30,0x30,0x7c,0x30,0x30,0x30,0x1c,0x00},
    /* ASCII 117 (u) */ {0x00,0x00,0x66,0x66,0x66,0x66,0x3e,0x00},
    /* ASCII 118 (v) */ {0x00,0x00,0x66,0x66,0x66,0x3c,0x18,0x00},
    /* ASCII 119 (w) */ {0x00,0x00,0x63,0x6b,0x7f,0x7f,0x36,0x00},
    /* ASCII 120 (x) */ {0x00,0x00,0x66,0x3c,0x18,0x3c,0x66,0x00},
    /* ASCII 121 (y) */ {0x00,0x00,0x66,0x66,0x66,0x3e,0x06,0x3c},
    /* ASCII 122 (z) */ {0x00,0x00,0x7e,0x0c,0x18,0x30,0x7e,0x00},
    /* ASCII 123 ([) */ {0x0c,0x18,0x18,0x70,0x18,0x18,0x0c,0x00},
    /* ASCII 124 (|) */ {0x18,0x18,0x18,0x00,0x18,0x18,0x18,0x00},
    /* ASCII 125 (}) */ {0x30,0x18,0x0c,0x00,0x00,0x00,0x00,0x00},
    /* ASCII 126 (~) */ {0x00,0x00,0x4c,0xb2,0x00,0x00,0x00,0x00}
};

/**
 * SYS_UTIL callback handler for game exit events
 * @param status Exit status code
 * @param param Exit parameter
 * @param usrdata User data pointer (unused)
 */
static void sysutil_exit_callback(u64 status, u64 param, void *usrdata) {
    (void)param; (void)usrdata;
    if (status == SYSUTIL_EXIT_GAME) running = 0;
}

/**
 * Draw the background gradient for the UI
 * Creates a dark gradient background with a blue title bar
 */
static void draw_background_gradient(void) {
    /* 1. Base Dark Gray Background (#303030 to #242424) */
    tiny3d_SetPolygon(TINY3D_TRIANGLE_STRIP);
    tiny3d_VertexPos(0, 0, 65535);
    tiny3d_VertexFcolor(0.188f, 0.188f, 0.188f, 1.0f); // #303030
    tiny3d_VertexPos(width, 0, 65535);
    tiny3d_VertexFcolor(0.188f, 0.188f, 0.188f, 1.0f);
    tiny3d_VertexPos(0, height * 0.72f, 65535);
    tiny3d_VertexFcolor(0.141f, 0.141f, 0.141f, 1.0f); // #242424
    tiny3d_VertexPos(width, height * 0.72f, 65535);
    tiny3d_VertexFcolor(0.141f, 0.141f, 0.141f, 1.0f);
    tiny3d_End();

    /* 2. Titlebar Header Bar (#3F51B5 Material Indigo Blue) */
    tiny3d_SetPolygon(TINY3D_TRIANGLE_STRIP);
    tiny3d_VertexPos(0, 0, 65535);
    tiny3d_VertexFcolor(0.247f, 0.318f, 0.710f, 1.0f); // #3F51B5
    tiny3d_VertexPos(width, 0, 65535);
    tiny3d_VertexFcolor(0.247f, 0.318f, 0.710f, 1.0f);
    tiny3d_VertexPos(0, SY(64), 65535);
    tiny3d_VertexFcolor(0.200f, 0.260f, 0.620f, 1.0f);
    tiny3d_VertexPos(width, SY(64), 65535);
    tiny3d_VertexFcolor(0.200f, 0.260f, 0.620f, 1.0f);
    tiny3d_End();

    /* 3. Titlebar Bottom Accent Line (#1A237E Deep Indigo) */
    tiny3d_SetPolygon(TINY3D_TRIANGLE_STRIP);
    tiny3d_VertexPos(0, SY(64), 65535);
    tiny3d_VertexFcolor(0.102f, 0.137f, 0.494f, 1.0f); // #1A237E
    tiny3d_VertexPos(width, SY(64), 65535);
    tiny3d_VertexFcolor(0.102f, 0.137f, 0.494f, 1.0f);
    tiny3d_VertexPos(0, SY(67), 65535);
    tiny3d_VertexFcolor(0.102f, 0.137f, 0.494f, 1.0f);
    tiny3d_VertexPos(width, SY(67), 65535);
    tiny3d_VertexFcolor(0.102f, 0.137f, 0.494f, 1.0f);
    tiny3d_End();
}

/**
 * List files in the current directory
 * Populates the global files[] array with directory entries
 */
static int device_exists(const char *path) {
    sysFSStat stat;
    int result = sysFsStat(path, &stat) == 0;
    printf("Device check: %s = %s\n", path, result ? "FOUND" : "NOT FOUND");
    return result;
}

static int available_devices_count = 0;
static char available_device_paths[5][MAX_PATH];

static void scan_available_devices(void) {
    const char *devices[] = {
        "/dev_bdvd",
        "/dev_hdd0", 
        "/dev_hdd1",
        "/dev_flash",
        "/dev_rewrite"
    };
    
    const size_t device_count = sizeof(devices) / sizeof(devices[0]);
    available_devices_count = 0;
    
    printf("Scanning for available devices...\n");
    
    /* Find all available devices */
    for (size_t i = 0; i < device_count; i++) {
        if (device_exists(devices[i])) {
            strncpy(available_device_paths[available_devices_count], devices[i], MAX_PATH - 1);
            available_device_paths[available_devices_count][MAX_PATH - 1] = '\0';
            available_devices_count++;
            printf("✓ Available: %s\n", devices[i]);
        } else {
            printf("✗ Not available: %s\n", devices[i]);
        }
    }
    
    if (available_devices_count == 0) {
        printf("No devices found, using default: /dev_hdd0\n");
        strncpy(current_path, "/dev_hdd0", MAX_PATH - 1);
        current_path[MAX_PATH - 1] = '\0';
    }
}

static void format_file_size(u64 size, char* buffer, size_t buffer_size) {
    if (size < 1024) {
        snprintf(buffer, buffer_size, "%lu B", size);
    } else if (size < 1024 * 1024) {
        snprintf(buffer, buffer_size, "%.1f KB", (double)size / 1024.0);
    } else if (size < 1024 * 1024 * 1024) {
        snprintf(buffer, buffer_size, "%.1f MB", (double)size / (1024.0 * 1024.0));
    } else {
        snprintf(buffer, buffer_size, "%.1f GB", (double)size / (1024.0 * 1024.0 * 1024.0));
    }
}

static void format_timestamp(u64 mtime, char* buffer, size_t buffer_size) {
    // PS3 uses Unix timestamps in seconds
    time_t timestamp = (time_t)mtime;
    struct tm* tm_info;
    char temp_buffer[64];
    
    if (timestamp == 0) {
        snprintf(buffer, buffer_size, "System file");
        return;
    }
    
    tm_info = gmtime(&timestamp);
    if (tm_info) {
        strftime(temp_buffer, sizeof(temp_buffer), "%m-%d-%Y %H:%M", tm_info);
        snprintf(buffer, buffer_size, "%s", temp_buffer);
    } else {
        snprintf(buffer, buffer_size, "Invalid date");
    }
}

static void list_directory(void) {
    printf("LIST: %s\n", current_path);
    file_count = 0;
    s32 fd;
    if (sysFsOpendir(current_path, &fd) != 0) {
        printf("ERROR: Can't open %s\n", current_path);
        return;
    }
    
    sysFSDirent entry;
    u64 bytes_read;
    while (sysFsReaddir(fd, &entry, &bytes_read) == 0 && bytes_read > 0 && file_count < MAX_FILES - 1) {
        if (strcmp(entry.d_name, ".") == 0 || strcmp(entry.d_name, "..") == 0) continue;
        
        strncpy(files[file_count].name, entry.d_name, MAX_NAME - 1);
        files[file_count].name[MAX_NAME - 1] = '\0';
        
        char full_path[MAX_PATH];
        snprintf(full_path, sizeof(full_path), "%s/%s", current_path, entry.d_name);
        
        sysFSStat stat;
        files[file_count].is_dir = 0;
        files[file_count].size = 0;
        files[file_count].mtime = 0;
        
        if (sysFsStat(full_path, &stat) == 0) {
            files[file_count].is_dir = ((stat.st_mode >> 12) & 017) == 04;
            files[file_count].size = stat.st_size;
            files[file_count].mtime = stat.st_mtime;
        }
        
        file_count++;
        printf("  [%d] %s (dir=%d, size=%lu, time=%lu)\n", 
               file_count-1, files[file_count-1].name, files[file_count-1].is_dir,
               files[file_count-1].size, files[file_count-1].mtime);
    }
    sysFsClosedir(fd);
}

/**
 * Initialize the font system
 * Allocates texture memory and registers the 8x8 bitmap font
 */
static void ui_init_fonts() {
    ResetFont();
    
    /* Allocate texture memory */
    texture_mem = tiny3d_AllocTexture(1024 * 1024);
    if (texture_mem) {
        AddFontFromBitmapArray((u8 *)font_8x8_basic, (u8 *)texture_mem, 32, 127, 8, 8, 1, BIT7_FIRST_PIXEL);
    }
    
    SetCurrentFont(0);
    SetFontSize(SF(16), SF(16));
    SetFontColor(0xffffffff, 0);
}

/**
 * Draw the current directory path at the specified Y position
 * @param path Directory path string to display
 * @param y Vertical position
 */
static void draw_text_with_path(const char *path, int y) {
    SetFontSize(SF(14), SF(14));  /* Slightly larger for visibility */
    SetFontColor(0xffffa500, 0); /* Orange for path */
    DrawString(SX(20), SY(y), (char*)path);
}


/**
 * Launch an ELF/SELF file
 * @param filepath Full path to the executable file
 * 
 * Uses sysProcessExitSpawn2 to terminate current process and launch the ELF/SELF
 */
static void launch_elf_file(const char *filepath) {
    const char *ext = strrchr(filepath, '.');
    if (!ext) {
        printf("Not an executable file\n");
        return;
    }
    
    int is_elf = 0;
    int is_self = 0;
    
    /* Check for ELF/SELF only */
    if (strcasecmp(ext, ".elf") == 0) is_elf = 1;
    if (strcasecmp(ext, ".self") == 0) is_self = 1;
    
    if (!is_elf && !is_self) {
        printf("Not an ELF/SELF: %s\n", ext);
        return;
    }
    
    printf("Launching executable: %s\n", filepath);
    printf("File extension: %s\n", ext);
    printf("Is ELF: %d, Is SELF: %d\n", is_elf, is_self);
    
    /* Use sysProcessExitSpawn2 to terminate current process and launch the new one */
    sysProcessExitSpawn2(filepath, NULL, NULL, NULL, 0, 0x100000, 
        SYS_PROCESS_SPAWN_STACK_SIZE_1M);
    
    printf("Launched executable - this message should not be seen\n");
    
    /* This point will not be reached */
}

/**
 * Main entry point
 * Initializes video, font, and input systems
 * Runs the main event loop handling controller input
 */
int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    
    videoState vstate;
    videoResolution vres;
    padInfo padinfo;
    padData paddata;
    
    printf("=== vLaunchSELF Starting ===\n");
    
    /* Get display resolution */
    videoGetState(VIDEO_PRIMARY, 0, &vstate);
    videoGetResolution(vstate.displayMode.resolution, &vres);
    
    width = (vres.width > 0) ? vres.width : 1280;
    height = (vres.height > 0) ? vres.height : 720;
    
    printf("Display: %dx%d\n", width, height);
    
    /* Initialize scaling factors for resolution-independent positioning */
    scale_x = (float)width / 1280.0f;
    scale_y = (float)height / 720.0f;
    scale_font = (scale_x < scale_y) ? scale_x : scale_y;
    
    /* Initialize controller */
    ioPadInit(7);
    
    /* Initialize SysModule for font system */
    sysModuleLoad(SYSMODULE_FONT);
    
    /* Initialize tiny3D */
    tiny3d_Init(1024 * 1024);  /* 1MB vertex buffer */
    tiny3d_Project2D();         /* Switch to 2D mode for font rendering */
    
    /* Initialize fonts */
    ui_init_fonts();
    
    /* Enable Alpha Test and Blending */
    tiny3d_AlphaTest(1, 0, TINY3D_ALPHA_FUNC_GREATER);
    tiny3d_BlendFunc(1, 
        (blend_src_func)(TINY3D_BLEND_FUNC_SRC_RGB_SRC_ALPHA | TINY3D_BLEND_FUNC_SRC_ALPHA_SRC_ALPHA),
        (blend_dst_func)(TINY3D_BLEND_FUNC_DST_RGB_ONE_MINUS_SRC_ALPHA | TINY3D_BLEND_FUNC_DST_ALPHA_ONE_MINUS_SRC_ALPHA),
        (blend_func)(TINY3D_BLEND_RGB_FUNC_ADD | TINY3D_BLEND_ALPHA_FUNC_ADD));
    
    /* Set VSync mode */
    gcmSetFlipMode(GCM_FLIP_VSYNC);
    
    /* Set up callback */
    sysUtilRegisterCallback(0, sysutil_exit_callback, NULL);
    
    printf("=== vLaunchSELF v1 Starting ===\n");
    
    /* Scan for available devices */
    scan_available_devices();
    
    if (available_devices_count == 0) {
        /* No devices found, use default */
        list_directory();
    } else if (available_devices_count == 1) {
        /* Only one device, use it directly */
        printf("Only one device available: %s\n", current_path);
        list_directory();
    } else {
        /* Multiple devices, show selection menu */
        printf("Multiple devices available. Select one:\n");
        for (int i = 0; i < available_devices_count; i++) {
            printf("  [%d] %s\n", i, available_device_paths[i]);
        }
        printf("Use UP/DOWN to select, CROSS to choose\n");
        
        /* Start with first device selected */
        selected_index = 0;
        strncpy(current_path, available_device_paths[0], MAX_PATH - 1);
        current_path[MAX_PATH - 1] = '\0';
        
        /* Set device selection mode */
        int device_selection_mode = 1;
        
        while (device_selection_mode) {
            sysUtilCheckCallback();
            ioPadGetInfo(&padinfo);
            
            if (padinfo.status[0]) {
                ioPadGetData(0, &paddata);
                
                /* Handle all buttons with debouncing */
                /* CIRCLE: Exit device selection */
                if (paddata.BTN_CIRCLE) {
                    circle_counter--;
                    if (circle_counter <= 0) {
                        printf("CIRCLE pressed - exit\n");
                        running = 0;
                        device_selection_mode = 0;
                        continue;
                    }
                    circle_counter = INPUT_DELAY_FRAMES;
                } else {
                    circle_counter = 0;
                }
                
                /* Handle UP with debouncing */
                if (paddata.BTN_UP) {
                    up_counter--;
                    if (up_counter <= 0) {
                        selected_index = (selected_index + available_devices_count - 1) % available_devices_count;
                        strncpy(current_path, available_device_paths[selected_index], MAX_PATH - 1);
                        current_path[MAX_PATH - 1] = '\0';
                        printf("Selected: %s\n", current_path);
                        up_counter = INPUT_DELAY_FRAMES;
                    }
                } else {
                    up_counter = 0;
                }
                
                /* Handle DOWN with debouncing */
                if (paddata.BTN_DOWN) {
                    down_counter--;
                    if (down_counter <= 0) {
                        selected_index = (selected_index + 1) % available_devices_count;
                        strncpy(current_path, available_device_paths[selected_index], MAX_PATH - 1);
                        current_path[MAX_PATH - 1] = '\0';
                        printf("Selected: %s\n", current_path);
                        down_counter = INPUT_DELAY_FRAMES;
                    }
                } else {
                    down_counter = 0;
                }
                
                /* Handle CROSS with debouncing */
                if (paddata.BTN_CROSS) {
                    cross_counter--;
                    if (cross_counter <= 0) {
                        printf("Selected device %d: %s\n", selected_index, current_path);
                        device_selection_mode = 0;
                        cross_counter = INPUT_DELAY_FRAMES;
                    }
                } else {
                    cross_counter = 0;
                }
            }
            
            /* Draw device selection screen */
            tiny3d_Clear(0x303030ff, TINY3D_CLEAR_ALL);
            draw_background_gradient();
            
            /* Draw title */
            SetFontSize(SF(22), SF(22));
            SetFontColor(0xffffffff, 0);
            DrawString(SX(35), SY(10), (char*)"Select Device");
            
            /* Draw device list */
            SetFontSize(SF(16), SF(16));
            for (int i = 0; i < available_devices_count; i++) {
                if (i == selected_index) {
                    SetFontColor(0xff82b1ff, 0);  /* Light Material Cyan/Blue for selected */
                } else {
                    SetFontColor(0xffffffff, 0);
                }
                char line[280];
                snprintf(line, sizeof(line), "  [%d] %s", i, available_device_paths[i]);
                DrawString(SX(20), SY(60 + i * SF(30)), line);
            }
            
            /* Draw controls */
            SetFontSize(SF(14), SF(14));
            SetFontColor(0xffffffff, 0);
            DrawString(SX(35), SY(350), (char*)"[X] Select  [O] Exit  [Up/Down] Navigate");
            
            tiny3d_Flip();
        }
        
        /* Device selected, now list its contents */
        list_directory();
    }
    
    printf("Total files: %d\n", file_count);
    printf("Current path: %s\n", current_path);
    
    /* Main loop */
    while (running) {
        sysUtilCheckCallback();
        ioPadGetInfo(&padinfo);
        
        if (padinfo.status[0]) {
            ioPadGetData(0, &paddata);
            
            /* Handle all buttons with debouncing */
            /* CIRCLE: Exit */
            if (paddata.BTN_CIRCLE) {
                circle_counter--;
                if (circle_counter <= 0) {
                    printf("CIRCLE pressed - exit\n");
                    running = 0;
                    continue;
                }
                circle_counter = INPUT_DELAY_FRAMES;
            } else {
                circle_counter = 0;
            }
            
            /* TRIANGLE: Go up one directory or show device selection at root */
            if (paddata.BTN_TRIANGLE) {
                triangle_counter--;
                if (triangle_counter <= 0) {
                    char *last_slash = strrchr(current_path, '/');
                    
                    /* If we're at the root of a device, show device selection */
                    if (last_slash == current_path || strlen(current_path) <= 5) {
                        printf("At device root, showing device selection...\n");
                        scan_available_devices();
                        
                        if (available_devices_count > 1) {
                            /* Show device selection menu */
                            printf("Multiple devices available. Select one:\n");
                            for (int i = 0; i < available_devices_count; i++) {
                                printf("  [%d] %s\n", i, available_device_paths[i]);
                            }
                            printf("Use UP/DOWN to select, CROSS to choose\n");
                            
                            /* Start with current device selected */
                            selected_index = 0;
                            for (int i = 0; i < available_devices_count; i++) {
                                if (strcmp(available_device_paths[i], current_path) == 0) {
                                    selected_index = i;
                                    break;
                                }
                            }
                            strncpy(current_path, available_device_paths[selected_index], MAX_PATH - 1);
                            current_path[MAX_PATH - 1] = '\0';
                            
                            /* Set device selection mode */
                            int device_selection_mode = 1;
                            
                            while (device_selection_mode) {
                                sysUtilCheckCallback();
                                ioPadGetInfo(&padinfo);
                                
                                if (padinfo.status[0]) {
                                    ioPadGetData(0, &paddata);
                                    
                                    /* Handle all buttons with debouncing */
                                    /* CIRCLE: Exit device selection */
                                    if (paddata.BTN_CIRCLE) {
                                        circle_counter--;
                                        if (circle_counter <= 0) {
                                            printf("CIRCLE pressed - exit\n");
                                            running = 0;
                                            device_selection_mode = 0;
                                            continue;
                                        }
                                        circle_counter = INPUT_DELAY_FRAMES;
                                    } else {
                                        circle_counter = 0;
                                    }
                                    
                                    /* Handle UP with debouncing */
                                    if (paddata.BTN_UP) {
                                        up_counter--;
                                        if (up_counter <= 0) {
                                            selected_index = (selected_index + available_devices_count - 1) % available_devices_count;
                                            strncpy(current_path, available_device_paths[selected_index], MAX_PATH - 1);
                                            current_path[MAX_PATH - 1] = '\0';
                                            printf("Selected: %s\n", current_path);
                                            up_counter = INPUT_DELAY_FRAMES;
                                        }
                                    } else {
                                        up_counter = 0;
                                    }
                                    
                                    /* Handle DOWN with debouncing */
                                    if (paddata.BTN_DOWN) {
                                        down_counter--;
                                        if (down_counter <= 0) {
                                            selected_index = (selected_index + 1) % available_devices_count;
                                            strncpy(current_path, available_device_paths[selected_index], MAX_PATH - 1);
                                            current_path[MAX_PATH - 1] = '\0';
                                            printf("Selected: %s\n", current_path);
                                            down_counter = INPUT_DELAY_FRAMES;
                                        }
                                    } else {
                                        down_counter = 0;
                                    }
                                    
                                    /* Handle CROSS with debouncing */
                                    if (paddata.BTN_CROSS) {
                                        cross_counter--;
                                        if (cross_counter <= 0) {
                                            printf("Selected device %d: %s\n", selected_index, current_path);
                                            device_selection_mode = 0;
                                            cross_counter = INPUT_DELAY_FRAMES;
                                        }
                                    } else {
                                        cross_counter = 0;
                                    }
                                }
                                
                                /* Draw device selection screen */
                                tiny3d_Clear(0x303030ff, TINY3D_CLEAR_ALL);
                                draw_background_gradient();
                                
                                /* Draw title */
                                SetFontSize(SF(22), SF(22));
                                SetFontColor(0xffffffff, 0);
                                DrawString(SX(35), SY(10), (char*)"Select Device");
                                
                                /* Draw device list */
                                SetFontSize(SF(16), SF(16));
                                for (int i = 0; i < available_devices_count; i++) {
                                    if (i == selected_index) {
                                        SetFontColor(0xff82b1ff, 0);  /* Light Material Cyan/Blue for selected */
                                    } else {
                                        SetFontColor(0xffffffff, 0);
                                    }
                                    char line[280];
                                    snprintf(line, sizeof(line), "  [%d] %s", i, available_device_paths[i]);
                                    DrawString(SX(20), SY(60 + i * SF(30)), line);
                                }
                                
                                /* Draw controls */
                                SetFontSize(SF(14), SF(14));
                                SetFontColor(0xffffffff, 0);
                                DrawString(SX(35), SY(350), (char*)"[X] Select  [O] Exit  [↑↓] Navigate");
                                
                                tiny3d_Flip();
                            }
                            
                            /* Device selected, now list its contents */
                            list_directory();
                        } else {
                            /* Only one device, just list it */
                            list_directory();
                        }
                    } else {
                        /* Go up one directory */
                        *last_slash = '\0';  /* Remove last component */
                        if (strlen(current_path) == 0) {
                            strcpy(current_path, "/dev_hdd0");
                        }
                        selected_index = 0;
                        printf("UP DIR: %s\n", current_path);
                        list_directory();
                    }
                    triangle_counter = INPUT_DELAY_FRAMES;
                }
            } else {
                triangle_counter = 0;
            }
            
            /* Handle UP with debouncing */
            if (paddata.BTN_UP) {
                up_counter--;
                if (up_counter <= 0 && file_count > 0) {
                    selected_index = (selected_index + file_count - 1) % file_count;
                    printf("UP: %d\n", selected_index);
                    up_counter = INPUT_DELAY_FRAMES;
                }
            } else {
                up_counter = 0;
            }
            
            /* Handle DOWN with debouncing */
            if (paddata.BTN_DOWN) {
                down_counter--;
                if (down_counter <= 0 && file_count > 0) {
                    selected_index = (selected_index + 1) % file_count;
                    printf("DOWN: %d\n", selected_index);
                    down_counter = INPUT_DELAY_FRAMES;
                }
            } else {
                down_counter = 0;
            }
            
            /* Handle CROSS with debouncing */
            if (paddata.BTN_CROSS) {
                cross_counter--;
                if (cross_counter <= 0 && file_count > 0 && selected_index < file_count) {
                    if (files[selected_index].is_dir) {
                        printf("ENTER: %s\n", files[selected_index].name);
                        char new_path[MAX_PATH];
                        snprintf(new_path, sizeof(new_path), "%s/%s", current_path, files[selected_index].name);
                        new_path[MAX_PATH - 1] = '\0';
                        strncpy(current_path, new_path, MAX_PATH - 1);
                        current_path[MAX_PATH - 1] = '\0';
                        selected_index = 0;
                        list_directory();
                    } else {
                        /* Try to launch ELF/SELF file */
                        char full_path[MAX_PATH];
                        snprintf(full_path, sizeof(full_path), "%s/%s", current_path, files[selected_index].name);
                        full_path[MAX_PATH - 1] = '\0';
                        
                        launch_elf_file(full_path);
                    }
                    cross_counter = INPUT_DELAY_FRAMES;
                }
            } else {
                cross_counter = 0;
            }
        }
        
        /* Clear screen with dark gray */
        tiny3d_Clear(0x303030ff, TINY3D_CLEAR_ALL);
        
        /* Draw background gradient */
        draw_background_gradient();
        
        /* Draw title */
        SetFontSize(SF(22), SF(22));
        SetFontColor(0xffffffff, 0);
        DrawString(SX(35), SY(10), (char*)"vLaunchSELF v1");
        
        /* Draw controls */
        SetFontSize(SF(14), SF(14));
        SetFontColor(0xffffffff, 0);
        DrawString(SX(35), SY(45), (char*)"[X] Enter  [O] Exit  [△] Up/Devices  [↑↓] Navigate");
        
        /* Draw current path */
        draw_text_with_path(current_path, 80);
        
        /* Draw file list (with scrolling for more items) */
        SetFontSize(SF(14), SF(14));
        int visible_items = 14;
        int start_idx = (selected_index >= visible_items) ? selected_index - visible_items + 1 : 0;
        int end_idx = start_idx + visible_items;
        if (end_idx > file_count) end_idx = file_count;
        
        for (int i = start_idx; i < end_idx; i++) {
            int display_idx = i - start_idx;
            if (i == selected_index) SetFontColor(0xff82b1ff, 0);  /* Light Material Cyan/Blue */
            else SetFontColor(0xffffffff, 0);
            
            char size_str[32];
            char time_str[32];
            char line[280];
            
            if (files[i].is_dir) {
                format_file_size(files[i].size, size_str, sizeof(size_str));
                snprintf(line, sizeof(line), "[D] %-15s %s", files[i].name, size_str);
            } else {
                format_file_size(files[i].size, size_str, sizeof(size_str));
                format_timestamp(files[i].mtime, time_str, sizeof(time_str));
                snprintf(line, sizeof(line), "   %-15s %5s %s", files[i].name, size_str, time_str);
            }
            DrawString(SX(20), SY(95) + display_idx * SY(28), line);
        }
        
        /* Show empty message if no files */
        if (file_count == 0) {
            SetFontColor(0xff808080, 0);
            DrawString(SX(20), SY(200), (char*)"Empty or error reading directory");
        }
        
        /* Draw footer credits */
        SetFontSize(SF(10), SF(10));
        SetFontColor(0xff808080, 0);
        DrawString(SX(20), SY(475), (char*)"Written by VTSTech - github.com/VTSTech - www.VTS-Tech.org");
        
        /* Flip buffers */
        tiny3d_Flip();
    }
    
    printf("=== vLaunchSELF Shutting Down ===\n");
    return 0;
}

#pragma GCC diagnostic pop
