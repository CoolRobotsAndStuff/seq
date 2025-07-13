#include <stdio.h>
#define SEQ_ENABLE_STACK
#define SEQ_IMPLEMENTATION
#include "../seq.h"
 
int main() {
    #define THREAD_COUNT 2
    SeqThread threads[THREAD_COUNT];

    for (int ti = 0; ti < THREAD_COUNT; ++ti) {
        int initial_number = (THREAD_COUNT-ti)*10+5;
        threads[ti] = seq_thread();
        seq_load_into_stack(&threads[ti], initial_number);
    }

    while(1) {
        for (int ti = 0; ti < THREAD_COUNT; ++ti) {
            seq_current_thread = &threads[ti];
            seq_independent_memory {
                seq_start();
                int seqv(number);
                seq printf("[%d] Started with number = %d\n", ti, number);
                seq_while(number > 0,
                    seq printf("[%d] number = %d\n", ti, number);
                    seq number--;
                    seq_sleep(0.5);
                )
            }
        }

        seq_sync_all(threads, THREAD_COUNT);
        seq puts("Finished.");
        seq break;
        sequtil_mini_sleep(); 
    }
    return 0;
}
