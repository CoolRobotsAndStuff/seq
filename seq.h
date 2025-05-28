#ifndef SEQ_H_
#define SEQ_H_

#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

long long __seq_get_time_ns();

#ifndef seq_get_time_ns

#define seq_get_time_ns __seq_get_time_ns 

#endif //ndef seq_get_time_ns

#define MAX_IF_CACHE 100
#define MAX_SUBTHREADS 10
#define SEQ_STACK_SIZE 100

typedef struct {
    int index;
    int counter;
    long long delay_start;

    int if_index;
    int if_cache_count;
    int if_cache_indexes[MAX_IF_CACHE];
    int if_cache_vals[MAX_IF_CACHE];

    int loop_cond_cache;
} SeqSubthread;


typedef struct {
    char data[SEQ_STACK_SIZE];
    size_t place;
} SeqStack;

typedef struct {
    SeqSubthread subthreads[MAX_SUBTHREADS];
    size_t count;
    size_t current_subthread;

    SeqStack stack;
} SeqThread;

bool check_if();

bool increment_if_index();

bool get_if_result();

bool save_if_result(bool cond);


SeqSubthread seq_subthread();

void seq_start();

void seq_sleep(double seconds);

int seq_check();

int seq_check_no_counter();

void seq_reset();

void seq_jump(int index);

SeqSubthread* seq_last_subthread(SeqThread* t);

void seq_sync_with_other_subthread(SeqSubthread* t);
void seq_sync_with_other(SeqThread* t);

void seq_wait_for_other_subthread(SeqSubthread* t);
void seq_wait_for_other(SeqThread* t);

void seq_sync_both(SeqThread* a, SeqThread* b);
void seq_sync_any(SeqThread* a, SeqThread* b);

void sleep_ms(long ms);

#define seq if(seq_check())

#define seq_if(cond) if(increment_if_index(),        \
    (check_if() ?                                     \
        seq_current_thread->index += 1, get_if_result()\
        :                                               \
        seq_check() && save_if_result((cond))            \
    )                                                     \
)

#define seq_else else if(check_if() || seq_check())
#define seq_else_if(cond) seq_else seq_if(cond)

#define SEQ_RESTORE_VARS 0
#define SEQ_SAVE_VARS 1
#define seq_independent_memory for (__seq_mem_mode = 0; __seq_mem_mode < 2; ++__seq_mem_mode)

#define seqv(varname)                                                      \
    static varname;                                                         \
    seq_current_superthread->stack.place -= sizeof(varname);                 \
    if (__seq_mem_mode == SEQ_SAVE_VARS) {                                                     \
        memcpy(&seq_current_superthread->stack.data[seq_current_superthread->stack.place], &varname, sizeof(varname)); \
    } else if (__seq_mem_mode == SEQ_RESTORE_VARS) {                                             \
        memcpy(&varname, &seq_current_superthread->stack.data[seq_current_superthread->stack.place], sizeof(varname));   \
    }                                                                             \
    seq varname


void seq_set_current(SeqThread* t);

SeqThread seq_thread();

int seq_while_begin(void);

void seq_while_end(bool condition);
bool __seq_forward_cond(bool cond);
int seq_faux_check_thread(SeqSubthread* thread);

#define seq_loop_cond_cache(cond) (                  \
        seq_current_thread->loop_cond_cache == -1 ?   \
            seq_faux_check_thread(&seq_current_superthread->subthreads[seq_current_superthread->count-2])\
            && __seq_forward_cond(cond) :               \
            seq_current_thread->loop_cond_cache          \
    )

// bool __seq__while__condition;
// int __seq_dummy_i;

