/**
 * BCD.cpp - BCD (Boot Configuration Data) Parser Implementation
 */

#include "BCD.h"
#include <fstream>
#include <cstring>

namespace BCD {
    
    // ==========================================
    // Parser Implementation
    // ==========================================
    
    Parser::Parser() {}
    
    Parser::~Parser() {}
    
    bool Parser::Load(const char* filename) {
        return m_hive.Load(filename);
    }
    
    bool Parser::Load(const uint8_t* data, size_t size) {
        return m_hive.Load(data, size);
    }
    
    const RegHive::Key* Parser::GetObjects() const {
        return m_hive.FindKey("Objects");
    }
    
    const RegHive::Key* Parser::FindObject(const char* guid) const {
        // BCD stores objects directly under root, named by GUID
        // Path would be like: \{guid}
        return m_hive.FindKey(guid);
    }
    
    uint32_t Parser::ParseElementID(const std::string& name) const {
        // Element names are like "Element35000007"
        if (name.substr(0, 7) == "Element") {
            return std::stoul(name.substr(7), nullptr, 16);
        }
        return 0;
    }
    
    const RegHive::Value* Parser::FindElement(const RegHive::Key* obj, uint32_t element_id) const {
        if (!obj) return nullptr;
        
        // Build element name
        char name[32];
        snprintf(name, sizeof(name), "Element%08X", element_id);
        
        return obj->FindValue(name);
    }
    
    bool Parser::GetDWORD(const char* object_guid, uint32_t element_id, uint32_t* value) const {
        const RegHive::Key* obj = FindObject(object_guid);
        if (!obj) return false;
        
        const RegHive::Value* val = FindElement(obj, element_id);
        if (!val) return false;
        
        *value = val->AsDWORD();
        return true;
    }
    
    bool Parser::GetBool(const char* object_guid, uint32_t element_id, bool* value) const {
        const RegHive::Key* obj = FindObject(object_guid);
        if (!obj) return false;
        
        const RegHive::Value* val = FindElement(obj, element_id);
        if (!val) return false;
        
        *value = val->AsBOOL();
        return true;
    }
    
    bool Parser::GetString(const char* object_guid, uint32_t element_id, std::wstring* value) const {
        const RegHive::Key* obj = FindObject(object_guid);
        if (!obj) return false;
        
        const RegHive::Value* val = FindElement(obj, element_id);
        if (!val) return false;
        
        *value = val->AsWSTRING();
        return true;
    }
    
    bool Parser::GetTFTPSettings(const char* ramdisk_guid, TFTPSettings* settings) const {
        const RegHive::Key* obj = FindObject(ramdisk_guid);
        if (!obj) return false;
        
        // Get blocksize (0x35000007)
        const RegHive::Value* val = FindElement(obj, Elements::Device::RAMDISK_TFTP_BLOCKSIZE);
        settings->blocksize = val ? val->AsDWORD() : 4096;
        
        // Get windowsize (0x35000008)
        val = FindElement(obj, Elements::Device::RAMDISK_TFTP_WINDOWSIZE);
        settings->windowsize = val ? val->AsDWORD() : 1;
        
        // Get varwindow (0x3600000B)
        val = FindElement(obj, Elements::Device::RAMDISK_TFTP_VAR_WINDOW);
        settings->varwindow = val ? val->AsBOOL() : false;
        
        // Get multicast (0x36000009)
        val = FindElement(obj, Elements::Device::RAMDISK_MC_ENABLED);
        settings->multicast = val ? val->AsBOOL() : false;
        
        // Get bootfile (0x1200004A)
        val = FindElement(obj, Elements::Generic::PATH);
        settings->bootfile = val ? val->AsWSTRING() : L"";
        
        return true;
    }
    
    std::vector<std::string> Parser::GetRamdiskObjects() const {
        std::vector<std::string> result;
        
        const RegHive::Key* objects = GetObjects();
        if (!objects) return result;
        
        for (const auto& sub : objects->subkeys) {
            // Check if this object has TFTP elements
            const RegHive::Value* val = FindElement(sub.get(), 
                                                   Elements::Device::RAMDISK_TFTP_BLOCKSIZE);
            if (val) {
                result.push_back(sub->name);
            }
        }
        
        return result;
    }
    
