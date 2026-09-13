/**
 * BCDEnums.h - BCD Element IDs and Object GUIDs as #defines
 * 
 * Use these instead of magic numbers!
 */

#pragma once

// ==========================================
// BCD Object GUIDs (Boot Entries)
// ==========================================

// Boot Library
#define BCD_OBJ_BOOT_LIBRARY "{0ce4991b-e6b3-4b16-b23c-5e0d9250e5d9}"

// Global Boot Managers Library  
#define BCD_OBJ_BOOT_MANAGERS "{4636856e-540f-4170-a130-a84776f4c654}"

// EFI Application
#define BCD_OBJ_EFI_APP "{6efb52bf-1766-41db-a6b3-0ee5eff72bd7}"

// EFI Boot Loader (boot.sdi)
#define BCD_OBJ_EFI_BOOT_LOADER "{7619dcc8-fafe-11d9-b411-000476eba25f}"

// Windows Setup / PE OS Loader
#define BCD_OBJ_OS_LOADER "{7619dcc9-fafe-11d9-b411-000476eba25f}"

// Application (RAM disk related)
#define BCD_OBJ_APP_RAMDISK "{7ea2e1ac-2e61-4728-aaa3-896d9d0a9f0e}"

// Hypervisor Settings
#define BCD_OBJ_HYPERVISOR "{7ff607e0-4395-11db-b0de-0800200c9a66}"

// Windows Boot Manager
#define BCD_OBJ_BOOT_MANAGER "{9dea862c-5cdd-4e70-acc1-f32b344d4795}"

// Memory Diagnostic
#define BCD_OBJ_MEM_DIAGNOSTIC "{b2721d73-1db4-4c62-bf78-c548a880142d}"

// ==========================================
// BCD Object Types
// ==========================================

#define BCD_TYPE_BOOT_MANAGER    0x10000002  // Boot manager
#define BCD_TYPE_OS_LOADER       0x10200002  // OS Loader (Vista+)
#define BCD_TYPE_OS_LOADER_LEGACY 0x10200003 // OS Loader (XP/2003)
#define BCD_TYPE_MEM_DIAG        0x10200005  // Memory Diagnostic
#define BCD_TYPE_BOOT_LIBRARY    0x20100000  // Boot Library
#define BCD_TYPE_BOOT_APP        0x20200003  // Boot Application
#define BCD_TYPE_EFI_APP         0x30200000  // EFI Application
#define BCD_TYPE_EFI_BOOT_APP    0x30200001  // EFI Boot Application

// ==========================================
// BCD Element IDs
// ==========================================

// --- Object Description Elements ---
#define BCD_ELEM_TYPE               0x00000001  // Object type (DWORD)
#define BCD_ELEM_DESCRIPTION_KEYNAME 0x00000002  // Key name (String)

// --- Device Elements ---
#define BCD_ELEM_DEVICE             0x11000001  // OS Device (Binary, BCD 1703+)
#define BCD_ELEM_DEVICE_PATH        0x12000002  // OS Loader path (String)
#define BCD_ELEM_DEVICE_BOOTFILE    0x12000004  // Bootfile/Description (String)
#define BCD_ELEM_DEVICE_LOCALE      0x12000005  // Locale (String, e.g. "en-US")

// --- RAM Disk Elements ---
#define BCD_ELEM_RAMDISK_SIZE       0x14000006  // RAM Disk size modifier (Binary)

// --- BCD Library Elements ---
#define BCD_ELEM_LIBRARY_DEVICE     0x15000011  // Boot Manager device (Binary)
#define BCD_ELEM_LIBRARY_APP_DEV    0x15000013  // Boot Manager app device (Binary)
#define BCD_ELEM_LIBRARY_APP_PATH   0x15000014  // Boot Manager app path (Binary)

// --- OS Loader Elements ---
#define BCD_ELEM_OS_DEVICE          0x21000001  // OS Device (Binary)
#define BCD_ELEM_OS_DEVICE_PATH     0x22000002  // OS Device path (String)

// --- Boot Manager Elements ---
#define BCD_ELEM_BM_DISPLAY_ORDER   0x23000003  // Boot Manager display order (GUIDs, Binary)
#define BCD_ELEM_BM_DEFAULT         0x24000001  // Default boot entry (Binary)
#define BCD_ELEM_BM_TOOLS           0x24000010  // Boot Manager tools (Binary)

// --- TFTP Elements (PXE/WDS) ---
#define BCD_ELEM_TFTP_BLOCKSIZE     0x25000004  // TFTP Blocksize (DWORD)
#define BCD_ELEM_TFTP_WINDOWSIZE    0x25000005  // TFTP Windowsize (DWORD)

// --- Hypervisor Elements ---
#define BCD_ELEM_HV_LOAD_OPTIONS    0x250000F3  // Hypervisor load options (DWORD)
#define BCD_ELEM_HV_DEBUG_PORT      0x250000F4  // Hypervisor debug port (DWORD)
#define BCD_ELEM_HV_DEBUG_BUS       0x250000F5  // Hypervisor debug bus (DWORD)

// --- Enable/Disable Elements ---
#define BCD_ELEM_DISABLE           0x26000010  // Disable (DWORD, 0=enabled, 1=disabled)
#define BCD_ELEM_RECOVERY          0x26000022  // Recovery enabled (DWORD)
#define BCD_ELEM_BOOT_DEBUG        0x26000030  // Boot debugging (DWORD)
#define BCD_ELEM_KERNEL_DEBUG      0x26000040  // Kernel debugging (DWORD)

// --- Custom Elements ---
#define BCD_ELEM_CUSTOM            0x260000A0  // Custom element (DWORD)
#define BCD_ELEM_BOOT_SDI         0x260000B0  // Custom boot SDI (DWORD)

// --- EFI Application Elements ---
#define BCD_ELEM_EFI_APP_DEVICE    0x31000003  // EFI Application device (Binary)
#define BCD_ELEM_EFI_APP_PATH      0x32000004  // EFI Application path (String)

// --- Network Boot Elements ---
#define BCD_ELEM_NET_BOOT          0x36000009  // Network boot (DWORD, multicast)
#define BCD_ELEM_NET_VARSIZE       0x3600000B  // Variable Window Size RFC 7449 (DWORD)

// ==========================================
// Helper Macros
// ==========================================

// Format element ID as string for path building
// Usage: BCD_PATH(BCD_OBJ_OS_LOADER, "Elements", BCD_ELEM_DEVICE_PATH, "Element")
#define BCD_MAKE_ELEM_ID(high, low) ((uint32_t)((((uint32_t)(high)) << 16) | ((uint32_t)(low))))

// Common element paths
#define BCD_PATH_DESC(obj, val) obj "/Description/" val
#define BCD_PATH_ELEM(obj, elem, val) obj "/Elements/" elem "/" val
