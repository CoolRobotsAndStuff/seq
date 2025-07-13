#include <stdio.h>
#include <time.h>

#define SEQ_ENABLE_STACK
#define SEQ_IMPLEMENTATION
#define SEQ_MANUAL_NONBLOCKING_STDIN
#include "../seq.h"

#define SEQ_POOL_CAPACITY 10
 
typedef struct {
    SeqThread threads[SEQ_POOL_CAPACITY];
    int next_inactive;
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
    printf("\r\033[K");
#endif
}

void flush_stdin() { for (int c=' ';  c!='\n' && c!=EOF; c=getchar()); }

int main() {
    puts("\nThis program finds the next prime that comes after any number.");
    puts("You can input a really high number, like 80000000, that takes a long"
         " time to compute.");
    puts("You can then calculate smaller numbers like 7000 while"
         " the big computation is running in the background.\n");

    sequtil_set_stdin_nonblocking();
    SeqThread input_thread = seq_thread();
    SeqThreadPool pool = {0};
    
    while(1) {
        seq_current_thread = &input_thread;

        seq_start();
        seq_miss_cycles(500); // make input thread 500 times less frequent

        long seqv(n);
        seq clear_line();
        seq printf("Find closest prime after: ");
        int ret = seq_scanf("%ld", &n);
        seq_if (ret == 0,
            seq if (getchar() == 'q') {
                sequtil_set_stdin_blocking();
                return 0;
            }
            seq printf("Invalid input\n");
            seq flush_stdin(); // Flush stdin
            seq_reset();
        )

        seq_if (pool.next_inactive >= SEQ_POOL_CAPACITY,
            seq puts("Too many threads, please wait for one to finish.");
        ) seq_else (
            seq {
                SeqThread new_thread = seq_thread();
                seq_load_into_stack(&new_thread, n); // Passing value of n to new thread
                pool.threads[pool.next_inactive++] = new_thread;
            }
        )
        seq_reset();

        // end of input_thread

        for (int ti = 0; ti < pool.next_inactive; ++ti) {
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
                    clear_line();
                    printf("Closest prime after %ld is %ld\n", start, possible_prime);
                    printf("Find closest prime after: ");
                }

                seq pool.threads[ti] = pool.threads[--pool.next_inactive];
            }
        }

        if (pool.next_inactive == 0) {
            sequtil_mini_sleep(); /* prevent busylooping */
        }
    }

    sequtil_set_stdin_blocking();
    return 0;
}
