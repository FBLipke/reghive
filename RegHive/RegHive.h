/**
 * RegHive.h - Windows Registry Hive Parser (Pure C++)
 * Based on libregf specification
 */

#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <cstring>

namespace RegHive {
    
    enum class ValueType : uint32_t {
        NONE = 0,
        REG_SZ = 1,
        REG_EXPAND_SZ = 2,
        REG_BINARY = 3,
        REG_DWORD = 4,
        REG_DWORD_BE = 5,
        REG_LINK = 6,
        REG_MULTI_SZ = 7,
        REG_RESOURCE_LIST = 8,
        REG_FULL_RESOURCE = 9,
        REG_RESOURCE_REQUIREMENTS = 10,
        REG_QWORD = 11,
    };
    
#pragma pack(push, 1)
    
    // File header is 512 bytes
    struct FileHeader {
        char     magic[4];           // 0: "regf"
        uint32_t primary_seq;        // 4
        uint32_t secondary_seq;      // 8
        uint64_t last_modified;      // 12
        uint32_t major_version;      // 20
        uint32_t minor_version;      // 24
        uint32_t file_type;         // 28
        uint32_t format;            // 32
        int32_t  root_key_offset;    // 36: Root key offset (relative to HBIN data area)
        uint32_t hbin_data_size;    // 40
        uint32_t cluster_factor;     // 44
        char     filename[64];       // 48
        uint8_t  reserved1[396];    // 112
        uint32_t checksum;          // 508
    };
    
    // HBIN header is 32 bytes
    struct HBINHeader {
        char     magic[4];           // 0: "hbin"
        uint32_t hbin_offset;        // 4
        uint32_t hbin_size;          // 8
        uint8_t  reserved1[16];     // 12
        uint64_t timestamp;          // 20
        uint32_t spare;             // 28
    };
    
    // NK (named key) record - version 1.2+
    // Cell: [size:4][nk signature at offset 4]
    // NK fields are relative to the "nk" signature position
    struct NKRecord {
        uint16_t magic;                 // 0: "nk" = 0x6B6E
        uint16_t flags;                  // 2
        uint64_t last_written;           // 4
        uint32_t unknown1;              // 12
        int32_t  parent_offset;         // 16
        uint32_t subkey_count;          // 20
        uint32_t volatile_count;        // 24
        int32_t  subkeys_offset;        // 28
        int32_t  volatile_offset;        // 32
        uint32_t value_count;            // 36
        int32_t  values_offset;         // 40
        int32_t  security_offset;       // 44
        int32_t  classname_offset;      // 48
        uint32_t largest_subkey_name;   // 52
        uint32_t largest_classname;     // 56
        uint32_t largest_value_name;    // 60
        uint32_t largest_value_data;    // 64
        uint32_t unknown2;              // 68
        uint16_t name_length;            // 72
        uint16_t classname_length;      // 74
        // Name starts at offset 76
    };
    
    // VK (value key) record
    struct VKRecord {
        int32_t  size;              // 0: Cell size
        uint16_t magic;             // 4: "vk" = 0x6B76
        uint16_t name_length;        // 6
        uint32_t data_size;          // 8
        uint32_t data_offset;        // 12
        ValueType type;             // 16
        uint16_t flags;             // 20
        uint16_t spare;            // 22
        uint16_t name_length2;     // 24
    };
    
    // lf/lh/ri subkey list
    struct LFRecord {
        uint16_t magic;
        uint16_t count;
    };
    
    struct LFEntry {
        uint32_t hash;
        int32_t  offset;
    };
    
#pragma pack(pop)
    
    struct Value {
        std::string name;
        ValueType type;
        std::vector<uint8_t> data;
        int32_t vk_offset;  // File offset of VK record (for in-place modification)
        
        uint32_t AsDWORD() const;
        uint64_t AsQWORD() const;
        bool AsBOOL() const;
        std::wstring AsWSTRING() const;
        std::string AsSTRING() const;
    };
    
    struct Key {
        std::string name;
        std::string path;
        int32_t file_offset;
        std::map<std::string, Value> values;
        std::vector<std::shared_ptr<Key>> subkeys;
        
