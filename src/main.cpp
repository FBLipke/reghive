/**
 * main.cpp - BCDTool CLI
 * 
 * Pure C++ BCD Parser - no libhivex dependency!
 * 
 * Usage: bcdtool <command> [options]
 * 
 * Commands:
 *   read <input.bcd>     Read and dump BCD contents
 *   create <output.bcd>  Create new BCD from template
 *   help                 Show this help
 * 
 * Options for create:
 *   --blocksize <n>      TFTP Block Size (default: 65564)
 *   --windowsize <n>     TFTP Window Size (default: 32)
 *   --varwindow          Enable Variable Window Size (RFC 7449)
 *   --no-varwindow       Disable Variable Window Size (default)
 *   --template <file>    Template BCD file (default: default.bcd)
 */

#include "BCD.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_usage(const char* prog) {
    printf("BCDTool v0.2.0 - Pure C++ BCD Parser\n");
    printf("====================================\n\n");
    printf("Usage: %s <command> [options]\n\n", prog);
    printf("Commands:\n");
    printf("  read <input.bcd>      Read and dump BCD contents\n");
    printf("  create <output.bcd>   Create new BCD from template\n");
    printf("  help                  Show this help\n\n");
    printf("Options for create:\n");
    printf("  --blocksize <n>      TFTP Block Size (default: 65564)\n");
    printf("  --windowsize <n>     TFTP Window Size (default: 32)\n");
    printf("  --varwindow          Enable Variable Window Size (RFC 7449)\n");
    printf("  --no-varwindow       Disable Variable Window Size\n");
    printf("  --template <file>    Template BCD file (default: default.bcd)\n\n");
    printf("Examples:\n");
    printf("  %s read default.bcd\n", prog);
    printf("  %s create test.bcd --blocksize 65564 --windowsize 32 --varwindow\n", prog);
}

int cmd_read(int argc, char* argv[]) {
    if (argc < 1) {
        fprintf(stderr, "Error: Missing input file\n");
        return 1;
    }
    
    const char* input_file = argv[0];
    
    printf("Reading BCD: %s\n\n", input_file);
    
    BCD::Parser parser;
    if (!parser.Load(input_file)) {
        fprintf(stderr, "Error: Cannot read BCD file\n");
        return 1;
    }
    
    parser.Dump();
    
    return 0;
}

int cmd_create(int argc, char* argv[]) {
    if (argc < 1) {
        fprintf(stderr, "Error: Missing output file\n");
        return 1;
    }
    
    const char* output_file = argv[0];
    
    // Default values
    uint32_t blocksize = 65564;
    uint32_t windowsize = 32;
    bool varwindow = false;
    const char* template_file = "default.bcd";
    
    // Parse options
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--blocksize") == 0 && i + 1 < argc) {
            blocksize = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--windowsize") == 0 && i + 1 < argc) {
            windowsize = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--varwindow") == 0) {
            varwindow = true;
        } else if (strcmp(argv[i], "--no-varwindow") == 0) {
            varwindow = false;
        } else if (strcmp(argv[i], "--template") == 0 && i + 1 < argc) {
            template_file = argv[++i];
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_usage("bcdtool");
            return 0;
        }
    }
    
    printf("BCDTool - Create BCD\n");
    printf("====================\n\n");
    printf("Output:    %s\n", output_file);
    printf("Template:  %s\n", template_file);
    printf("Blocksize: %u\n", blocksize);
    printf("Windowsize: %u\n", windowsize);
    printf("VarWindow: %s\n\n", varwindow ? "ON (RFC 7449)" : "OFF");
    
    // Load template
    BCD::Builder builder;
    if (!builder.LoadTemplate(template_file)) {
        fprintf(stderr, "Error: Cannot load template: %s\n", template_file);
        return 1;
    }
    
    // Set values
    BCD::TFTPSettings settings;
    settings.blocksize = blocksize;
    settings.windowsize = windowsize;
    settings.varwindow = varwindow;
    
    if (!builder.SetTFTPSettings(BCD::Objects::RAMDISK_OPTIONS, settings)) {
        fprintf(stderr, "Error: Cannot set TFTP settings\n");
        return 1;
    }
    
    // Save
    if (!builder.Save(output_file)) {
        fprintf(stderr, "Error: Cannot save BCD\n");
        return 1;
    }
    
    printf("BCD created successfully!\n");
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    const char* cmd = argv[1];
    
    if (strcmp(cmd, "help") == 0 || strcmp(cmd, "--help") == 0 || strcmp(cmd, "-h") == 0) {
        print_usage(argv[0]);
        return 0;
    }
    
    if (strcmp(cmd, "read") == 0) {
        return cmd_read(argc - 2, &argv[2]);
    }
    
    if (strcmp(cmd, "create") == 0) {
        return cmd_create(argc - 2, &argv[2]);
    }
    
    fprintf(stderr, "Error: Unknown command: %s\n", cmd);
    print_usage(argv[0]);
    return 1;
}
