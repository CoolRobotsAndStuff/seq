#ifndef SEQ_H_
#define SEQ_H_

#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <limits.h>

// TODO Independent memory across threads
// TODO Lazy ifs and loops
// TODO Async IO
// TODO prevent_busyloop() function
// TODO crossplatform timing


#define SEQ_DISABLE INT_MIN

#define SEQ_STACK_SIZE 1000
typedef struct {
    char data[SEQ_STACK_SIZE];
    size_t place;
} SeqStack;

#define SEQ_RESTORE_VARS          0
#define SEQ_SAVE_VARS             1
#define SEQ_NO_INDEPENDENT_MEMORY 2

int seq_init_independent_memory();

#define seq_independent_memory for (seq_current_thread->mem_mode = seq_init_independent_memory(); seq_current_thread->mem_mode < 2; seq_init_independent_memory(), ++seq_current_thread->mem_mode)

long long __seq_get_time_ns();

#ifndef seq_get_time_ns

#define seq_get_time_ns __seq_get_time_ns 

#endif //ndef seq_get_time_ns


typedef struct {
    int index;
    int counter;
    int counter_bkp;
    char do_else;
    bool missed;
    long long delay_start;
    SeqStack stack;
    int mem_mode;
} SeqThread;

SeqThread seq_thread() {
    SeqThread ret;
    ret.index   = 0;
    ret.counter = 1;
    ret.counter_bkp = 1;
    ret.do_else = 0;
    ret.delay_start = -1;
    ret.missed = false;
    ret.mem_mode = SEQ_NO_INDEPENDENT_MEMORY;;
    ret.stack.place = SEQ_STACK_SIZE;
    return ret;
}

void seq_miss_cicle();

void seq_sleep(double seconds);

void seq_start();

int seq_check();

void seq_reset();

void seq_goto_index(int index);

void seq_goto_index_if_positive(int index);
bool seq_goto_index_if_not(int index, bool cond);

void seq_sync_both(SeqThread* a, SeqThread* b);
void seq_sync_any(SeqThread* a, SeqThread* b);
void seq_wait_for(SeqThread* t);

#define seq if(seq_check())

#define seq_break seq_goto_index(out)

#define seq_while(cond, ...)                   \
    do {                                        \
        int label = seq_current_thread->index+1; \
        static int out;                           \
        seq_goto_index_if_not(out, (cond));        \
            __VA_ARGS__                             \
        seq_goto_index(label);                       \
        out = seq_current_thread->index+1;            \
    } while(0);

#define COMBINE1(X,Y) X##Y  // helper
#define COMBINE(X,Y) COMBINE1(X,Y)

#define seq_if(cond, ...)                                  \
    static int COMBINE(elze, __LINE__);                     \
    seq do_else = true;                                      \
    seq_goto_index_if_not(COMBINE(elze, __LINE__), (cond));   \
        do {                                                   \
            static char do_else;                                \
            __VA_ARGS__                                          \
        } while(0);                                               \
        seq {                                                      \
            if (do_else == -1) seq_current_thread->do_else = false; \
            else do_else = false;                                    \
        }                                                             \
    COMBINE(elze, __LINE__) = seq_current_thread->index + 1;

#define seq_else_if(cond, ...)                                                            \
    static int COMBINE(elze, __LINE__);                                                    \
    seq_goto_index_if_not(COMBINE(elze, __LINE__),                                          \
                          (do_else == -1 ? seq_current_thread->do_else : do_else) && (cond));\
        do {                                                                                  \
            static char do_else;                                                               \
            __VA_ARGS__                                                                         \
        } while(0);                                                                              \
        seq {                                                                                     \
            if (do_else == -1) seq_current_thread->do_else = false;                                \
            else do_else = false;                                                                   \
        }                                                                                            \
    COMBINE(elze, __LINE__) = seq_current_thread->index + 1;

#define seq_else(...)                                                                                     \
    static int COMBINE(elze, __LINE__);                                                                    \
    seq_goto_index_if_not(COMBINE(elze, __LINE__), (do_else == -1 ? seq_current_thread->do_else : do_else));\
        do {                                                                                                 \
            static char do_else;                                                                              \
            __VA_ARGS__                                                                                        \
        } while(0);                                                                                             \
    COMBINE(elze, __LINE__) = seq_current_thread->index + 1;

