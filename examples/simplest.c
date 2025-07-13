#include <stdio.h>
#define SEQ_IMPLEMENTATION
#include "../seq.h"

int main() {
    SeqThread t = seq_thread();
    seq_current_thread = &t;
    while (1) {
        seq puts("hello");
        seq_sleep(1);
        seq puts("world");
        seq break;
    }
    return 0;
}
