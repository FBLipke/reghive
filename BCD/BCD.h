/**
 * BCD.h - BCD (Boot Configuration Data) Parser using RegHive
 * 
 * BCD is a Windows Registry Hive with specific Objects and Elements.
 * This layer provides BCD-specific access on top of RegHive.
 */

#pragma once
#include "RegHive.h"
#include "BCDEnums.h"
#include <string>

namespace BCD {
    
    // ==========================================
    // BCD-Specific Structures
    // ==========================================
    
    struct TFTPSettings {
        uint32_t blocksize;      // Element 0x35000007
        uint32_t windowsize;      // Element 0x35000008
        bool     varwindow;      // Element 0x3600000B (RFC 7449)
        bool     multicast;     // Element 0x36000009
        std::wstring bootfile;   // Element 0x1200004A
    };
    
    // ==========================================
    // BCD Parser
    // ==========================================
    
    class Parser {
    public:
        Parser();
        ~Parser();
        
        // Load BCD from file
        bool Load(const char* filename);
        
        // Load BCD from memory
        bool Load(const uint8_t* data, size_t size);
        
        // Get all BCD objects
        const RegHive::Key* GetObjects() const;
        
        // Find object by GUID
        const RegHive::Key* FindObject(const char* guid) const;
        
        // Get element value helpers
        bool GetDWORD(const char* object_guid, uint32_t element_id, uint32_t* value) const;
        bool GetBool(const char* object_guid, uint32_t element_id, bool* value) const;
        bool GetString(const char* object_guid, uint32_t element_id, std::wstring* value) const;
        
        // Get TFTP settings from Ramdisk object
        bool GetTFTPSettings(const char* ramdisk_guid, TFTPSettings* settings) const;
        
        // Get all TFTP-capable objects
        std::vector<std::string> GetRamdiskObjects() const;
        
        // Debug
        void Dump() const;
        
        // Access to underlying RegHive
        const RegHive::Parser* GetHive() const { return &m_hive; }
        
    private:
        RegHive::Parser m_hive;
        std::string m_error;
        
        // Parse element ID from value name (format: "ElementXXXXXXXX")
        uint32_t ParseElementID(const std::string& name) const;
        
        // Get element value as specific type
        const RegHive::Value* FindElement(const RegHive::Key* obj, uint32_t element_id) const;
    };
    
    // ==========================================
    // BCD Builder
    // ==========================================
    
    class Builder {
    public:
        Builder();
        ~Builder();
        
        // Load existing BCD as template
        bool LoadTemplate(const char* filename);
        
        // Create new BCD
        bool Create();
        
        // Set TFTP settings
        bool SetTFTPSettings(const char* ramdisk_guid, const TFTPSettings& settings);
        
        // Individual setters
        bool SetBlocksize(const char* ramdisk_guid, uint32_t blocksize);
        bool SetWindowsize(const char* ramdisk_guid, uint32_t windowsize);
        bool SetVarWindow(const char* ramdisk_guid, bool enabled);
        
        // Get binary data
        const uint8_t* Data() const { return m_data.data(); }
        size_t Size() const { return m_data.size(); }
        
        // Save to file
        bool Save(const char* filename);
        
        // Clear
        void Clear();
        
    private:
        std::vector<uint8_t> m_data;
        std::map<std::string, TFTPSettings> m_objects;
        bool m_modified;
    };
}
