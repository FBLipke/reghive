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
if (hive.GetString("Objects/{GUID}/Elements/1200004A", "Element", &str)) {
    printf("Bootfile = %S\n", str.c_str());
}

// Navigate manually
RegHive::Key* root = hive.GetRoot();
RegHive::Key* objects = root->FindSubkey("Objects");
RegHive::Key* entry = objects->FindSubkey("{GUID}");
RegHive::Value* val = entry->FindValue("Type");

// === Write Values ===
// Modify DWORD in-place
hive.SetDWORD("Objects/{GUID}/Description", "Type", 0x10200003);

// Modify String in-place
hive.SetString("Objects/{GUID}/Elements/1200004A", "Element", L"\\Boot\\x86\\boot.sdi");

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
| `{9dea862c-5cdd-4e70-acc1-f32b344d4795}` | BCD Template | Base template for all boot entries |
| `{4636856e-540f-4170-a130-a84776f4c654}` | Library | Boot managers and loaders |
| `{3714d6c9-7bae-4bfa-a4cb-987915956ebc}` | Library | Device information |
| `{68d9e51c-a129-4ee1-9725-2ab00a957daf}` | OS Loader | Windows PE |
| `{7e2b9d3e-4cc6-4a21-bb1b-b7165bcb17d1}` | OS Loader | Windows Setup |
| `{9dea862c-5cdd-4e70-acc1-f32b344d4795}` | OS Loader | Windows Boot Loader |

### Common BCD Elements

| Element ID | Type | Description |
|------------|------|-------------|
| `12000004` | String | Bootfile path (e.g. `\Boot\x86\boot.sdi`) |
| `14000006` | Binary | RAM disk size |
| `25000004` | DWORD | TFTP Blocksize |
| `35000007` | DWORD | TFTP Windowsize |
| `3600000B` | DWORD | Variable Window Size (0/1) |

## Format

Windows Registry Hive (REGF) format:
- Header: 4096 bytes
- HBIN cells: variable size records
- NK record: Key node
- VK record: Value key (contains data inline or pointer)
- lf/lh/ri: Subkey indexes

## License

GNU General Public License v3 (GPL-3.0)

See LICENSE file for details.
