/**
 * BCDEnums.h - BCD Element and Object IDs
 * 
 * BCD (Boot Configuration Data) Registry Hive Enums
 * Based on Microsoft BCDEdit Reference and MistyProjects docs
 */

#pragma once
#include <cstdint>

namespace BCD {
    
    // ==========================================
    // Object GUIDs (bekannte BCD Objects)
    // ==========================================
    namespace Objects {
        // Well-known BCD Object GUIDs
        constexpr const char* BOOTMGR = "{9DEA862C-5CDD-4E70-ACC1-F32B344D4795}";
        constexpr const char* FWBOOTMGR = "{A5A30FA2-3D06-4E9F-B5F4-A01DF9D1FCBA}";
        constexpr const char* RAMDISK_OPTIONS = "{AE5534E0-A924-466C-B836-758539A3EE3A}";
        constexpr const char* MEMDIAG = "{B2721D73-1DB4-4C62-BF78-C548A880142D}";
        constexpr const char* NTLDR = "{466F5A88-0AF2-4F76-9038-095B170DC21C}";
        
        // Object type prefixes (for identification)
        constexpr uint32_t BOOTMGR_TYPE = 0x10100002;
        constexpr uint32_t OSLOADER_TYPE = 0x10200003;
        constexpr uint32_t RESUME_TYPE = 0x10200004;
        constexpr uint32_t MEMDIAG_TYPE = 0x10200005;
        constexpr uint32_t LEGACY_TYPE = 0x10300006;
        constexpr uint32_t RAMDISK_TYPE = 0x30000000;
    }
    
    // ==========================================
    // Element Types (Registry Hive Types)
    // ==========================================
    enum class ElementType : uint32_t {
        NONE = 0,
        REG_SZ = 1,           // String
        REG_EXPAND_SZ = 2,    // Expandable string (REG_EXpand_SZ)
        REG_BINARY = 3,       // Binary data
        REG_DWORD = 4,        // 32-bit integer (REG_DWORD)
        REG_QWORD = 11,      // 64-bit integer (REG_QWORD)
        REG_BOolean = 17,    // Boolean (TRUE/FALSE)
    };
    
    // ==========================================
    // Generic Elements (für alle Objects)
    // ==========================================
    namespace Elements {
        enum Generic : uint32_t {
            DEVICE = 0x11000001,
            PATH = 0x12000002,
            DESCRIPTION = 0x12000004,
            LOCALE = 0x12000005,
            INHERIT = 0x14000006,
            RECOVERY_SEQUENCE = 0x14000008,
            RECOVERY_ENABLED = 0x16000009,
        };
        
        // Bootmgr/FWBootmgr Elements
        enum Bootmgr : uint32_t {
            DISPLAY_ORDER = 0x24000001,
            BOOT_SEQUENCE = 0x24000002,
            DEFAULT = 0x23000003,
            TIMEOUT = 0x25000004,
            RESUME = 0x26000005,
            RESUME_OBJECT = 0x23000006,
            TOOLS_DISPLAY_ORDER = 0x24000010,
            DISPLAY_BOOT_MENU = 0x26000020,
            NO_ERROR_DISPLAY = 0x26000021,
            BCD_DEVICE = 0x21000022,
            BCD_FILEPATH = 0x22000023,
            CUSTOM_ACTIONS = 0x27000030,
        };
        
        // OSLoader Elements
        enum OSLoader : uint32_t {
            OS_DEVICE = 0x21000001,
            SYSTEM_ROOT = 0x22000002,
            OSRESUME_OBJECT = 0x23000003,
            STAMP_DISKS = 0x26000004,
            DETECT_HAL = 0x26000010,
            KERNEL = 0x22000011,
            HAL = 0x22000012,
            DBG_TRANSPORT = 0x22000013,
            NX = 0x25000020,
            OS_PAE = 0x25000021,
            WINPE = 0x26000022,
            NO_CRASH_AUTO_REBOOT = 0x26000024,
            LAST_KNOWN_GOOD = 0x26000025,
            OSLN_INTEGRITY_CHECKS = 0x26000026,
            NO_LOW_MEM = 0x26000030,
        };
        
        // Device (Ramdisk) Elements
        enum Device : uint32_t {
            RAMDISK_IMAGE_OFFSET = 0x35000001,
            RAMDISK_TFTP_CLIENT_PORT = 0x35000002,
            RAMDISK_SDI_DEVICE = 0x31000003,
            RAMDISK_SDI_PATH = 0x32000004,
            RAMDISK_IMAGE_LENGTH = 0x35000005,
            RAMDISK_EXPORT_AS_CD = 0x36000006,
            RAMDISK_TFTP_BLOCKSIZE = 0x35000007,  // TFTP Blocksize
            RAMDISK_TFTP_WINDOWSIZE = 0x35000008, // TFTP Windowsize
            RAMDISK_MC_ENABLED = 0x36000009,
            RAMDISK_MC_TFTP_FALLBACK = 0x3600000A,
            RAMDISK_TFTP_VAR_WINDOW = 0x3600000B, // RFC 7449 Variable Window
        };
        
        // Resume Elements
        enum Resume : uint32_t {
            FILE_DEVICE = 0x21000001,
            FILE_PATH = 0x22000002,
            CUSTOM_SETTINGS = 0x26000003,
            PAE = 0x26000004,
            ASSOCIATED_OS_DEVICE = 0x21000005,
            DEBUG_OPTION_ENABLED = 0x26000006,
            BOOT_UX = 0x25000007,
        };
    }
    
    // ==========================================
    // Helper Functions
    // ==========================================
    
    /**
     * Get element type name as string
     */
    inline const char* ElementTypeToString(ElementType type) {
        switch (type) {
            case ElementType::REG_SZ: return "REG_SZ";
            case ElementType::REG_EXPAND_SZ: return "REG_EXPAND_SZ";
            case ElementType::REG_BINARY: return "REG_BINARY";
            case ElementType::REG_DWORD: return "REG_DWORD";
            case ElementType::REG_QWORD: return "REG_QWORD";
            case ElementType::REG_BOolean: return "REG_BOOLEAN";
            default: return "UNKNOWN";
        }
    }
    
    /**
     * Check if element is TFTP-related
     */
    inline bool IsTFTPElement(uint32_t element) {
        return element == Elements::Device::RAMDISK_TFTP_BLOCKSIZE ||
               element == Elements::Device::RAMDISK_TFTP_WINDOWSIZE ||
               element == Elements::Device::RAMDISK_TFTP_VAR_WINDOW;
    }
    
    /**
     * Check if element is Ramdisk-related
     */
    inline bool IsRamdiskElement(uint32_t element) {
        return (element & 0xFF000000) == 0x35000000 ||
               (element & 0xFF000000) == 0x36000000;
    }
}
