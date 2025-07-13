#include <stdio.h>
#include <time.h>
#include <inttypes.h>
#define SEQ_ENABLE_STACK
#define SEQ_MANUAL_NONBLOCKING_STDIN
#define SEQ_IMPLEMENTATION
#include "../seq.h"

int64_t normal(long n) {
    int64_t start = seq_get_time_ns();
    long possible_prime = n;
    try_again:
    for (long i = possible_prime/2; i>1; --i) {
        if (possible_prime % i == 0) {
            possible_prime += 1;
            goto try_again;
        }
    }
    printf("%ld", possible_prime);
    printf("\r\033[K");
    int64_t end = seq_get_time_ns();
    return end - start;
}

int64_t seq_stackless(long n) {
    SeqThread t = seq_thread();
    while (1) {
        seq_current_thread = &t;
        seq_start();
        int64_t seqv(start) = seq_get_time_ns();
        long seqv(possible_prime) = n;
        int try_again = seq_label();
        seq_for (long seqv(i) = possible_prime/2, i>1, --i,
            seq_if (possible_prime % i == 0,
                seq possible_prime += 1;
                seq_goto(try_again);
            )
        )
        seq printf("%ld", possible_prime);
        seq printf("\r\033[K");
        int64_t seqv(end) = seq_get_time_ns();
        seq return end - start;
    }
}

int64_t seq_stackful(long n) {
    SeqThread t = seq_thread();
    while (1) {
        seq_current_thread = &t;
        seq_independent_memory {
            seq_start();
            int64_t seqv(start) = seq_get_time_ns();
            long seqv(possible_prime) = n;
            int try_again = seq_label();
            seq_for (long seqv(i) = possible_prime/2, i>1, --i,
                seq_if (possible_prime % i == 0,
                    seq possible_prime += 1;
                    seq_goto(try_again);
                )
            )
            seq printf("%ld", possible_prime);
            seq printf("\r\033[K");
            int64_t seqv(end) = seq_get_time_ns();
            seq return end - start;
        }
    }
}

float ns2sec(int64_t ns) {
    return ns / (1000.0f * 1000 * 1000);
}

int main() {
    long n = 8000000;
    int iters = 10;

    float baseline = 0;
    for (int i = 0; i<iters; ++i) baseline += ns2sec(normal(n));
    baseline /= iters;

    printf("Normal:        %fs\n", baseline);

    float stackless = 0;
    for (int i = 0; i<iters; ++i) stackless += ns2sec(seq_stackless(n));
    stackless /= iters;


    printf("Seq stackless: %fs, %fx more than normal\n", stackless, stackless/baseline);

    float stackful = 0;
    for (int i = 0; i<iters; ++i) stackful += ns2sec(seq_stackful(n));
    stackful /= iters;
    printf("Seq stackful:  %fs, %fx more than normal\n", stackful, stackful/baseline);
    return 0;
}