//TODO make unique for file too?
#define seq_while(condition)                                      \
        seq_while_begin();                                         \
        bool __seq__while__condition##__LINE__ = seq_loop_cond_cache((condition)); \
            for (int __seq_dummy_i##__LINE__=0;__seq_dummy_i##__LINE__!=1;__seq_dummy_i##__LINE__=1,   \
                seq_while_end(__seq__while__condition##__LINE__)                \
            ) if (__seq__while__condition##__LINE__)

#endif // SEQ_H_

#ifdef SEQ_IMPLEMENTATION

long long __seq_get_time_ns() {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1) return -1;
    return ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

SeqSubthread* seq_current_thread = NULL;
SeqThread* seq_current_superthread = NULL;

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


SeqSubthread seq_subthread() {
    SeqSubthread ret;
    ret.index   = 0;
    ret.counter = 1;
    ret.delay_start = -1;

    ret.if_cache_count = 0;
    ret.if_index = 0;
    for (size_t i = 0; i < MAX_IF_CACHE; ++i) {
        ret.if_cache_indexes[i] = -1;
        ret.if_cache_vals[i] = -1;
    }

    ret.loop_cond_cache = -1;

    return ret;
}


int __seq_mem_mode;
void seq_start() {
    seq_current_thread->index = 0;
    seq_current_thread->if_index = 0;

    seq_current_superthread->stack.place = SEQ_STACK_SIZE;
    if (__seq_mem_mode == SEQ_SAVE_VARS) {
        seq_current_thread->index = 100000000;
    }
}

void seq_sleep(double seconds) {
    SeqSubthread* t = seq_current_thread;
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

int seq_check_no_counter() {
    seq_current_thread->index += 1;
    if (seq_current_thread->index == seq_current_thread->counter) {
        return 1;
    }
    return 0;
}

void seq_reset() {
    if (seq_check()) {
        seq_current_thread->counter = 1;
        seq_current_thread->delay_start = -1;

        seq_current_thread->if_cache_count = 0;
        seq_current_thread->if_index = 0;
        for (size_t i = 0; i < MAX_IF_CACHE; ++i) {
            seq_current_thread->if_cache_indexes[i] = -1;
            seq_current_thread->if_cache_vals[i] = -1;
        }

        seq_current_thread->loop_cond_cache = -1;
    }
    seq_current_thread->index = 100000000;
}

void seq_reset_thread(SeqSubthread* t) {
    if (seq_check()) {
        t->counter = 1;
        t->delay_start = -1;

        t->if_cache_count = 0;
        t->if_index = 0;
        for (size_t i = 0; i < MAX_IF_CACHE; ++i) {
            t->if_cache_indexes[i] = -1;
            t->if_cache_vals[i] = -1;
        }

        t->loop_cond_cache = -1;
    }
    t->index = 100000000;
}

void seq_jump(int index) {
    seq_current_thread->index += 1;
    if (seq_current_thread->index == seq_current_thread->counter) { 
        seq_current_thread->counter = index;
    }
}

SeqSubthread* seq_last_subthread(SeqThread* t) {
    return &t->subthreads[t->count-1];
}

void seq_sync_with_other_subthread(SeqSubthread* t) {
    seq_current_thread->index += 1;
    t->index += 1;
    if (seq_current_thread->index == seq_current_thread->counter && t->index == t->counter) {
        seq_current_thread->counter += 1;
        t->counter += 1;
    }
}

void seq_sync_with_other(SeqThread* t) {
    seq_sync_with_other_subthread(seq_last_subthread(t));
}

void seq_wait_for_other_subthread(SeqSubthread* t) {
    seq_current_thread->index += 1;
    t->index += 1;
    if (seq_current_thread->index == seq_current_thread->counter) {
        if (t->counter == t->index) {
            seq_current_thread->counter += 1;
            t->counter += 1;
        } else if (t->counter > t->index) {
            seq_current_thread->counter += 1;
        } 
    }
}

void seq_wait_for_other(SeqThread* t) {
    seq_wait_for_other_subthread(seq_last_subthread(t));
}

void seq_sync_both(SeqThread* a, SeqThread* b) {
    SeqSubthread* aa = seq_last_subthread(a);
    SeqSubthread* bb = seq_last_subthread(b);
    aa->index += 1;
    bb->index += 1;
    if (aa->counter == aa->index && bb->counter == bb->index) { 
        aa->counter += 1;
        bb->counter += 1;
    }
}

void seq_sync_any(SeqThread* a, SeqThread* b) {
    SeqSubthread* aa = seq_last_subthread(a);
    SeqSubthread* bb = seq_last_subthread(b);
    aa->index += 1;
    bb->index += 1;
    if ((aa->index == aa->counter) || (bb->index == bb->counter)) { 
        aa->counter = aa->index + 1; 
        bb->counter = bb->index + 1; 
    }
}

void sleep_ms(long ms) {
    usleep(ms * 1000);
}


void seq_set_current(SeqThread* t) {
    seq_current_thread = &t->subthreads[t->count-1];
    seq_current_superthread = t;
}

SeqThread seq_thread() {
    SeqThread ret;
    for (int i = 0; i < MAX_SUBTHREADS; ++i)
        ret.subthreads[i] = seq_subthread();
    ret.count = 1;
    ret.stack.place = SEQ_STACK_SIZE;
    return ret;
}



int seq_while_begin(void) {
    seq_reset_thread(&seq_current_superthread->subthreads[seq_current_superthread->count]);
    seq_current_superthread->count += 1;
    seq_current_thread = &seq_current_superthread->subthreads[seq_current_superthread->count-1];
    seq_start();
    seq_wait_for_other_subthread(&seq_current_superthread->subthreads[seq_current_superthread->count-2]);
    return 1;
}

void seq_while_end(bool condition) {
    seq_if(condition) {
        seq_reset();
    }
    seq_current_superthread->count -= 1;
    seq_current_thread = &seq_current_superthread->subthreads[seq_current_superthread->count-1];
    seq_sync_with_other_subthread(&seq_current_superthread->subthreads[seq_current_superthread->count]);
}

bool __seq_forward_cond(bool cond) {
    seq_current_thread->loop_cond_cache = cond;
    return cond;
}
int seq_faux_check_thread(SeqSubthread* thread) {
    thread->index += 1;
    if (thread->index == thread->counter) {
        thread->index -= 1;
        return 1;
    }
    thread->index -= 1;
    return 0;
}

bool __seq__while__condition;
int __seq_dummy_i;

#endif // SEQ_IMPLEMENTATION
