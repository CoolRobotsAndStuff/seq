#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"
#include <stdbool.h>

#define streq(a, b) (strcmp((a), (b)) == 0)

#define BUILD "build/"

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
    while (argc) {
        char* opt = shift(argv, argc);
        if (streq(opt, "run")) {
            run = true;
        } else if (streq(opt, "debug")) {
            debug = true;
        }
    }

    Cmd cmd = {0};
    nob_cc(&cmd);
    cmd_append(&cmd, temp_sprintf("%s.c", target));
    nob_cc_output(&cmd, temp_sprintf("%s%s", BUILD, target));
    if (debug) {
        cmd_append(&cmd, "-ggdb");
    }
    if (!cmd_run_sync(cmd)) return 1;
    
    if (run) {
        cmd.count = 0;
        cmd_append(&cmd, temp_sprintf("%s%s", BUILD, target));
        if (!cmd_run_sync(cmd)) return 1;
    }

    return 0;
}
