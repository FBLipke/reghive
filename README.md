# RegHive - Windows Registry Hive Parser & Editor

A pure C++ library and CLI tool for parsing and **editing** Windows Registry Hive files (REGF format). Includes BCD (Boot Configuration Data) support.

## Features

- **Parse** Windows Registry Hive files (.reg, BCD stores)
- **Read** any value (REG_SZ, REG_DWORD, REG_BINARY, REG_MULTI_SZ, etc.)
- **Write/Modify** any value in-place (no template needed)
- Support for NK, VK, lf/lh/ri subkey lists
- UTF-16 LE to UTF-8 string conversion
- Pure C++14, no external dependencies

## Building

```bash
make
```

## CLI Usage

```bash
# Read and dump BCD contents
./bin/bcdtool read /path/to/BCD

# Create new BCD with custom TFTP settings
./bin/bcdtool create output.bcd --blocksize 4096 --windowsize 16
./bin/bcdtool create output.bcd --blocksize 65536 --windowsize 32 --varwindow
```

## Library Usage

```cpp
#include "RegHive/RegHive.h"
#include "BCD/BCD.h"

// === Read Values ===
RegHive::Parser hive;
hive.Load("BCD");

// Find key by path (use / or \ as separator)
RegHive::Key* key = hive.FindKey("Objects/{GUID}/Description");

// Read DWORD value
uint32_t val = 0;
if (hive.GetDWORD("Objects/{GUID}/Description", "Type", &val)) {
    printf("Type = 0x%X\n", val);
}

// Read String value
std::wstring str;
if (hive.GetString("Objects/{GUID}/Elements/BCD_ELEM_DEVICE_BOOTFILE", "Element", &str)) {
    printf("Bootfile = %S\n", str.c_str());
}

// Navigate manually
RegHive::Key* root = hive.GetRoot();
RegHive::Key* objects = root->FindSubkey("Objects");
RegHive::Key* entry = objects->FindSubkey("{GUID}");
RegHive::Value* val = entry->FindValue("Type");

// === Write Values ===
// Modify DWORD in-place
hive.SetDWORD("Objects/{GUID}/Description", "Type", BCD_TYPE_OS_LOADER);

// Modify String in-place
hive.SetString("Objects/{GUID}/Elements/BCD_ELEM_DEVICE_BOOTFILE", "Element", L"\\Boot\\x86\\boot.sdi");

// Save modified BCD
std::ofstream out("modified.bcd", std::ios::binary);
out.write(reinterpret_cast<const char*>(hive.Data()), hive.Size());
```

## BCD Convenience API

```cpp
#include "BCD/BCD.h"

// Read TFTP settings from BCD
BCD::Parser bcd;
bcd.Load("BCD");

BCD::TFTPSettings settings;
if (bcd.GetTFTPSettings("{GUID}", &settings)) {
    printf("Blocksize: %u, Windowsize: %u\n", 
           settings.blocksize, settings.windowsize);
}

// Write TFTP settings
settings.blocksize = 65536;
settings.windowsize = 32;
settings.varwindow = true;
bcd.SetTFTPSettings("{GUID}", settings);
bcd.Save("modified.bcd");
```

## BCD Object Structure

Windows BCD files contain boot entries identified by GUIDs:

| Object | Type | Description |
|--------|------|-------------|
| `BCD_OBJ_BOOT_MANAGER` | BCD Template | Base template for all boot entries |
| `BCD_OBJ_BOOT_MANAGERS` | Library | Boot managers and loaders |
| `BCD_OBJ_BOOT_LIBRARY` | Library | Device information |
| `BCD_OBJ_EFI_BOOT_LOADER` | OS Loader | Windows PE |
| `BCD_OBJ_OS_LOADER` | OS Loader | Windows Setup |
| `BCD_OBJ_BOOT_MANAGER` | OS Loader | Windows Boot Loader |

### Common BCD Elements

