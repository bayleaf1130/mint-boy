#include "util/arguments.h"

#include <string.h>

Arguments ParseArgs(int argc, char** argv) {
    Arguments args;
    args.debug = false;
    
    for(int i = 1; i < argc; i++) {
        char* arg = argv[i];
        if (strcmp(arg, "--debug") == 0) {
            args.debug = true;
        } else {
            args.rom_name = argv[i];
        }
    }

    return args;
}
