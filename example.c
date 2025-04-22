#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#ifndef seq_get_time_ns

long long __seq_get_time_ns() {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1) return -1;
    return ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

#define seq_get_time_ns __seq_get_time_ns 

#endif //ndef seq_get_time_ns

#define MAX_IF_CACHE 100

typedef struct {
    int index;
    int counter;
    long long delay_start;
    int if_index;
    int if_cache_count;
    int if_cache_indexes[MAX_IF_CACHE];
    int if_cache_vals[MAX_IF_CACHE];
} SeqThread;

SeqThread* seq_current_thread = NULL;


bool check_if() {
    if (seq_current_thread->if_index-1 >= seq_current_thread->if_cache_count) return false; 
    return (seq_current_thread->if_cache_vals[seq_current_thread->if_index-1] != -1);
}

bool increment_if_index() {
    seq_current_thread->if_index += 1;
    return true;
}

bool get_if_result() {
    assert(seq_current_thread->if_index-1 < seq_current_thread->if_cache_count && "if_index out of range");
    return seq_current_thread->if_cache_vals[seq_current_thread->if_index-1];
}

bool save_if_result(bool cond) {
    assert(seq_current_thread->if_cache_count < MAX_IF_CACHE && "MAX_IF_CACHE exceeded");

    seq_current_thread->if_cache_vals   [seq_current_thread->if_cache_count] = cond;
    seq_current_thread->if_cache_indexes[seq_current_thread->if_cache_count] = seq_current_thread->index;
    seq_current_thread->if_cache_count += 1;
    return cond;
}


SeqThread seq_thread() {
    SeqThread ret;
    ret.index   = 0;
    ret.counter = 1;
    ret.delay_start = -1;

    ret.if_cache_count = 0;
    ret.if_index = 0;
    for (size_t i = 0; i < MAX_IF_CACHE; ++i) {
        ret.if_cache_indexes[i] = -1;
        ret.if_cache_vals[i] = -1;
    }

    return ret;
}


void seq_start() {
    seq_current_thread->index = 0;
    seq_current_thread->if_index = 0;
}

void seq_sleep(double seconds) {
    SeqThread* t = seq_current_thread;
    t->index += 1;
    if (t->index == t->counter) { 
        long long nanoseconds = seconds * 1000 * 1000 * 1000;
        if (t->delay_start < 0) { 
            t->delay_start = seq_get_time_ns();
        }
        if (seq_get_time_ns() - t->delay_start > nanoseconds) {
            t->delay_start = -1;
            t->counter += 1;
        } 
    }
}

int seq_check() {
    seq_current_thread->index += 1;
    if (seq_current_thread->index == seq_current_thread->counter) {
        seq_current_thread->counter += 1;
        return 1;
    }
    return 0;
}

void seq_jump(int index) {
    seq_current_thread->index += 1;
    if (seq_current_thread->index == seq_current_thread->counter) { 
        seq_current_thread->counter = index;
    }
}

void seq_sync_both(SeqThread* a, SeqThread* b) {
    a->index += 1;
    b->index += 1;
    if (a->index == a->counter && b->index == b->counter) { 
        a->counter += 1;
        b->counter += 1;
    }
}

void seq_sync_any(SeqThread* a, SeqThread* b) {
    a->index += 1;
    b->index += 1;
    if ((a->index == a->counter) || (b->index == b->counter)) { 
        a->counter = a->index + 1; 
        b->counter = b->index + 1; 
    }
}

void sleep_ms(long ms) {
    usleep(ms * 1000);
}


#define seq if(seq_check())

#define seq_if(cond) if(increment_if_index(),                \
    (                                                        \
        (check_if() ?                                        \
            seq_current_thread->index += 1, get_if_result()  \
            :                                                \
            seq_check() && save_if_result((cond))            \
        )                                                    \
    )                                                        \
)

#define seq_else else if(check_if() || seq_check())
#define seq_else_if(cond) seq_else seq_if(cond)


bool always_true() {
    puts("evaled always_true!");
    return true;
}

bool always_false() {
    puts("evaled always_false!");
    return false;
}

int main() {
    SeqThread thread1 = seq_thread();
    SeqThread thread2 = seq_thread();

    #define T1 seq_current_thread = &thread1;
    #define T2 seq_current_thread = &thread2;

    while (1) {
        time_t tick_time = time(NULL);
        printf("tick: %ld\n", tick_time);

        T1 seq_start();
        
        seq_if(always_false()) {
            seq puts("three");
            seq_sleep(3)   ;
            seq puts("two");
            seq_sleep(3)   ;
            seq puts("one");
        } seq_else {
            seq_if(always_false()) {
                seq puts("bim");
                seq_sleep(3)   ;
                seq puts("bam");
                seq_sleep(3)   ;
                seq puts("bum");
            } seq_else {
                seq puts("whaa");
                seq_sleep(3)   ;
                seq puts("whii");
                seq_sleep(3)   ;
                seq puts("whoo");
            }
        }

        sleep_ms(500);
    }

    // while (1) {
    //     printf("tick: %ld\n", time(NULL));
    //
    //     seq_start(&thread1); seq_start(&thread2);
    //
    //     T1 seq puts("bim");    T2 seq puts("three");
    //   //     |                         |
    //     T1 seq_sleep(3)   ;    T2 seq_sleep(4)     ;
    //   //     |                         |
    //     T1 seq puts("bam");    T2 seq puts("two")  ;
    //   //     |                         |
    //     T1 seq_sleep(3)   ;    T2 seq_sleep(6)     ;
    //   //     |                         |
    //     T1 seq puts("bum");    T2 seq puts("one")  ;
    //   //      \                       /
    //         seq_sync_any(&thread1, &thread2);
    //   //                |
    //              T1 seq puts("BOOM");
    //
    //  sleep_ms(500);
    // }
}
