#include <stdio.h>
#define SEQ_IMPLEMENTATION
#include "../seq.h"

int main() {
    SeqThread t = seq_thread();
    seq_current_thread = &t;
    while (1) {
        if (seq_start()) {
            puts("[initial] This is some initialization code.");
            puts("[initial] It runs before the rest of both sync and async code.");
        }
        seq puts("[thread1] hello");
        seq_sleep(1);
        seq puts("[thread1] world");
        seq_sleep(0.5);
        seq puts("[thread1] bye bye!");
        seq break;

        puts("[on loop] printing this always");

        sequtil_usleep(1000*100); // making this slow to not flood the terminal
    }
    return 0;
}