#define seq_load_into_stack(thread_ptr, variable) \
        do {\
            (thread_ptr)->stack.place -= sizeof(variable);\
            memcpy(&(thread_ptr)->stack.data[(thread_ptr)->stack.place], &variable, sizeof(variable));\
        } while(0);

#define seqv(varname)                                                                                      \
    static varname;                                                                                         \
    if (seq_current_thread->mem_mode != SEQ_NO_INDEPENDENT_MEMORY) {                                         \
        seq_current_thread->stack.place -= sizeof(varname);                                                   \
        if (seq_current_thread->mem_mode == SEQ_SAVE_VARS) {                                                   \
            memcpy(&seq_current_thread->stack.data[seq_current_thread->stack.place], &varname, sizeof(varname));\
        } else if (seq_current_thread->mem_mode == SEQ_RESTORE_VARS) {                                           \
            memcpy(&varname, &seq_current_thread->stack.data[seq_current_thread->stack.place], sizeof(varname));  \
        }                                                                                                          \
    }                                                                                                               \
    seq varname

#define seq_for(vardef, cond, increment, ...) \
vardef;                                        \
seq_while(cond,                                 \
    __VA_ARGS__                                  \
    seq {increment;}                              \
)

#endif // SEQ_H_

#ifdef SEQ_IMPLEMENTATION

long long __seq_get_time_ns() {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1) return -1;
    return ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

SeqThread* seq_current_thread;

static char do_else = -1;

void seq_miss_cicle() {
    seq_current_thread->index += 1;
    if (seq_current_thread->index == seq_current_thread->counter) { 
        if (seq_current_thread->missed) seq_current_thread->counter += 1;
        seq_current_thread->missed = !seq_current_thread->missed;
    }
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

void seq_start() { 
    seq_current_thread->index = 0;

    seq_current_thread->stack.place = SEQ_STACK_SIZE;
    if (seq_current_thread->mem_mode == SEQ_SAVE_VARS) {
        seq_current_thread->counter_bkp = seq_current_thread->counter;
        seq_current_thread->counter = SEQ_DISABLE;
    } else if (seq_current_thread->mem_mode == SEQ_RESTORE_VARS) {
         seq_current_thread->counter = seq_current_thread->counter_bkp;
    }

    seq_miss_cicle();
}

int seq_check() {
    seq_current_thread->index += 1;
    if (seq_current_thread->index == seq_current_thread->counter) {
        seq_current_thread->counter += 1;
        return 1;
    }
    return 0;
}

void seq_reset() {
    if (seq_check()) {
        seq_current_thread->counter = 1;
        seq_current_thread->delay_start = -1;
    }
}

void seq_goto_index(int index) {
    seq_current_thread->index += 1;
    if (seq_current_thread->index == seq_current_thread->counter) { 
        seq_current_thread->counter = index;
    }
}

void seq_goto_index_if_positive(int index) {
    seq_current_thread->index += 1;
    if (seq_current_thread->index == seq_current_thread->counter) { 
        if (index < 0) {
            seq_current_thread->counter++;
        } else {
            seq_current_thread->counter = index;
        }
    }
}

bool seq_goto_index_if_not(int index, bool cond) {
    seq_current_thread->index += 1;
    if (seq_current_thread->index == seq_current_thread->counter) { 
        if (cond) {
            seq_current_thread->counter++;
        } else {
            seq_current_thread->counter = index;
        }
    }
    return true;
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

void seq_wait_for(SeqThread* t) {
    seq_current_thread->index += 1;
    t->index += 1;
    if (seq_current_thread->index == seq_current_thread->counter) {
        printf("counter: %d\n", t->counter);
        printf("index: %d\n", t->index);
        if (t->counter == t->index) {
            seq_current_thread->counter += 1;
            puts("advancing");
            t->counter += 1;
        } else if (t->counter > t->index) {
            seq_current_thread->counter += 1;
            puts("advancing2");
        } 
    }
}

int seq_init_independent_memory() {
    // seq_current_thread->stack.place = SEQ_STACK_SIZE;
    // if (seq_current_thread->mem_mode == SEQ_SAVE_VARS) {
    //     seq_current_thread->counter = SEQ_DISABLE;
    // }
    return 0;
}

#endif // SEQ_IMPLEMENTATION