        const Value* FindValue(const std::string& n) const;
        Value* FindValue(const std::string& n);
        const Key* FindSubkey(const std::string& n) const;
        Key* FindSubkey(const std::string& n);
    };
    
    class Parser {
    public:
        Parser();
        ~Parser();
        
        bool Load(const char* filename);
        bool Load(const uint8_t* data, size_t size);
        
        bool Parse();
        
        const Key* GetRoot() const { return m_root.get(); }
        Key* GetRoot() { return m_root.get(); }
        const Key* FindKey(const std::string& path) const;
        Key* FindKey(const std::string& path);
        
        // Read values
        bool GetDWORD(const std::string& path, const std::string& name, uint32_t* value);
        bool GetString(const std::string& path, const std::string& name, std::wstring* value);
        
        // Modify values in-place
        bool SetDWORD(const std::string& path, const std::string& name, uint32_t value);
        bool SetString(const std::string& path, const std::string& name, const std::wstring& value);
        
        void Dump() const;
        
        const uint8_t* Data() const { return m_data.data(); }
        size_t Size() const { return m_data.size(); }
        bool IsValid() const { return m_valid; }
        
    private:
        std::vector<uint8_t> m_data;
        std::shared_ptr<Key> m_root;
        int32_t m_hbin_base;
        bool m_valid;
        
        const uint8_t* GetPtr(int32_t offset) const;
        uint8_t* GetPtrWritable(int32_t offset);
        std::shared_ptr<Key> ParseNKData(const uint8_t* nk, int32_t offset);
        void ParseValues(std::shared_ptr<Key> key, int32_t offset, uint32_t count);
        void ParseSubkeys(std::shared_ptr<Key> parent, int32_t offset, uint32_t count);
        std::shared_ptr<Key> ParseNK(int32_t offset);
        Value ParseVK(const VKRecord* vk, const uint8_t* vk_base);
        void ParseSubkeys(std::shared_ptr<Key> parent, int32_t offset);
    };
    
    inline uint32_t Value::AsDWORD() const {
        // Inline DWORD: data contains the raw value bytes (copied from VK data_offset field)
        if (type == ValueType::REG_DWORD && data.size() >= 4) {
            return *reinterpret_cast<const uint32_t*>(data.data());
        }
        return 0;
    }
    
    inline std::string Value::AsSTRING() const {
        if (data.empty()) return "";
        // UTF-16 LE string, skip 4-byte size prefix
        if (data.size() >= 4) {
            const uint8_t* str_data = data.data() + 4;
            size_t str_len = (data.size() - 4) / 2;
            std::string s;
            for (size_t i = 0; i < str_len; i++) {
                uint16_t c = str_data[i * 2] | (str_data[i * 2 + 1] << 8);
                if (c == 0) break;
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
            return s;
        }
        return "";
    }
    
    inline uint64_t Value::AsQWORD() const {
        if (type == ValueType::REG_QWORD && data.size() >= 8) {
            return *reinterpret_cast<const uint64_t*>(data.data());
        }
        return 0;
    }
    
    inline bool Value::AsBOOL() const {
        if (type == ValueType::REG_DWORD) return AsDWORD() != 0;
        return !data.empty() && data[0] != 0;
    }
    
    inline std::wstring Value::AsWSTRING() const {
        if (data.empty()) return L"";
        return std::wstring(reinterpret_cast<const wchar_t*>(data.data()), 
                           data.size() / sizeof(wchar_t));
    }
    
    inline const Value* Key::FindValue(const std::string& n) const {
        auto it = values.find(n);
        return (it != values.end()) ? &it->second : nullptr;
    }
    
    inline const Key* Key::FindSubkey(const std::string& n) const {
        for (const auto& s : subkeys) {
            if (s->name == n) return s.get();
        }
        return nullptr;
    }
    
    inline Value* Key::FindValue(const std::string& n) {
        auto it = values.find(n);
        return (it != values.end()) ? &it->second : nullptr;
    }
    
    inline Key* Key::FindSubkey(const std::string& n) {
        for (const auto& s : subkeys) {
            if (s->name == n) return s.get();
        }
        return nullptr;
    }
}
