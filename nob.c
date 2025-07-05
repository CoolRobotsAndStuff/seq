#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"
#include <stdbool.h>

#define streq(a, b) (strcmp((a), (b)) == 0)

#define BUILD "build/"

typedef enum {
    WINDOWS_64,
    LINUX
} Platform;

int main(int argc, char** argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);

    char* program_name = shift(argv, argc);

    if (argc == 0) {
        nob_log(NOB_ERROR, "No target provided");
        exit(1);
    }
    char* target = shift(argv, argc);

    bool run = false;
    bool debug = false;
    Platform platform = LINUX;
    while (argc) {
        char* opt = shift(argv, argc);
        if (streq(opt, "run")) {
            run = true;
        } else if (streq(opt, "debug")) {
            debug = true;
        } else if (streq(opt, "windows")) {
            platform = WINDOWS_64;
        }
    }

    Cmd cmd = {0};
    switch (platform) {
        case LINUX:
            nob_cmd_append(&cmd, "gcc");
            break;
        case WINDOWS_64:
            nob_cmd_append(&cmd, "x86_64-w64-mingw32-gcc");
            break;
    }
    cmd_append(&cmd, temp_sprintf("%s.c", target));
    
    switch (platform) {
        case LINUX:
            nob_cc_output(&cmd, temp_sprintf("%s%s", BUILD, target));
            break;
        case WINDOWS_64:
            nob_cc_output(&cmd, temp_sprintf("%s%s.exe", BUILD, target));
            break;
    }

    if (debug) {
        cmd_append(&cmd, "-ggdb");
    }
    if (!cmd_run_sync(cmd)) return 1;
    
    if (run) {
        cmd.count = 0;
        switch (platform) {
        case LINUX:
            cmd_append(&cmd, temp_sprintf("%s%s", BUILD, target));
            break;
        case WINDOWS_64:
            cmd_append(&cmd, 
                "wineconsole",
                "cmd.exe", "/K",
                temp_sprintf("%s%s.exe", BUILD, target)
            );
            break;
        }
        if (!cmd_run_sync(cmd)) return 1;
    }

    return 0;
}
