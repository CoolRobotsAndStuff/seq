#include <stdio.h>
#include <stdbool.h>

int index_;
int counter;

bool seq_check() {
    index_++;
    if (index_ == counter) {
        counter++;
        return true;
    }
    return false;
}

#define seq if (seq_check())

bool seq_wait_until(bool cond) {
    index_++;
    if (index_ == counter && cond) {
        counter++;
        return true;
    }
    return false;
}

void seq_reset() {
    seq counter = 1;
}

int main() {
    long cycle_count = 0;
    int i = 0;

    counter = 1;
    while (true) {
        index_ = 0; 

        seq puts("Do something");
        seq_wait_until(cycle_count > 1000000000);
        seq cycle_count = 0;
        seq puts("Do something else");
        seq if (++i > 3) break;
        seq_reset();

        cycle_count++;
        // Non-sequential code can go here
    }
}
