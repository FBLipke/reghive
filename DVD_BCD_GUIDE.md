# DVD/BCD Complete API Guide

This guide documents all 9 boot objects in `dvd.bcd` with examples how to read/write them using the RegHive API.

**Always include the enum header for readable constants:**
```cpp
#include "BCD/BCDEnums.h"
```

## BCD Objects Overview

| Define | GUID | Type | Description |
|--------|------|------|-------------|
| `BCD_OBJ_BOOT_LIBRARY` | `{0ce4991b-e6b3-4b16-b23c-5e0d9250e5d9}` | Library | Boot Entries |
| `BCD_OBJ_BOOT_MANAGERS` | `{4636856e-540f-4170-a130-a84776f4c654}` | Library | Global Boot Managers |
| `BCD_OBJ_EFI_APP` | `{6efb52bf-1766-41db-a6b3-0ee5eff72bd7}` | App | EFI Application |
| `BCD_OBJ_EFI_BOOT_LOADER` | `{7619dcc8-fafe-11d9-b411-000476eba25f}` | App | EFI Boot Loader |
| `BCD_OBJ_OS_LOADER` | `{7619dcc9-fafe-11d9-b411-000476eba25f}` | **OS Loader** | Windows Setup/PE |
| `BCD_OBJ_APP_RAMDISK` | `{7ea2e1ac-2e61-4728-aaa3-896d9d0a9f0e}` | App | RAM Disk related |
| `BCD_OBJ_HYPERVISOR` | `{7ff607e0-4395-11db-b0de-0800200c9a66}` | App | Hypervisor |
| `BCD_OBJ_BOOT_MANAGER` | `{9dea862c-5cdd-4e70-acc1-f32b344d4795}` | **Boot Manager** | Windows Boot Manager |
| `BCD_OBJ_MEM_DIAGNOSTIC` | `{b2721d73-1db4-4c62-bf78-c548a880142d}` | App | Memory Diagnostic |

## Common Element IDs

| Define | Value | Type | Description |
|--------|-------|------|-------------|
| `BCD_ELEM_DEVICE_PATH` | `0x12000002` | String | OS Loader path |
| `BCD_ELEM_DEVICE_BOOTFILE` | `0x12000004` | String | Description/name |
| `BCD_ELEM_DEVICE_LOCALE` | `0x12000005` | String | Locale (e.g. "en-US") |
| `BCD_ELEM_RAMDISK_SIZE` | `0x14000006` | Binary | RAM Disk size |
| `BCD_ELEM_BM_DISPLAY_ORDER` | `0x23000003` | Binary | Boot order (GUIDs) |
| `BCD_ELEM_BM_DEFAULT` | `0x24000001` | Binary | Default boot entry |
| `BCD_ELEM_TFTP_BLOCKSIZE` | `0x25000004` | DWORD | TFTP Blocksize |
| `BCD_ELEM_TFTP_WINDOWSIZE` | `0x25000005` | DWORD | TFTP Windowsize |
| `BCD_ELEM_DISABLE` | `0x26000010` | DWORD | Disable (0/1) |
| `BCD_ELEM_RECOVERY` | `0x26000022` | DWORD | Recovery enabled |
| `BCD_ELEM_EFI_APP_DEVICE` | `0x31000003` | Binary | EFI App device |
| `BCD_ELEM_EFI_APP_PATH` | `0x32000004` | String | EFI App path |
| `BCD_ELEM_NET_VARSIZE` | `0x3600000B` | DWORD | Variable Window Size |

## Type Values

| Define | Value | Description |
|--------|-------|-------------|
| `BCD_TYPE_BOOT_MANAGER` | `0x10000002` | Boot manager |
| `BCD_TYPE_OS_LOADER` | `0x10200002` | OS Loader (Vista+) |
| `BCD_TYPE_MEM_DIAG` | `0x10200005` | Memory Diagnostic |
| `BCD_TYPE_BOOT_LIBRARY` | `0x20100000` | Boot Library |
| `BCD_TYPE_EFI_APP` | `0x30200000` | EFI Application |

---

## Examples

### 1. Read OS Loader Settings

