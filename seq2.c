#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

// TODO Independent memory across threads
// TODO Lazy ifs and loops
// TODO Async IO
// TODO prevent_busyloop() function
// TODO crossplatform timing


typedef struct {
    int index;
    int counter;
    char do_else;
    bool missed;
    long long delay_start;
} SeqThread;

SeqThread seq_thread() {
    SeqThread ret;
    ret.index   = 0;
    ret.counter = 1;
    ret.do_else = 0;
    ret.delay_start = -1;
    ret.missed = false;
    return ret;
}

SeqThread* seq_current_thread;

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

static char do_else = -1;

#define seqv(varname) static varname; seq varname

#define seq_for(vardef, cond, increment, ...) \
vardef;                                        \
seq_while(cond,                                 \
    __VA_ARGS__                                  \
    seq {increment;}                              \
)

int main(void) {
    SeqThread thread1 = seq_thread();
    SeqThread thread2 = seq_thread();

    #define T1 seq_current_thread = &thread1;
    #define T2 seq_current_thread = &thread2;

    while (1) {
        T1 seq_start();

        seq puts("bim");

        // int label_0 = seq_current_thread->index+1;
        // static int out; seq_goto_index_if_not(out, j < 3);
        
        seq_for(int seqv(j) = 0, j < 3, ++j,
            seq puts("bam");
            seq puts("bum");
            seq_for(int seqv(k) = 0, k < 2, ++k,
                seq printf("hello %d\n", k);
            )
        )

        
        static int x = 4;
        seq_if(x > 0,
            seq puts("x positive!");
        ) 

        static int y = 0;
        seq_if(y > 0,
            seq puts("y positive!");
        ) seq_else_if (y%2 == 0,
            seq_if (y == 0,
                seq puts("y is zero!");
            ) seq_else (
                seq puts("y negative and even!");
            )
        ) seq_else (
            seq puts("wiii!");
        )
        
        T2 seq_start();
        seq puts("Hello from thread 2");
        static int z = -4;
        seq_if(z > 0,
            seq puts("z positive!");
        ) seq_else_if (z%2 == 0,
            seq_if (z == 0,
                seq puts("z is zero!");
            ) seq_else (
                seq puts("z negative and even!");
            )
        ) seq_else (
            seq puts("wiii!");
        )


        sleep(1);
    }
}
