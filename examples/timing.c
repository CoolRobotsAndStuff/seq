#include <stdio.h>
#define SEQ_IMPLEMENTATION
#include "../seq.h"

int main() {
    SeqThread thread1 = seq_thread();
    SeqThread thread2 = seq_thread();

    #define T1 seq_current_thread = &thread1;
    #define T2 seq_current_thread = &thread2;

    while (1) {
        T1 seq_start();      T2 seq_start();
/*             |                    |                  */
        T1 seq puts("bim");  T2 seq puts("three");
/*             |                    |                  */
        T1 seq_sleep(2);     T2 seq_sleep(1);
/*             |                    |                  */
        T1 seq puts("bam");  T2 seq puts("two");
/*             |                    |                  */
        T1 seq_sleep(2);     T2 seq_sleep(1);
/*             |                    |                  */
        T1 seq puts("bum");  T2 seq puts("one");
/*              \                  /                   */
         seq_sync_both(&thread1, &thread2);
/*                       |                             */
                T1 seq puts("BOOOOM!");
                T1 seq break;

        sequtil_mini_sleep(); /* Prevent busylooping. */
    }
    return 0;
}