```cpp
#include "RegHive/RegHive.h"
#include "BCD/BCDEnums.h"

RegHive::Parser hive;
hive.Load("dvd.bcd");

// Read with defines - no magic numbers!
std::wstring loader, desc;
uint32_t blocksize;

hive.GetString(BCD_OBJ_OS_LOADER, BCD_ELEM_DEVICE_PATH, &loader);
hive.GetString(BCD_OBJ_OS_LOADER, BCD_ELEM_DEVICE_BOOTFILE, &desc);
hive.GetDWORD(BCD_OBJ_OS_LOADER, BCD_ELEM_TFTP_BLOCKSIZE, &blocksize);

printf("Loader: %S\n", loader.c_str());
printf("Description: %S\n", desc.c_str());
printf("TFTP Blocksize: %u\n", blocksize);
```

### 2. Modify Boot Manager

```cpp
#include "RegHive/RegHive.h"
#include "BCD/BCDEnums.h"

RegHive::Parser hive;
hive.Load("dvd.bcd");

// Change boot manager name
hive.SetString(BCD_OBJ_BOOT_MANAGER, BCD_ELEM_DEVICE_BOOTFILE, L"WDS Boot");

// Set locale
hive.SetString(BCD_OBJ_BOOT_MANAGER, BCD_ELEM_DEVICE_LOCALE, L"de-DE");

// Set TFTP Blocksize
hive.SetDWORD(BCD_OBJ_BOOT_MANAGER, BCD_ELEM_TFTP_BLOCKSIZE, 16384);

// Save
std::ofstream out("dvd_modified.bcd", std::ios::binary);
out.write(reinterpret_cast<const char*>(hive.Data()), hive.Size());
```

### 3. Configure OS Loader for WDS/PXE Boot

```cpp
#include "RegHive/RegHive.h"
#include "BCD/BCDEnums.h"
#include <fstream>

RegHive::Parser hive;
hive.Load("dvd.bcd");

// Configure OS Loader
hive.SetString(BCD_OBJ_OS_LOADER, BCD_ELEM_DEVICE_BOOTFILE, L"Windows PE (Network)");
hive.SetString(BCD_OBJ_OS_LOADER, BCD_ELEM_DEVICE_LOCALE, L"en-US");
hive.SetString(BCD_OBJ_OS_LOADER, BCD_ELEM_DEVICE_PATH, 
    L"\\Windows\\System32\\boot\\winload.exe");

// TFTP optimization
hive.SetDWORD(BCD_OBJ_OS_LOADER, BCD_ELEM_TFTP_BLOCKSIZE, 8192);
hive.SetDWORD(BCD_OBJ_OS_LOADER, BCD_ELEM_DISABLE, 0);
hive.SetDWORD(BCD_OBJ_OS_LOADER, BCD_ELEM_RECOVERY, 1);

// Save
std::ofstream out("dvd_wds.bcd", std::ios::binary);
out.write(reinterpret_cast<const char*>(hive.Data()), hive.Size());
```

### 4. Change Bootfile Path

```cpp
#include "RegHive/RegHive.h"
#include "BCD/BCDEnums.h"

RegHive::Parser hive;
hive.Load("dvd.bcd");

// Change EFI boot loader bootfile
hive.SetString(BCD_OBJ_EFI_BOOT_LOADER, BCD_ELEM_EFI_APP_PATH, 
    L"\\Boot\\boot.sdi");

// Save
std::ofstream out("dvd_modified.bcd", std::ios::binary);
out.write(reinterpret_cast<const char*>(hive.Data()), hive.Size());
```

### 5. Disable Boot Entry

```cpp
#include "RegHive/RegHive.h"
#include "BCD/BCDEnums.h"

RegHive::Parser hive;
hive.Load("dvd.bcd");

// Disable Memory Diagnostic
hive.SetDWORD(BCD_OBJ_MEM_DIAGNOSTIC, BCD_ELEM_DISABLE, 1);

// Save
std::ofstream out("dvd_modified.bcd", std::ios::binary);
out.write(reinterpret_cast<const char*>(hive.Data()), hive.Size());
```

### 6. Complete WDS/PXE Boot Configuration

