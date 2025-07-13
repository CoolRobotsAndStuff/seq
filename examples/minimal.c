#include <stdio.h>

#define SEQ_MINIMAL
 /* This attaches a bit of extra data to a thread inside the field 'ud'*/
#define SEQ_ADD_USER_DATA
typedef struct {
    long cycles;
} SeqUserData;
#define SEQ_ON_START seq_current_thread->ud.cycles++;

#define SEQ_IMPLEMENTATION
#include "../seq.h"

int main(void) {
    SeqThread thread1 = seq_thread();
    SeqThread thread2 = seq_thread();

    #define T1 seq_current_thread = &thread1;
    #define T2 seq_current_thread = &thread2;

    /* Note: This use of cycle_counter inteferes with the seq_miss_cycles function */
    #define seq_wait(c) do {                                               \
        seq_wait_until(seq_current_thread->ud.cycles >= (c)*10000000L); \
        seq seq_current_thread->ud.cycles = 0;                           \
    } while(0)

    while (1) {
        T1 seq_start()    ; T2 seq_start()      ;
        T1 seq_wait(15)   ; T2 seq_wait(5)      ;
        T1 seq puts("bim"); T2 seq puts("three");
        T1 seq_wait(15)   ; T2 seq_wait(5)      ;
        T1 seq puts("bam"); T2 seq puts("two")  ;
        T1 seq_wait(15)   ; T2 seq_wait(5)      ;
        T1 seq puts("bum"); T2 seq puts("one")  ;
                            T2 seq_wait(5)      ;
                            T2 seq puts("zero") ;
            seq_sync_both(&thread1, &thread2);
                 T1 seq puts("BOOM");
                    seq break;
    }
}
