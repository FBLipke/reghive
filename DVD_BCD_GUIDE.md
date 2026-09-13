# DVD/BCD Complete API Guide

This guide documents all 9 boot objects in `dvd.bcd` with examples how to read/write them using the RegHive API.

## BCD Objects Overview

| Object ID | Type | Description | Common Elements |
|-----------|------|------------|-----------------|
| `{0ce4991b-e6b3-4b16-b23c-5e0d9250e5d9}` | Library | Boot Entries | 16000020 |
| `{4636856e-540f-4170-a130-a84776f4c654}` | Library | Global Boot Managers | 15000011, 15000013, 15000014 |
| `{6efb52bf-1766-41db-a6b3-0ee5eff72bd7}` | App | EFI Application | 14000006 |
| `{7619dcc8-fafe-11d9-b411-000476eba25f}` | App | EFI Boot Loader | 31000003, 32000004 |
| `{7619dcc9-fafe-11d9-b411-000476eba25f}` | **OS Loader** | Windows Setup/PE | 11000001, 12000002, 14000006, 25000004, etc. |
| `{7ea2e1ac-2e61-4728-aaa3-896d9d0a9f0e}` | App | ??? | 14000006 |
| `{7ff607e0-4395-11db-b0de-0800200c9a66}` | App | Hypervisor | 12000004, 250000f3/f4/f5 |
| `{9dea862c-5cdd-4e70-acc1-f32b344d4795}` | **Boot Manager** | Windows Boot Manager | 12000004, 23000003, 24000001, 25000004 |
| `{b2721d73-1db4-4c62-bf78-c548a880142d}` | App | Memory Diagnostic | 11000001, 12000002, 14000006 |

## Common BCD Element IDs

| Element ID | Type | Description |
|------------|------|-------------|
| `11000001` | Binary | OS Device (BCD 1703+) |
| `12000002` | String | OS Loader path (e.g. `\windows\system32\boot\winload.exe`) |
| `12000004` | String | Description/name |
| `12000005` | String | Locale (e.g. `en-US`) |
| `14000006` | Binary | RAM Disk size/modifier |
| `21000001` | Binary | BCD Device |
| `22000002` | String | OS Device path |
| `23000003` | String | Boot Manager display order (GUIDs) |
| `24000001` | Binary | BCD Library appropriate group |
| `25000004` | DWORD | TFTP Blocksize |
| `250000c2` | Binary | Custom value |
| `26000010` | DWORD | Disable (0/1) |
| `26000022` | DWORD | Recovery enabled (0/1) |
| `31000003` | Binary | EFI Application device |
| `32000004` | String | EFI Application path |

---

## Object 1: Library - `{0ce4991b-e6b3-4b16-b23c-5e0d9250e5d9}`

**Type:** 537919488 (0x20100000) = `BCD_BOOT_LIB`

**Description:** Boot library entries (inherited by all boot objects)

### Read

```cpp
RegHive::Parser hive;
hive.Load("dvd.bcd");

const char* guid = "{0ce4991b-e6b3-4b16-b23c-5e0d9250e5d9}";
RegHive::Key* obj = hive.FindKey(guid);
RegHive::Key* desc = obj->FindSubkey("Description");
RegHive::Key* elem = obj->FindSubkey("Elements");

// Read 16000020
RegHive::Key* e160 = elem->FindSubkey("16000020");
RegHive::Value* val = e160->FindValue("Element");
printf("16000020 size: %zu bytes\n", val->data.size());
```

### Write

```cpp
// Library entries usually don't need modification
// They're inherited by other boot objects
```

---

## Object 2: Library - `{4636856e-540f-4170-a130-a84776f4c654}`

**Type:** 537919488 (0x20100000) = `BCD_BOOT_LIB`

**Description:** Global boot managers library - contains settings for all boot managers

### Read

