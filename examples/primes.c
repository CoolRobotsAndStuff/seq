#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#define SEQ_IMPLEMENTATION
#include "../seq.h"

int gets(char*); // stay safe kids ;)

int seq_scanf(const char* fmt, ...) {
    seq_current_thread->index += 1;
    if (seq_current_thread->index == seq_current_thread->counter) {
        va_list args;
        int ret;
        va_start(args, fmt);
        ret = vscanf(fmt, args);
        va_end(args);

        if (ret < 0) {
            if (errno == EAGAIN) {
                return INT_MIN;
            }
            perror("seq_scanf");
            seq_current_thread->counter += 1;
            return -1;
        } else {
            seq_current_thread->counter += 1;
            return ret;
        }
    }
    return 0;
}

#define SEQ_POOL_CAPACITY 10
 
typedef struct {
    SeqThread threads[SEQ_POOL_CAPACITY];
    bool is_active[SEQ_POOL_CAPACITY];
} SeqThreadPool;


int main() {
    #define TEMP_BUF_SIZE 1000
    char temp_buffer[TEMP_BUF_SIZE]; 
    SeqThread thread1 = seq_thread();
    SeqThreadPool pool = {0};

    #define T1 seq_current_thread = &thread1;

    // non-blocking stdin 
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl");
        return 1;
    }
    if (fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl");
        return 1;
    }
    
    while(1) {
        T1 seq_start();
        seq printf("Find closest prime after: ");
        bool seqv(responded) = false;
        long seqv(n);
        int ret = seq_scanf("%ld", &n);
        seq_if (ret < 1,
            seq gets(temp_buffer); // stay safe kids ;)
            seq_if (strcmp(temp_buffer, "q") == 0,
                seq exit(0);
            )
            seq printf("Invalid input: %s\n", temp_buffer);
            seq_reset();
        )
        seq {
            for (int ti = 0; ti < SEQ_POOL_CAPACITY; ++ti) {
                if (!pool.is_active[ti]) {
                    pool.is_active[ti] = true;
                    pool.threads[ti] = seq_thread();
                    // This makes it so the first variable declared 
                    // in thread "ti" will have n as a value by default
                    seq_load_into_stack(&pool.threads[ti], n);
                    break;
                }
            }
        }
        seq responded = true;
        seq_reset();


        for (int ti = 0; ti < SEQ_POOL_CAPACITY; ++ti) {
            if (!pool.is_active[ti]) continue;

            seq_current_thread = &pool.threads[ti];
            seq_independent_memory {
                seq_start();

                long seqv(start); // load from initialization 
                long seqv(possible_prime) = start;

                int try_again = seq_label();

                seq_for (long seqv(i)=possible_prime/2, i>1, --i,
                    seq_if (possible_prime % i == 0,
                        seq possible_prime += 1;
                        seq_goto(try_again);
                    )
                )
                seq {
                    // The "responded" variable is accessible by all threads, since it
                    // wasn't declared under "seq_independent_memory"
                    if (!responded) printf("\n\033[F\033[K"); 
                    printf("Closest prime after %ld is %ld\n", start, possible_prime);
                    if (!responded) printf("Find closest prime after: ");
                }
                seq pool.is_active[ti] = false;

            } // seq_independent_memory
        }
        //sleep(1);
    }
    return 0;
}
