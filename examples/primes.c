#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#define SEQ_IMPLEMENTATION
#include "../seq.h"

#define SEQ_POOL_CAPACITY 10
 
typedef struct {
    SeqThread threads[SEQ_POOL_CAPACITY];
    bool is_active[SEQ_POOL_CAPACITY];
} SeqThreadPool;

void clear_line() {
#ifdef _WIN32
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(console, &csbi);
    COORD beggining_of_line = {.X=0, .Y=csbi.dwCursorPosition.Y};
    FillConsoleOutputCharacter(console, ' ', csbi.dwSize.X, beggining_of_line, NULL);
    SetConsoleCursorPosition(console, beggining_of_line);
#else
    printf("\n\033[F\033[K");
#endif
}


int main() {
    SeqThread input_thread = seq_thread();
    SeqThreadPool pool = {0};

    while(1) {
        seq_current_thread = &input_thread;

        seq_start();
        seq_sleep(0.001); // Make input thread less frequent

        bool seqv(responded) = false;
        seq printf("Find closest prime after: ");
        long seqv(n);
        int ret = seq_scanf("%ld", &n);
        seq_if (ret == 0,
            seq_if (getchar() == 'q',
                seq exit(0);
            )
            seq printf("Invalid input.\n");
            seq_reset();
        )
        seq {
            for (int ti = 0; ti < SEQ_POOL_CAPACITY; ++ti) {
                if (!pool.is_active[ti]) {
                    pool.is_active[ti] = true;
                    pool.threads[ti] = seq_thread();
                    seq_load_into_stack(&pool.threads[ti], n); // Passing value of n to new thread
                    break;
                }
            }
        }
        seq responded = true;
        seq_reset();
        // end of input_thread


        bool calculating = false;
        for (int ti = 0; ti < SEQ_POOL_CAPACITY; ++ti) {
            if (!pool.is_active[ti]) continue;

            calculating = true;

            seq_current_thread = &pool.threads[ti];
            seq_independent_memory { // This makes the variables different for every thread
                seq_start();

                long seqv(start); // Load value of n from initialization in input_thread
                long seqv(possible_prime) = start;

                int try_again = seq_label();

                seq_for (long seqv(i) = possible_prime/2, i>1, --i,
                    seq_if (possible_prime % i == 0,
                        seq possible_prime += 1;
                        seq_goto(try_again);
                    )
                )
                seq {
                    if (!responded) clear_line();
                    printf("Closest prime after %ld is %ld\n", start, possible_prime);
                    if (!responded) printf("Find closest prime after: ");
                }
                seq pool.is_active[ti] = false;

            }
        }

        if (!calculating) usleep(1); // prevent busylooping
    }
    return 0;
}
