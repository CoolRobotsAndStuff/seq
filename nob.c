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

    nob_mkdir_if_not_exists(BUILD);
    nob_mkdir_if_not_exists(BUILD"examples");

    char* program_name = shift(argv, argc);

    if (argc == 0) {
        printf(
            "Usage: ./nob target [options]\n"
            "Options:\n"
            "    run           - Run executable after compilation.\n"
            "    windows       - Crosscompile to Windows with mingw.\n"
            "                    If used with 'run' option run with wineconsole.\n"
            "                    (requirements: mingw, wine)\n"
            "    debug         - Include debug information.\n"
            "                    If used with 'run' option run with gdb.\n"
            "    valgrind      - Use with the 'run' option. Run the executable with\n"
            "                    callgrind and launch kcachegrind to see results.\n"
            "                    (requirements: valgrind, kcachegrind)\n"
            "    expand_macros - Debug info with all macros expanded.\n"
            "Examples:\n"
            "   ./nob examples/primes run\n"
            "   ./nob examples/primes run windows\n"
            "   ./nob examples/primes run debug expand_macros\n"
        );
        nob_log(NOB_ERROR, "No target provided");
        exit(1);
    }
    char* target = shift(argv, argc);

    bool run = false;
    bool debug = false;
    bool valgrind = false;
    bool debug_expand_macros = false;
    Platform platform = LINUX;
    while (argc) {
        char* opt = shift(argv, argc);
        if (streq(opt, "run")) {
            run = true;
        } else if (streq(opt, "debug")) {
            debug = true;
        } else if (streq(opt, "windows")) {
            platform = WINDOWS_64;
        } else if (streq(opt, "valgrind")) {
            valgrind = true;
        } else if (streq(opt, "expand_macros")) {
            debug_expand_macros = true;
        }
    }


    Cmd cmd = {0};

    if (debug_expand_macros) {
        char* new_target = temp_sprintf("%s.preprocessed", target);
        cmd_append(&cmd, 
            "gcc", "-E", "-P",
            temp_sprintf("%s.c", target),
            "-o", temp_sprintf("%s%s.c", BUILD, new_target)
        );
        if (!cmd_run_sync(cmd)) return 1;
        target = new_target;
        cmd.count = 0;
    }

    switch (platform) {
        case LINUX:
            nob_cmd_append(&cmd, "gcc", "-O3", "-Wall", "-Wno-unused-variable", "-Wno-unused-value");
            break;
        case WINDOWS_64:
            nob_cmd_append(&cmd, "x86_64-w64-mingw32-gcc", "-O3", "-Wall", "-Wno-unused-variable", "-Wno-unused-value");
            break;
    }

    if (debug_expand_macros) {
        cmd_append(&cmd, temp_sprintf("%s%s.c", BUILD, target));
    } else {
        cmd_append(&cmd, temp_sprintf("%s.c", target));
    }

    switch (platform) {
        case LINUX:
            nob_cc_output(&cmd, temp_sprintf("%s%s", BUILD, target));
            break;
        case WINDOWS_64:
            nob_cc_output(&cmd, temp_sprintf("%s%s.exe", BUILD, target));
            break;
    }

    if (debug || valgrind) {
        cmd_append(&cmd, "-ggdb");
    }
    if (!cmd_run_sync(cmd)) return 1;

    
    if (run) {
        cmd.count = 0;
        if (valgrind) {
            char* outfile = temp_sprintf("%s%s.callgrind.out", BUILD, target);
            cmd_append(&cmd, 
                "valgrind",
                "--tool=callgrind",
                temp_sprintf("--callgrind-out-file=%s", outfile),
                temp_sprintf("%s%s", BUILD, target)
            );
            if (!cmd_run_sync(cmd)) return 1;

            cmd.count = 0;
            cmd_append(&cmd, "kcachegrind", outfile);
            if (!cmd_run_sync(cmd)) return 1;
        } else if (debug) { 
            switch (platform) {
            case LINUX:
                cmd_append(&cmd, "gdb", temp_sprintf("%s%s", BUILD, target));
                break;
            case WINDOWS_64:
                nob_log(NOB_WARNING, "Debugging not available for windows, running executable normally");
                cmd_append(&cmd, 
                    "wineconsole",
                    "cmd.exe", "/K",
                    temp_sprintf("%s%s.exe", BUILD, target)
                );
            }
            if (!cmd_run_sync(cmd)) return 1;
        } else {
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
    }

    return 0;
}