    void Parser::Dump() const {
        printf("\n=== BCD Dump ===\n");
        printf("Hive valid: %s\n", m_hive.IsValid() ? "YES" : "NO");
        
        if (!m_hive.IsValid()) return;
        
        // Dump Objects
        const RegHive::Key* objects = GetObjects();
        if (objects) {
            printf("\nBCD Objects (%zu):\n", objects->subkeys.size());
            
            for (const auto& obj : objects->subkeys) {
                printf("\n  Object: %s\n", obj->name.c_str());
                
                // Dump all values
                for (const auto& val : obj->values) {
                    uint32_t elem_id = ParseElementID(val.second.name);
                    
                    printf("    [0x%08X] ", elem_id);
                    
                    switch (val.second.type) {
                        case RegHive::ValueType::REG_DWORD:
                            printf("DWORD = %u\n", val.second.AsDWORD());
                            break;
                        case RegHive::ValueType::REG_SZ:
                        case RegHive::ValueType::REG_EXPAND_SZ:
                            printf("STRING = %S\n", val.second.AsWSTRING().c_str());
                            break;
                        case RegHive::ValueType::REG_BINARY:
                            printf("BINARY (%zu bytes)\n", val.second.data.size());
                            break;
                        default:
                            printf("type %d\n", (int)val.second.type);
                    }
                }
            }
        }
        
        // Dump known TFTP settings
        printf("\n\n=== TFTP Settings ===\n");
        
        TFTPSettings settings;
        if (GetTFTPSettings(Objects::RAMDISK_OPTIONS, &settings)) {
            printf("Ramdisk Options ({%s}):\n", Objects::RAMDISK_OPTIONS);
            printf("  Blocksize:  %u\n", settings.blocksize);
            printf("  Windowsize: %u\n", settings.windowsize);
            printf("  VarWindow:  %s (RFC 7449)\n", settings.varwindow ? "ON" : "OFF");
            printf("  Multicast:  %s\n", settings.multicast ? "ON" : "OFF");
            printf("  Boot File: %S\n", settings.bootfile.c_str());
        }
    }
    
    // ==========================================
    // Builder Implementation  
    // ==========================================
    
    Builder::Builder() : m_modified(false) {}
    
    Builder::~Builder() {
        Clear();
    }
    
    void Builder::Clear() {
        m_data.clear();
        m_objects.clear();
        m_modified = false;
    }
    
    bool Builder::LoadTemplate(const char* filename) {
        // For now, just load raw data
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            printf("BCDBuilder: Cannot open template: %s\n", filename);
            return false;
        }
        
        size_t size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        m_data.resize(size);
        if (!file.read(reinterpret_cast<char*>(m_data.data()), size)) {
            printf("BCDBuilder: Cannot read template\n");
            return false;
        }
        
        printf("BCDBuilder: Loaded template (%zu bytes)\n", size);
        return true;
    }
    
    bool Builder::Create() {
        // TODO: Create minimal BCD structure
        printf("BCDBuilder: Create not yet implemented\n");
        return false;
    }
    
    bool Builder::SetBlocksize(const char* ramdisk_guid, uint32_t blocksize) {
        TFTPSettings& settings = m_objects[ramdisk_guid];
        settings.blocksize = blocksize;
        m_modified = true;
        return true;
    }
    
    bool Builder::SetWindowsize(const char* ramdisk_guid, uint32_t windowsize) {
        TFTPSettings& settings = m_objects[ramdisk_guid];
        settings.windowsize = windowsize;
        m_modified = true;
        return true;
    }
    
    bool Builder::SetVarWindow(const char* ramdisk_guid, bool enabled) {
        TFTPSettings& settings = m_objects[ramdisk_guid];
        settings.varwindow = enabled;
        m_modified = true;
        return true;
    }
    
    bool Builder::SetTFTPSettings(const char* ramdisk_guid, const TFTPSettings& settings) {
        m_objects[ramdisk_guid] = settings;
        m_modified = true;
        return true;
    }
    
    bool Builder::Save(const char* filename) {
        // For now, just copy raw data (needs full implementation)
        if (m_data.empty()) {
            printf("BCDBuilder: No data to save\n");
            return false;
        }
        
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            printf("BCDBuilder: Cannot create file: %s\n", filename);
            return false;
        }
        
        file.write(reinterpret_cast<const char*>(m_data.data()), m_data.size());
        printf("BCDBuilder: Saved to %s\n", filename);
        return true;
    }
}
