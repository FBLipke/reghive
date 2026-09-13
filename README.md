# RegHive - Windows Registry Hive Parser

A pure C++ library for parsing Windows Registry Hive files (REGF format).

## Features

- Parse Windows Registry Hive files (.reg, BCD stores)
- Support for NK, VK, lf/lh/ri subkey lists
- Extract values (REG_SZ, REG_DWORD, REG_BINARY, etc.)
- UTF-16 LE to UTF-8 string conversion
- Pure C++14, no external dependencies

## Building

```bash
make
```

## Usage

```cpp
#include "RegHive/RegHive.h"

RegHive::Parser parser;
if (parser.Load("default.bcd")) {
    const RegHive::Key* key = parser.FindKey("\\Description");
    if (key) {
        for (auto& kv : key->values) {
            printf("%s = %s\n", kv.second.name.c_str(), ...);
        }
    }
}
```

## License

GNU General Public License v3 (GPL-3.0)

See LICENSE file for details.
