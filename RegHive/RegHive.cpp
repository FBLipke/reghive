/**
 * RegHive.cpp - Windows Registry Hive Parser (Pure C++)
 * Based on libregf specification and binary analysis
 */

#include "RegHive.h"
#include <fstream>
#include <cstdio>

namespace RegHive {
    
    Parser::Parser() : m_root(nullptr), m_hbin_base(0), m_valid(false) {}
    Parser::~Parser() {}
    
    bool Parser::Load(const char* filename) {
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            printf("Error: Cannot open: %s\n", filename);
            return false;
        }
        
        size_t size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        m_data.resize(size);
        if (!file.read(reinterpret_cast<char*>(m_data.data()), size)) {
            printf("Error: Cannot read: %s\n", filename);
            return false;
        }
        
        return Parse();
    }
    
    bool Parser::Load(const uint8_t* data, size_t size) {
        m_data.assign(data, data + size);
        return Parse();
    }
    
    const uint8_t* Parser::GetPtr(int32_t offset) const {
        if (offset < 0) return nullptr;
        uint32_t abs = m_hbin_base + offset;
        if (abs >= m_data.size()) return nullptr;
        return m_data.data() + abs;
    }
    
    uint8_t* Parser::GetPtrWritable(int32_t offset) {
        if (offset < 0) return nullptr;
        uint32_t abs = m_hbin_base + offset;
        if (abs >= m_data.size()) return nullptr;
        return m_data.data() + abs;
    }
    
    Key* Parser::FindKey(const std::string& path) {
        if (!m_root) return nullptr;
        if (path.empty()) return m_root.get();
        
        std::vector<std::string> parts;
        std::string current;
        for (char c : path) {
            if (c == '\\' || c == '/') {
                if (!current.empty()) {
                    parts.push_back(current);
                    current.clear();
                }
            } else {
                current += c;
            }
        }
        if (!current.empty()) parts.push_back(current);
        
        Key* k = m_root.get();
        for (const auto& part : parts) {
            if (part.empty()) continue;
            k = k->FindSubkey(part);
            if (!k) return nullptr;
        }
        return k;
    }
    
    bool Parser::GetDWORD(const std::string& path, const std::string& name, uint32_t* value) {
        Key* key = FindKey(path);
        if (!key) return false;
        Value* val = key->FindValue(name);
        if (!val) return false;
        *value = val->AsDWORD();
        return true;
    }
    
    bool Parser::GetString(const std::string& path, const std::string& name, std::wstring* value) {
        Key* key = FindKey(path);
        if (!key) return false;
        Value* val = key->FindValue(name);
        if (!val) return false;
        *value = val->AsWSTRING();
        return true;
    }
    
    bool Parser::SetDWORD(const std::string& path, const std::string& name, uint32_t value) {
        Key* key = FindKey(path);
        if (!key) {
            printf("RegHive: Key not found: %s\n", path.c_str());
            return false;
        }
        Value* val = key->FindValue(name);
        if (!val) {
            printf("RegHive: Value not found: %s\n", name.c_str());
            return false;
        }
        
        // Modify VK record in-place
        uint8_t* vk = GetPtrWritable(val->vk_offset);
        if (!vk) return false;
        
        // VK structure: [cell_size:4][magic:2][name_len:2][data_size:4][data_offset:4][type:4][flags:2][name...]
        // For inline DWORD: data_size = 0x80000004 (bit31=1 inline, low31=4 bytes)
        *(uint32_t*)(vk + 8) = 0x80000004;  // data_size = inline 4 bytes
        *(uint32_t*)(vk + 12) = value;       // data_offset contains the actual DWORD
        *(uint32_t*)(vk + 16) = 4;           // value_type = REG_DWORD
        
        // Update in-memory copy
        val->data.assign(reinterpret_cast<const uint8_t*>(&value),
                        reinterpret_cast<const uint8_t*>(&value) + 4);
        
        return true;
    }
    
    bool Parser::SetString(const std::string& path, const std::string& name, const std::wstring& value) {
        Key* key = FindKey(path);
        if (!key) {
            printf("RegHive: Key not found: %s\n", path.c_str());
            return false;
        }
        Value* val = key->FindValue(name);
        if (!val) {
            printf("RegHive: Value not found: %s\n", name.c_str());
            return false;
        }
        
        uint8_t* vk = GetPtrWritable(val->vk_offset);
        if (!vk) return false;
        
        // For strings: update inline data (REG_SZ = type 1)
        // String format: [4-byte size][UTF-16 LE string with null terminator]
        size_t str_bytes = (value.length() + 1) * sizeof(wchar_t);
        uint32_t data_size_raw = 0x80000000 | (uint32_t)(str_bytes + 4);
        
        *(uint32_t*)(vk + 8) = data_size_raw;  // data_size with inline flag
        *(uint32_t*)(vk + 16) = 1;             // value_type = REG_SZ
        
        // Write string after VK header (8-byte aligned after name)
        uint16_t name_len = *reinterpret_cast<uint16_t*>(vk + 6);
        size_t data_off = 20 + name_len;
        data_off = (data_off + 7) & ~7;  // 8-byte align
        
        // Write 4-byte size prefix
        *(uint32_t*)(vk + data_off) = (uint32_t)str_bytes;
        
        // Write UTF-16 LE string
        wchar_t* dst = (wchar_t*)(vk + data_off + 4);
        wcscpy(dst, value.c_str());
        
        return true;
    }
    
    bool Parser::Parse() {
        if (m_data.size() < 512) {
            printf("Error: File too small\n");
            return false;
        }
        
        const uint8_t* hdr = m_data.data();
        
        if (std::strncmp((const char*)hdr, "regf", 4) != 0) {
            printf("Error: Invalid magic\n");
            return false;
        }
        
        // HBIN data area starts at 0x1000
        m_hbin_base = 0x1000;
        
        // Root key offset at header offset 36
        int32_t root_offset = *reinterpret_cast<const int32_t*>(hdr + 36);
        
        printf("=== RegHive Parser ===\n");
        printf("Magic: regf\n");
        printf("Size: %zu bytes\n", m_data.size());
        printf("Root key offset: %d (0x%X)\n", root_offset, root_offset);
        
        // Parse root key
        m_root = ParseNK(root_offset);
        if (!m_root) {
            printf("Error: Cannot parse root key\n");
            return false;
        }
        
        m_root->path = "\\";
        
        printf("\n=== Root Key: %s ===\n", m_root->name.c_str());
        printf("Subkeys: %zu\n", m_root->subkeys.size());
        printf("Values: %zu\n", m_root->values.size());
        
        for (const auto& sub : m_root->subkeys) {
            printf("\n  [%s]\n", sub->name.c_str());
            for (const auto& v : sub->values) {
                printf("    %s = ", v.second.name.c_str());
                switch (v.second.type) {
                    case ValueType::REG_DWORD:
                        printf("%u (DWORD)\n", v.second.AsDWORD());
                        break;
                    case ValueType::REG_SZ:
                    case ValueType::REG_EXPAND_SZ: {
                        std::wstring ws = v.second.AsWSTRING();
                        std::string s(ws.begin(), ws.end());
                        printf("%s\n", s.c_str());
                        break;
                    }
                    default:
                        printf("(type %d, %zu bytes)\n", 
                               (int)v.second.type, v.second.data.size());
                }
            }
        }
        
        m_valid = true;
        return true;
    }
    
    // Cell: [size:4][nk at 4][flags:2][...]
    // For REGF v1.3: nk starts at ptr + 4
    // v1.3 NK offsets from 'nk' (which is at ptr + 4):
    //   0x00: "nk" signature
    //   0x02: flags
    //   0x04: last_written (8 bytes)
    //   0x0C: unknown1
    //   0x10: parent_offset
    //   0x14: subkey_count
    //   0x18: volatile_count
    //   0x1C: subkeys_offset
    //   0x24: value_count
    //   0x28: values_offset
    //   0x48: name_length
    //   0x4C: name
    std::shared_ptr<Key> Parser::ParseNK(int32_t offset) {
        const uint8_t* ptr = GetPtr(offset);
        if (!ptr) return nullptr;
        
        // 'nk' signature is at ptr + 4 (after cell size header)
        uint16_t nk_sig = *reinterpret_cast<const uint16_t*>(ptr + 4);
        if (nk_sig != 0x6B6E) {
            return nullptr;
        }
        
        // nk points to where "nk" signature is
        const uint8_t* nk = ptr + 4;
        
        // Read fields at correct offsets from nk position (v1.2)
        uint32_t subkey_count = *reinterpret_cast<const uint32_t*>(nk + 0x14);
        uint32_t value_count = *reinterpret_cast<const uint32_t*>(nk + 36);  // v1.2: offset 36
        int32_t subkeys_offset = *reinterpret_cast<const int32_t*>(nk + 28);
        int32_t values_offset = *reinterpret_cast<const int32_t*>(nk + 40);  // v1.2: offset 40
        uint16_t name_len = *reinterpret_cast<const uint16_t*>(nk + 0x48);
        
        auto key = std::make_shared<Key>();
        key->file_offset = offset;
        
        // Name is at nk + 0x4C
        if (name_len > 0 && name_len < 256) {
            key->name = std::string((const char*)(nk + 0x4C), name_len);
        }
        
        printf("NK: offset=%d, name='%s', values=%u, subkeys=%u\n",
               offset, key->name.c_str(), value_count, subkey_count);
        
        if (value_count > 0 && values_offset >= 0) {
            ParseValues(key, values_offset, value_count);
        }
        
        if (subkey_count > 0 && subkeys_offset >= 0) {
            ParseSubkeys(key, subkeys_offset);
        }
        
        return key;
    }
    
    void Parser::ParseValues(std::shared_ptr<Key> key, int32_t offset, uint32_t count) {
        const uint8_t* ptr = GetPtr(offset);
        if (!ptr) return;
        
        // Values list: no signature, just array of 4-byte offsets starting at ptr+4
        for (uint32_t i = 0; i < count; i++) {
            int32_t vk_off = *reinterpret_cast<const int32_t*>(ptr + 4 + i * 4);
            if (vk_off <= 0) continue;
            
            const uint8_t* vk_ptr = GetPtr(vk_off);
            if (!vk_ptr) continue;
            
            uint16_t vk_sig = *reinterpret_cast<const uint16_t*>(vk_ptr + 4);
            if (vk_sig != 0x6B76) continue;
            
            // VK v1.2: [size:4]["vk":2][name_len:2][data_size:4][data_off:4][type:4][flags:2][padding:2][name...]
            const uint8_t* vk = vk_ptr + 4;  // skip cell size
            
            uint16_t name_len = *reinterpret_cast<const uint16_t*>(vk + 2);
            uint32_t data_size_raw = *reinterpret_cast<const uint32_t*>(vk + 4);
            uint32_t data_offset = *reinterpret_cast<const uint32_t*>(vk + 8);
            ValueType type = (ValueType)*reinterpret_cast<const uint32_t*>(vk + 12);
            
            // data_size: bit 31 set = inline data, low 31 bits = actual size
            uint32_t data_size = data_size_raw & 0x7FFFFFFF;
            bool is_inline = (data_size_raw & 0x80000000) != 0;
            
            Value val;
            val.vk_offset = vk_off;  // Store offset for in-place modification
            val.type = type;
            
            // Name starts at vk + 20 (after 20 bytes of header)
            if (name_len > 0 && name_len < 256) {
                val.name = std::string((const char*)(vk + 20), name_len);
            } else {
                val.name = "(Default)";
            }
            
            // Handle data location based on inline flag
            if (is_inline) {
                // MSB set: data is stored directly in data_offset field (not a pointer)
                if (data_size <= 4) {
                    val.data.assign(reinterpret_cast<const uint8_t*>(&data_offset),
                                   reinterpret_cast<const uint8_t*>(&data_offset) + data_size);
                } else {
                    // Larger inline data: after name, 8-byte aligned
                    size_t off = 20 + name_len;
                    off = (off + 7) & ~7;
                    val.data.assign(vk + off, vk + off + data_size);
                }
            } else {
                // External data: data_offset is a POINTER - read from that location
                const uint8_t* d = GetPtr(data_offset);
                if (d) val.data.assign(d, d + data_size);
            }
            
            printf("      %s (type=%d): ", val.name.c_str(), (int)type);
            // For inline REG_DWORD, data_offset CONTAINS the value directly
            if (is_inline && type == ValueType::REG_DWORD) {
                printf("%u (DWORD)\n", data_offset);
            } else if (type == ValueType::REG_DWORD && val.data.size() >= 4) {
                printf("%u (DWORD)\n", val.AsDWORD());
            } else if (type == ValueType::REG_SZ && val.data.size() >= 4) {
                // Unicode string (UTF-16 LE) - skip first 4 bytes, convert directly
                const uint8_t* str_data = val.data.data() + 4;
                size_t str_len = (val.data.size() - 4) / 2;
                
                // Convert UTF-16 LE to UTF-8 directly (no wchar_t)
                std::string s;
                for (size_t i = 0; i < str_len; i++) {
                    uint16_t c = str_data[i * 2] | (str_data[i * 2 + 1] << 8);
                    if (c == 0) break;  // null terminator
                    if (c < 0x80) s += (char)c;
                    else if (c < 0x800) {
                        s += (char)(0xC0 | (c >> 6));
                        s += (char)(0x80 | (c & 0x3F));
                    } else {
                        s += (char)(0xE0 | (c >> 12));
                        s += (char)(0x80 | ((c >> 6) & 0x3F));
                        s += (char)(0x80 | (c & 0x3F));
                    }
                }
                printf("%s\n", s.c_str());
            } else {
                printf("%zu bytes\n", val.data.size());
            }
            
            key->values[val.name] = val;
        }
    }
    
    void Parser::ParseSubkeys(std::shared_ptr<Key> parent, int32_t offset) {
        const uint8_t* ptr = GetPtr(offset);
        if (!ptr) return;
        
        // Skip cell size header (4 bytes) to get to actual data
        uint16_t magic = *reinterpret_cast<const uint16_t*>(ptr + 4);
        
        if (magic == 0x6B6E) {
            auto subkey = ParseNK(offset);
            if (subkey) {
                subkey->path = parent->path + subkey->name + "\\";
                parent->subkeys.push_back(subkey);
            }
        } else if (magic == 0x666C || magic == 0x686C) {
            // 'lf' at ptr+4, count at ptr+6, entries at ptr+8
            // Each entry: offset(4) + hash(4) = 8 bytes
            // Offset is at entries + i*8
            // Hash is at entries + i*8 + 4 (not used)
            uint16_t count = *reinterpret_cast<const uint16_t*>(ptr + 6);
            const uint8_t* entries = ptr + 8;
            for (uint16_t i = 0; i < count; i++) {
                int32_t nk_off = *reinterpret_cast<const int32_t*>(entries + i * 8);
                auto subkey = ParseNK(nk_off);
                if (subkey) {
                    subkey->path = parent->path + subkey->name + "\\";
                    parent->subkeys.push_back(subkey);
                }
            }
        }
    }
    
    const Key* Parser::FindKey(const std::string& path) const {
        if (!m_root) return nullptr;
        if (path.empty() || path == "\\") return m_root.get();
        
        std::vector<std::string> parts;
        std::string current;
        for (char c : path) {
            if (c == '\\') {
                if (!current.empty()) {
                    parts.push_back(current);
                    current.clear();
                }
            } else {
                current += c;
            }
        }
        if (!current.empty()) parts.push_back(current);
        
        const Key* k = m_root.get();
        for (const auto& part : parts) {
            if (part.empty()) continue;
            k = k->FindSubkey(part);
            if (!k) return nullptr;
        }
        return k;
    }
    
    void Parser::Dump() const {
        printf("\n=== RegHive Dump ===\n");
        printf("Valid: %s\n", m_valid ? "YES" : "NO");
        if (m_root) printf("Root: %s\n", m_root->name.c_str());
    }
}
