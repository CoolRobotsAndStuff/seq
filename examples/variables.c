#include <stdio.h>
#define SEQ_ENABLE_STACK
#define SEQ_IMPLEMENTATION
#include "../seq.h"

#define TCOUNT 2

int main() {
    SeqThread threads[TCOUNT];
    for (int i = 0; i < TCOUNT; ++i) {
        threads[i] = seq_thread();
    }

    while (1) {
        for (int i = 0; i < TCOUNT; ++i) {
            seq_current_thread = &threads[i];
            seq_independent_memory {
                seq_start();

                char seqarr(my_array,,"goo");
                int seqv(my_int) = 0;

                static long my_long; 
                seq_register(my_long);
                seq my_long = i*100;

                float seqv(my_float) = 3.0f*i;
                char seqp(my_string) = "hello";

                // seqv(long, my_long) = i*100;
                // seqv(int, my_int) = 0;
                // seqv(char*, my_string) = "hello";

                seq_while(my_int < 4, 
                    seq printf("[%d] hi\n", i);
                    seq my_int++;
                    seq my_long += 5;
                    seq my_float -= 3.24f;
                    seq printf("[%d] my_int: %d\n", i, my_int);
                    seq printf("[%d] my_long: %ld\n", i, my_long);
                    seq printf("[%d] my_float: %f\n", i, my_float);
                    seq printf("[%d] my_string: %s\n", i, my_string);
                    seq my_array[0] = (char)(my_int+i*5)+'A';
                    seq printf("[%d] my_array[%zu]: %s\n", i, sizeof(my_array), my_array);
                )

            } // seq_independent_memory
        }
        seq_sync_all(threads, TCOUNT);
        seq break;
        sequtil_msleep(1000);
    }
    return 0;
}