```cpp
const char* guid = "{4636856e-540f-4170-a130-a84776f4c654}";
RegHive::Key* obj = hive.FindKey(guid);
RegHive::Key* elem = obj->FindSubkey("Elements");

// Read all elements
for (auto& sk : elem->subkeys) {
    printf("Element %s:\n", sk->name.c_str());
    for (auto& kv : sk->values) {
        printf("  %s (type=%d)\n", kv.first.c_str(), (int)kv.second.type);
    }
}
```

### Elements

| Element | Type | Description |
|---------|------|-------------|
| `15000011` | Binary | Boot Manager device (1 byte) |
| `15000013` | Binary | Boot Manager application device (8 bytes) |
| `15000014` | Binary | Boot Manager application path (3 bytes) |

---

## Object 3: EFI Application - `{6efb52bf-1766-41db-a6b3-0ee5eff72bd7}`

**Type:** 538968067 (0x20200003) = `BCD_BOOT_APP`

**Description:** EFI Application entry

### Read

```cpp
const char* guid = "{6efb52bf-1766-41db-a6b3-0ee5eff72bd7}";
RegHive::Key* obj = hive.FindKey(guid);
RegHive::Key* elem = obj->FindSubkey("Elements/14000006");

RegHive::Value* val = elem->FindValue("Element");
printf("RAM Disk modifier: %zu bytes\n", val->data.size());
// val->data contains binary RAM disk settings
```

### Write

```cpp
// Modify RAM disk settings
// Note: 14000006 is binary, not DWORD
// Use direct data manipulation for binary elements
```

---

## Object 4: EFI Boot Loader - `{7619dcc8-fafe-11d9-b411-000476eba25f}`

**Type:** 805306368 (0x30200000) = `BCD_BOOT_APP`

**Description:** EFI Boot Loader (boot.sdi from DVD)

### Read

```cpp
const char* guid = "{7619dcc8-fafe-11d9-b411-000476eba25f}";
RegHive::Key* obj = hive.FindKey(guid);
RegHive::Key* elem = obj->FindSubkey("Elements");

// Read bootfile path
RegHive::Key* bootfile = elem->FindSubkey("32000004");
std::wstring path;
hive.GetString(guid, "32000004", &path);
printf("Bootfile: %S\n", path.c_str());
// Output: \boot\boot.sd

// Read device
RegHive::Key* device = elem->FindSubkey("31000003");
```

### Write

```cpp
// Change bootfile
hive.SetString("Objects/{7619dcc8-fafe-11d9-b411-000476eba25f}/Elements/32000004", 
               "Element", L"\\boot\\newboot.sd");

// Save
std::ofstream out("dvd_modified.bcd", std::ios::binary);
out.write(reinterpret_cast<const char*>(hive.Data()), hive.Size());
```

---

## Object 5: OS Loader - `{7619dcc9-fafe-11d9-b411-000476eba25f}` ⭐

**Type:** 270532611 (0x10200003) = `BCD_OS_LOADER`

**Description:** Windows Setup / PE Loader - **THIS IS THE MAIN BOOT ENTRY**

### Read

```cpp
const char* guid = "{7619dcc9-fafe-11d9-b411-000476eba25f}";
RegHive::Key* obj = hive.FindKey(guid);

// Read all elements
RegHive::Key* elem = obj->FindSubkey("Elements");
for (auto& sk : elem->subkeys) {
    RegHive::Value* val = sk->FindValue("Element");
    printf("%s: type=%d, size=%zu\n", sk->name.c_str(), (int)val->type, val->data.size());
}

// Read specific values
std::wstring loader;
hive.GetString(guid, "12000002", &loader);
printf("Loader: %S\n", loader.c_str());
// Output: \windows\system32\boot\winload.exe

std::wstring desc;
hive.GetString(guid, "12000004", &desc);
printf("Description: %S\n", desc.c_str());
// Output: Windows Setup

uint32_t tftp_block = 0;
hive.GetDWORD(guid, "25000004", &tftp_block);
printf("TFTP Blocksize: %u\n", tftp_block);
```

