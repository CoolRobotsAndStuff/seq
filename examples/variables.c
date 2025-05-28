#include <stdio.h>
#define SEQ_IMPLEMENTATION
#include "../seq.h"

#define TCOUNT 7

int main() {
    SeqThread threads[TCOUNT];
    for (int i = 0; i < TCOUNT; ++i) {
        threads[i] = seq_thread();
    }

    bool finish = false;
    
    while (!finish) {
        for (int i = 0; i < TCOUNT; ++i) {
            seq_set_current(&threads[i]);
            seq_independent_memory {

                seq_start();
                int seqv(counter) = 0;
                int seqv(counter2) = 0;
                float seqv(my_decimal) = 3.0f;
                seq_while(counter < 4) {
                    seq printf("[%d] hi\n", i);
                    seq counter++;
                    seq counter2 += 5;
                    seq my_decimal -= 3.24f;
                    seq printf("[%d] counter: %d\n", i, counter);
                    seq printf("[%d] counter2: %d\n", i, counter2);
                    seq printf("[%d] my_decimal: %f\n", i, my_decimal);
                }
                seq finish = 1;

            } // seq_independent_memory
        }
    }
    return 1;
}