| Element ID | Type | Description |
|------------|------|-------------|
| `BCD_ELEM_DEVICE_BOOTFILE` | String | Bootfile path (e.g. `\Boot\x86\boot.sdi`) |
| `BCD_ELEM_RAMDISK_SIZE` | Binary | RAM disk size |
| `BCD_ELEM_TFTP_BLOCKSIZE` | DWORD | TFTP Blocksize |
| `BCD_ELEM_TFTP_WINDOWSIZE` | DWORD | TFTP Windowsize |
| `BCD_ELEM_NET_VARSIZE` | DWORD | Variable Window Size (0/1) |

## Format

Windows Registry Hive (REGF) format:
- Header: 4096 bytes
- HBIN cells: variable size records
- NK record: Key node
- VK record: Value key (contains data inline or pointer)
- lf/lh/ri: Subkey indexes

## Examples

### 1. Read all boot entries from BCD
```cpp
RegHive::Parser hive;
hive.Load("C:\\Boot\\BCD");

RegHive::Key* root = hive.GetRoot();
RegHive::Key* objects = root->FindSubkey("Objects");

for (auto& obj : objects->subkeys) {
    printf("Boot Entry: %s\n", obj->name.c_str());
    
    RegHive::Key* desc = obj->FindSubkey("Description");
    if (desc) {
        RegHive::Value* type = desc->FindValue("Type");
        if (type) {
            printf("  Type: 0x%X\n", type->AsDWORD());
        }
    }
}
```

### 2. Modify TFTP Blocksize for WDS
```cpp
BCD::Parser bcd;
bcd.Load("\\\\192.168.1.10\\REMINST\\Boot\\BCD");

// Find WDS boot entry
BCD::TFTPSettings settings;
settings.blocksize = 8192;    // Increase for faster boot over WAN
settings.windowsize = 16;
settings.varwindow = true;

bcd.SetTFTPSettings("BCD_OBJ_EFI_BOOT_LOADER", settings);
bcd.Save("BCD_modified.bcd");
```

### 3. Change bootfile path
```cpp
RegHive::Parser hive;
hive.Load("BCD");

// Change bootfile for Windows Setup
const char* path = "Objects/BCD_OBJ_OS_LOADER/Elements/BCD_ELEM_DEVICE_BOOTFILE";

// Set new bootfile
hive.SetString(path, "Element", L"\\WINDOWS\\System32\\boot\winload.exe");

// Save
std::ofstream out("BCD_new.bcd", std::ios::binary);
out.write(reinterpret_cast<const char*>(hive.Data()), hive.Size());
```

### 4. Create new boot entry
```cpp
// Copy existing entry as template
RegHive::Parser hive;
hive.Load("BCD");

// Find template
RegHive::Key* tmpl = hive.FindKey("Objects/BCD_OBJ_BOOT_MANAGER");

// Create new entry by copying NK records (advanced)
// Note: Full entry creation requires allocating new HBIN cells
```

### 5. Dump complete BCD structure
```bash
# CLI: Read and dump all entries
./bin/bcdtool read /path/to/BCD

# CLI: Create new BCD with TFTP optimization
./bin/bcdtool create wds_bcd.bcd --blocksize 16384 --windowsize 32 --varwindow
```

### 6. Read RAM disk settings
```cpp
RegHive::Parser hive;
hive.Load("BCD");

const char* ramdisk_path = "Objects/{GUID}/Elements/BCD_ELEM_RAMDISK_SIZE";
RegHive::Key* ramdisk = hive.FindKey(ramdisk_path);

if (ramdisk) {
    RegHive::Value* elem = ramdisk->FindValue("Element");
    if (elem && elem->type == RegHive::ValueType::REG_BINARY) {
        printf("RAM Disk size (bytes): %zu\n", elem->data.size());
        // First 8 bytes typically contain the size
        uint64_t size = *reinterpret_cast<const uint64_t*>(elem->data.data());
        printf("Size: %llu MB\n", size / (1024*1024));
    }
}
```

## License

GNU General Public License v3 (GPL-3.0)

See LICENSE file for details.