```cpp
#include "RegHive/RegHive.h"
#include "BCD/BCDEnums.h"
#include <fstream>
#include <iostream>

int main() {
    RegHive::Parser hive;
    if (!hive.Load("dvd.bcd")) {
        std::cerr << "Failed to load BCD\n";
        return 1;
    }
    
    // ============================================
    // Configure Boot Manager
    // ============================================
    hive.SetString(BCD_OBJ_BOOT_MANAGER, BCD_ELEM_DEVICE_BOOTFILE, L"WDS Boot");
    hive.SetString(BCD_OBJ_BOOT_MANAGER, BCD_ELEM_DEVICE_LOCALE, L"en-US");
    hive.SetDWORD(BCD_OBJ_BOOT_MANAGER, BCD_ELEM_TFTP_BLOCKSIZE, 8192);
    
    // ============================================
    // Configure OS Loader (Windows Setup/PE)
    // ============================================
    hive.SetString(BCD_OBJ_OS_LOADER, BCD_ELEM_DEVICE_BOOTFILE, L"Windows PE (Network)");
    hive.SetString(BCD_OBJ_OS_LOADER, BCD_ELEM_DEVICE_LOCALE, L"en-US");
    hive.SetString(BCD_OBJ_OS_LOADER, BCD_ELEM_DEVICE_PATH, 
        L"\\Windows\\System32\\boot\\winload.exe");
    hive.SetDWORD(BCD_OBJ_OS_LOADER, BCD_ELEM_TFTP_BLOCKSIZE, 8192);
    hive.SetDWORD(BCD_OBJ_OS_LOADER, BCD_ELEM_DISABLE, 0);
    hive.SetDWORD(BCD_OBJ_OS_LOADER, BCD_ELEM_RECOVERY, 1);
    
    // ============================================
    // Configure EFI Boot Loader
    // ============================================
    hive.SetString(BCD_OBJ_EFI_BOOT_LOADER, BCD_ELEM_EFI_APP_PATH, 
        L"\\Boot\\boot.sdi");
    
    // ============================================
    // Save
    // ============================================
    std::ofstream out("dvd_wds.bcd", std::ios::binary);
    out.write(reinterpret_cast<const char*>(hive.Data()), hive.Size());
    
    std::cout << "Created dvd_wds.bcd\n";
    std::cout << "TFTP Blocksize: 8192 for all entries\n";
    
    return 0;
}
```

### 7. Read All Boot Entries

```cpp
#include "RegHive/RegHive.h"
#include "BCD/BCDEnums.h"

RegHive::Parser hive;
hive.Load("dvd.bcd");

RegHive::Key* root = hive.GetRoot();
RegHive::Key* objects = root->FindSubkey("Objects");

printf("BCD Objects (%zu):\n", objects->subkeys.size());

for (auto& obj : objects->subkeys) {
    printf("\n%s\n", obj->name.c_str());
    
    // Read description
    RegHive::Key* desc = obj->FindSubkey("Description");
    if (desc) {
        RegHive::Value* type = desc->FindValue("Type");
        if (type) {
            printf("  Type: 0x%X\n", type->AsDWORD());
        }
    }
    
    // Read elements
    RegHive::Key* elem = obj->FindSubkey("Elements");
    if (elem) {
        printf("  Elements: %zu\n", elem->subkeys.size());
    }
}
```

---

## Compile & Run

```bash
# Compile
g++ -std=c++14 -O2 -Iinclude -IRegHive -IBCD example.cpp RegHive/RegHive.o -o example

# Run
./example

# Result
Created dvd_wds.bcd
TFTP Blocksize: 8192 for all entries
```

---

## Quick Reference: All Defines

```cpp
// Object GUIDs
BCD_OBJ_BOOT_LIBRARY
BCD_OBJ_BOOT_MANAGERS
BCD_OBJ_EFI_APP
BCD_OBJ_EFI_BOOT_LOADER
BCD_OBJ_OS_LOADER
BCD_OBJ_APP_RAMDISK
BCD_OBJ_HYPERVISOR
BCD_OBJ_BOOT_MANAGER
BCD_OBJ_MEM_DIAGNOSTIC

// Types
BCD_TYPE_BOOT_MANAGER
BCD_TYPE_OS_LOADER
BCD_TYPE_MEM_DIAG
BCD_TYPE_BOOT_LIBRARY
BCD_TYPE_EFI_APP

// Elements
BCD_ELEM_DEVICE_PATH
BCD_ELEM_DEVICE_BOOTFILE
BCD_ELEM_DEVICE_LOCALE
BCD_ELEM_RAMDISK_SIZE
BCD_ELEM_BM_DISPLAY_ORDER
BCD_ELEM_BM_DEFAULT
BCD_ELEM_TFTP_BLOCKSIZE
BCD_ELEM_TFTP_WINDOWSIZE
BCD_ELEM_DISABLE
BCD_ELEM_RECOVERY
BCD_ELEM_EFI_APP_DEVICE
BCD_ELEM_EFI_APP_PATH
BCD_ELEM_NET_VARSIZE
```