### Write

```cpp
// Change bootfile
hive.SetString("Objects/{7619dcc9-fafe-11d9-b411-000476eba25f}/Elements/12000002", 
               "Element", L"\\windows\\system32\\boot\\winload.efi");

// Change description
hive.SetString("Objects/{7619dcc9-fafe-11d9-b411-000476eba25f}/Elements/12000004", 
               "Element", L"Windows PE");

// Set TFTP Blocksize
hive.SetDWORD("Objects/{7619dcc9-fafe-11d9-b411-000476eba25f}/Elements/25000004", 
              "Element", 4096);

// Disable recovery
hive.SetDWORD("Objects/{7619dcc9-fafe-11d9-b411-000476eba25f}/Elements/26000022", 
              "Element", 0);

// Save
std::ofstream out("dvd_modified.bcd", std::ios::binary);
out.write(reinterpret_cast<const char*>(hive.Data()), hive.Size());
```

### Complete OS Loader Example

```cpp
#include "RegHive/RegHive.h"
#include "BCD/BCD.h"
#include <fstream>

int main() {
    RegHive::Parser hive;
    hive.Load("dvd.bcd");
    
    const char* os_guid = "{7619dcc9-fafe-11d9-b411-000476eba25f}";
    
    // Read current settings
    std::wstring desc, loader;
    uint32_t blocksize;
    
    hive.GetString(os_guid, "12000004", &desc);
    hive.GetString(os_guid, "12000002", &loader);
    hive.GetDWORD(os_guid, "25000004", &blocksize);
    
    printf("Current: %S | %S | Blocksize=%u\n", desc.c_str(), loader.c_str(), blocksize);
    
    // Modify for WDS/PXE boot
    hive.SetString(os_guid, "12000002", L"\\Windows\\System32\\boot\\winload.exe");
    hive.SetString(os_guid, "12000004", L"Windows PE (Network Boot)");
    hive.SetDWORD(os_guid, "25000004", 8192);  // TFTP Blocksize
    hive.SetDWORD(os_guid, "26000010", 0);      // Not disabled
    
    // Save
    std::ofstream out("dvd_wds.bcd", std::ios::binary);
    out.write(reinterpret_cast<const char*>(hive.Data()), hive.Size());
    
    printf("Saved to dvd_wds.bcd\n");
    return 0;
}
```

---

## Object 6: Application - `{7ea2e1ac-2e61-4728-aaa3-896d9d0a9f0e}`

**Type:** 537919488 (0x20100000)

**Description:** Another application entry (RAM disk related)

### Read/Write

```cpp
const char* guid = "{7ea2e1ac-2e61-4728-aaa3-896d9d0a9f0e}";

// Read RAM disk size
RegHive::Key* elem = hive.FindKey(guid);
RegHive::Key* ramdisk = elem->FindSubkey("Elements/14000006");
RegHive::Value* val = ramdisk->FindValue("Element");
printf("RAM disk settings: %zu bytes\n", val->data.size());
```

---

## Object 7: Hypervisor - `{7ff607e0-4395-11db-b0de-0800200c9a66}`

**Type:** 538968067 (0x20200003)

**Description:** Hypervisor settings entry

### Read

```cpp
const char* guid = "{7ff607e0-4395-11db-b0de-0800200c9a66}";
RegHive::Key* obj = hive.FindKey(guid);
RegHive::Key* elem = obj->FindSubkey("Elements");

// Read hypervisor settings
for (auto& sk : elem->subkeys) {
    printf("Setting %s:\n", sk->name.c_str());
    if (sk->name == "12000004") {
        std::wstring name;
        hive.GetString(guid, "12000004", &name);
        printf("  Name: %S\n", name.c_str());
    }
    if (sk->name == "250000f3" || sk->name == "250000f4" || sk->name == "250000f5") {
        // Hypervisor-specific DWORDs
        uint32_t val;
        hive.GetDWORD(guid, sk->name.c_str(), &val);
        printf("  Value: %u (0x%X)\n", val, val);
    }
}
```

