#define SEQ_IMPLEMENTATION
#include "../seq.h"

bool always_true() {
    puts("(Im a function with side effects!)");
    return true;
}

int main(void) {
    SeqThread thread = seq_thread();

    while (1) {
        seq_current_thread = &thread;
        seq_start();
        seq puts("This...");
        seq_if(always_true(),
            seq puts("...makes sense");
        )
        seq puts(".");
        seq_sleep(1);
        seq puts(".");
        seq_sleep(1);
        seq break;

        sequtil_usleep(1);
    }
    return 0;
}
