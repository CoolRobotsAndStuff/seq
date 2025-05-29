#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#define SEQ_IMPLEMENTATION
#include "../seq.h"

void seq_scanf(const char* fmt, long long int* n) {
    seq_current_thread->index += 1;
    if (seq_current_thread->index == seq_current_thread->counter) {
        int ret = scanf(fmt, n);
        if (ret < 0) {
            if (errno != EAGAIN) {
                perror("seq_scanf");
                return;
            }
        } else {
            seq_current_thread->counter += 1;
        }
    }
}

#define SEQ_POOL_CAPACITY 10
 
typedef struct {
    SeqThread threads[SEQ_POOL_CAPACITY];
    bool is_active[SEQ_POOL_CAPACITY];
} SeqThreadPool;

int main() {
    SeqThread thread1 = seq_thread();
    SeqThreadPool pool = {0};

    #define T1 seq_set_current(&thread1);

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
        T1
        seq_start();
        seq printf("Find closest prime after: ");
        bool seqv(responded) = false;
        long long seqv(n);
        seq_scanf("%lld", &n);
        seq {
            for (int ti = 0; ti < SEQ_POOL_CAPACITY; ++ti) {
                if (!pool.is_active[ti]) {
                    pool.is_active[ti] = true;
                    pool.threads[ti] = seq_thread();

                    seq_set_current(&pool.threads[ti]);
                        seq_independent_memory {
                            seq_start();
                            long long seqv(start) = n; 
                        }

                    seq_set_current(&thread1);
                    break;
                }
            }
        }
        seq responded = true;

        T1 seq_reset();

        for (int ti = 0; ti < SEQ_POOL_CAPACITY; ++ti) {
            if (!pool.is_active[ti]) continue;
            seq_set_current(&pool.threads[ti]);
            seq_independent_memory {

                seq_start();
                seq_wait_for_other(&thread1);

                long long seqv(start); // load from initialization
                long long seqv(possible_prime) = start; 
                bool seqv(found) = false;
                long long seqv(i) = 0;
                bool seqv(is_prime) = true;

                seq_while (!found) {
                    seq is_prime = true;
                    seq i = possible_prime-1;

                    seq_while (i>1 && is_prime) {
                        seq_if (possible_prime % i == 0)
                            seq is_prime = false;
                        seq i--;
                    }

                    seq_if (is_prime) {
                        seq {
                            if (!responded) printf("\n\033[F\033[K");
                            printf("Closest prime after %lld is %lld\n", start, possible_prime);
                            if (!responded) printf("Find closest prime after: ");
                            found = true;
                        }
                    } seq_else {
                        seq possible_prime += 1;
                    }

                }
                seq found = false;
                seq pool.is_active[ti] = false;

            } // seq_independent_memory
        }
    }
    return 1;
}