### Write

```cpp
// Modify hypervisor settings
// 250000f3, 250000f4, 250000f5 are hypervisor-specific DWORDs
hive.SetDWORD("Objects/{7ff607e0-4395-11db-b0de-0800200c9a66}/Elements/250000f3", 
              "Element", 0);
hive.SetDWORD("Objects/{7ff607e0-4395-11db-b0de-0800200c9a66}/Elements/250000f4", 
              "Element", 0);
hive.SetDWORD("Objects/{7ff607e0-4395-11db-b0de-0800200c9a66}/Elements/250000f5", 
              "Element", 0);
```

---

## Object 8: Boot Manager - `{9dea862c-5cdd-4e70-acc1-f32b344d4795}` ⭐

**Type:** 269484034 (0x10000002) = `BCD_BOOT_MANAGER`

**Description:** Windows Boot Manager - the MAIN boot menu

### Read

```cpp
const char* guid = "{9dea862c-5cdd-4e70-acc1-f32b344d4795}";

// Read boot manager name
std::wstring name;
hive.GetString(guid, "12000004", &name);
printf("Boot Manager Name: %S\n", name.c_str());
// Output: Windows Boot Manager

// Read locale
std::wstring locale;
hive.GetString(guid, "12000005", &locale);
printf("Locale: %S\n", locale.c_str());
// Output: en-US

// Read display order (boot manager entries)
RegHive::Key* elem = hive.FindKey(guid);
RegHive::Key* order = elem->FindSubkey("Elements/23000003");
RegHive::Value* order_val = order->FindValue("Element");
// 23000003 contains binary list of GUIDs in display order

// Read default boot entry
RegHive::Key* def = elem->FindSubkey("Elements/24000001");
// Contains the default boot entry GUID

// Read TFTP Blocksize
uint32_t bs;
hive.GetDWORD(guid, "25000004", &bs);
printf("TFTP Blocksize: %u\n", bs);
```

### Write

```cpp
// Change boot manager name
hive.SetString("Objects/{9dea862c-5cdd-4e70-acc1-f32b344d4795}/Elements/12000004", 
               "Element", L" Mein Boot Manager");

// Set locale
hive.SetString("Objects/{9dea862c-5cdd-4e70-acc1-f32b344d4795}/Elements/12000005", 
               "Element", L"de-DE");

// Set TFTP Blocksize for network boot
hive.SetDWORD("Objects/{9dea862c-5cdd-4e70-acc1-f32b344d4795}/Elements/25000004", 
              "Element", 16384);
```

### Set Default Boot Entry

```cpp
// 24000001 contains the default boot entry GUID
// To set a new default, modify the binary data at offset+12
RegHive::Key* def = hive.FindKey(
    "Objects/{9dea862c-5cdd-4e70-acc1-f32b344d4795}/Elements/24000001");
RegHive::Value* val = def->FindValue("Element");

// The GUID is stored at the beginning of val->data
// Replace with desired boot entry GUID
const char* new_default = "{7619dcc9-fafe-11d9-b411-000476eba25f}";
// Note: Binary GUID modification requires careful byte handling
```

---

## Object 9: Memory Diagnostic - `{b2721d73-1db4-4c62-bf78-c548a880142d}`

**Type:** 270532613 (0x10200005) = `BCD_MEMDIAG`

**Description:** Windows Memory Diagnostic tool

### Read

```cpp
const char* guid = "{b2721d73-1db4-4c62-bf78-c548a880142d}";

std::wstring name, desc, locale;
hive.GetString(guid, "12000004", &name);
hive.GetString(guid, "12000002", &desc);
hive.GetString(guid, "12000005", &locale);

printf("Name: %S\n", name.c_str());
// Output: Windows Memory Diagnostic
printf("Path: %S\n", desc.c_str());
// Output: \boot\memtest.ex
printf("Locale: %S\n", locale.c_str());
```

