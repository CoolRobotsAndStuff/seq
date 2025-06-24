#define SEQ_IMPLEMENTATION
#include "../seq.h"

int main(void) {
    SeqThread thread1 = seq_thread();
    SeqThread thread2 = seq_thread();

    #define T1 seq_current_thread = &thread1;
    #define T2 seq_current_thread = &thread2;

    while (1) {
        T1 seq_start();

        seq puts("bim");

        // int label_0 = seq_current_thread->index+1;
        // static int out; seq_goto_index_if_not(out, j < 3);
        
        seq_for(int seqv(j) = 0, j < 3, ++j,
            seq puts("bam");
            seq puts("bum");
            seq_for(int seqv(k) = 0, k < 2, ++k,
                seq printf("hello %d\n", k);
            )
        )

        
        static int x = 4;
        seq_if(x > 0,
            seq puts("x positive!");
        ) 

        static int y = 0;
        seq_if(y > 0,
            seq puts("y positive!");
        ) seq_else_if (y%2 == 0,
            seq_if (y == 0,
                seq puts("y is zero!");
            ) seq_else (
                seq puts("y negative and even!");
            )
        ) seq_else (
            seq puts("wiii!");
        )
        
        T2 seq_start();
        seq puts("Hello from thread 2");
        static int z = -4;
        seq_if(z > 0,
            seq puts("z positive!");
        ) seq_else_if (z%2 == 0,
            seq_if (z == 0,
                seq puts("z is zero!");
            ) seq_else (
                seq puts("z negative and even!");
            )
        ) seq_else (
            seq puts("wiii!");
        )


        sleep(1);
    }
}
