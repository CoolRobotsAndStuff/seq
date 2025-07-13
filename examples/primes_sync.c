#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#define SEQ_IMPLEMENTATION
#include "../seq.h"

void flush_stdin() { for (int c=' ';  c!='\n' && c!=EOF; c=getchar()); }

int main() {
    puts("Input 'q' to quit.")
    while (1) {
        long n;
        printf("Find closest prime after: ");
        int ret = scanf("%ld", &n);
        if (ret == 0) {
            if (getchar() == 'q') exit(0);
            flush_stdin();
            printf("Invalid input.\n");
            continue;
        } else if (ret < 1) {
            perror("Tha fuck");
            return 1;
        }
        int64_t start_time = seq_get_time_ns();
        long possible_prime = n;

        try_again:
        for (long long i = possible_prime/2; i>1; --i) {
            if (possible_prime % i == 0) {
                possible_prime += 1;
                goto try_again;
            }
        }
        printf("Closest prime after %ld is %ld\n", n, possible_prime);
        int64_t end_time = seq_get_time_ns();
        printf("Calculation took %f s\n", (end_time-start_time)/1000.0/1000.0/1000.0);
    }
    return 1;
}