### Write

```cpp
// Change memory diagnostic path
hive.SetString("Objects/{b2721d73-1db4-4c62-bf78-c548a880142d}/Elements/12000002", 
               "Element", L"\\Boot\\memtest.exe");

// Rename
hive.SetString("Objects/{b2721d73-1db4-4c62-bf78-c548a880142d}/Elements/12000004", 
               "Element", L"Memory Test Pro");
```

---

## Complete Example: Modify DVD BCD for WDS/PXE Boot

```cpp
#include "RegHive/RegHive.h"
#include <fstream>
#include <iostream>

int main() {
    RegHive::Parser hive;
    if (!hive.Load("dvd.bcd")) {
        std::cerr << "Failed to load BCD\n";
        return 1;
    }
    
    // ============================================
    // 1. Modify Boot Manager
    // ============================================
    const char* bm_guid = "{9dea862c-5cdd-4e70-acc1-f32b344d4795}";
    
    hive.SetString(bm_guid, "12000004", L"WDS Boot");           // Name
    hive.SetString(bm_guid, "12000005", L"en-US");             // Locale
    hive.SetDWORD(bm_guid, "25000004", 8192);                   // TFTP Blocksize
    
    // ============================================
    // 2. Modify OS Loader (Windows Setup)
    // ============================================
    const char* os_guid = "{7619dcc9-fafe-11d9-b411-000476eba25f}";
    
    hive.SetString(os_guid, "12000004", L"Windows PE (Network)"); // Description
    hive.SetString(os_guid, "12000005", L"en-US");                // Locale
    hive.SetDWORD(os_guid, "25000004", 8192);                      // TFTP Blocksize
    hive.SetDWORD(os_guid, "26000010", 0);                        // Not disabled
    hive.SetDWORD(os_guid, "26000022", 1);                        // Recovery enabled
    
    // ============================================
    // 3. Modify EFI Boot Loader
    // ============================================
    const char* efi_guid = "{7619dcc8-fafe-11d9-b411-000476eba25f}";
    
    hive.SetString(efi_guid, "32000004", L"\\Boot\\boot.sdi");   // Bootfile
    
    // ============================================
    // Save
    // ============================================
    std::ofstream out("dvd_wds.bcd", std::ios::binary);
    out.write(reinterpret_cast<const char*>(hive.Data()), hive.Size());
    
    std::cout << "Created dvd_wds.bcd\n";
    std::cout << "TFTP Blocksize set to 8192 for all entries\n";
    
    return 0;
}
```

## Compile & Run

```bash
# Compile
g++ -std=c++14 -O2 -Iinclude -IRegHive -IBCD example.cpp RegHive/RegHive.o -o example

# Run
./example

# Result
Created dvd_wds.bcd
TFTP Blocksize set to 8192 for all entries
```

## Type Reference

| Type Value | Hex | Name | Description |
|------------|-----|------|-------------|
| 269484034 | 0x10000002 | BCD_BOOT_MANAGER | Boot manager |
| 270532610 | 0x10200002 | BCD_OS_LOADER | OS Loader (Vista+) |
| 270532611 | 0x10200003 | BCD_OS_LOADER | OS Loader (XP/2003) |
| 270532613 | 0x10200005 | BCD_MEMDIAG | Memory Diagnostic |
| 537919488 | 0x20100000 | BCD_BOOT_LIB | Boot Library |
| 538968067 | 0x20200003 | BCD_BOOT_APP | Boot Application |
| 805306368 | 0x30200000 | BCD_EFI_APP | EFI Application |
| 805306369 | 0x30200001 | BCD_EFI_BOOT_APP | EFI Boot Application |

