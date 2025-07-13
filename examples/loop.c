#define SEQ_IMPLEMENTATION
#include "../seq.h"

int main() {
    SeqThread thread1 = seq_thread();
    SeqThread thread2 = seq_thread();

    #define T1 seq_current_thread = &thread1;
    #define T2 seq_current_thread = &thread2;

    while (1) {
        T1 seq_start()                ;   T2 seq_start();
        T1 seq puts("[1] starting...");   T2 seq puts("[2] starting...");
        T1 seq_sleep(1)               ;   T2 seq_sleep(3);
        T1 seq puts("[1] ...thread1") ;   T2 seq puts("[2] ...slower thread2");
        T1 seq_sleep(1)               ;   T2 seq_sleep(3);
        
        T1
        seq_for(int seqv(i) = 0, i < 6, i++,
            seq printf("[1] i = %d, ", i);
            seq_if(i % 2 == 0,
                seq puts("even");
            ) seq_else (
                seq puts("odd");
            )
            seq_sleep(1);
        )

        T1 seq puts("[1] ending thread1");
        T2 seq puts("[2] ending thread2");

        seq_sync_both(&thread1, &thread2);

        T2 seq break;
    }
    return 0;
}
