#ifndef SEQ_H_
#define SEQ_H_

#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>

#if defined(_WIN32)
#include <windows.h>
#include <conio.h>
#endif

// TODO Async IO
// TODO Option to disable control flow and independent memory
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

#define seq_independent_memory \
    for (seq_current_thread->mem_mode=0; seq_current_thread->mem_mode < 2; ++seq_current_thread->mem_mode)


int64_t seq_get_time_ns(void);


typedef struct {
    int index;
    int counter;
    int counter_bkp;
    char do_else;
    bool missed;
    int64_t delay_start;
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

void seq_goto(int index);
int seq_label();

void seq_sync_both(SeqThread* a, SeqThread* b);
void seq_sync_any(SeqThread* a, SeqThread* b);

int seq_scanf(const char* fmt, ...);

#define seq if(seq_check())

#define seq_break seq_goto(out)

#define seq_while(cond, ...)                   \
    do {                                        \
        int label = seq_current_thread->index+1; \
        static int out;                           \
        seq_goto_if_not(out, (cond));              \
            __VA_ARGS__                             \
        seq_goto(label);                             \
        out = seq_current_thread->index+1;            \
    } while(0);

#define COMBINE1(X,Y) X##Y  // helper
#define COMBINE(X,Y) COMBINE1(X,Y)

#define seq_if(cond, ...)                                  \
    static int COMBINE(elze, __LINE__);                     \
    seq do_else = true;                                      \
    seq_goto_if_not(COMBINE(elze, __LINE__), (cond));         \
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
    seq_goto_if_not(COMBINE(elze, __LINE__),                                                \
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

#define seq_else(...)                                                                                \
    static int COMBINE(elze, __LINE__);                                                               \
    seq_goto_if_not(COMBINE(elze, __LINE__), (do_else == -1 ? seq_current_thread->do_else : do_else)); \
        do {                                                                                            \
            static char do_else;                                                                         \
            __VA_ARGS__                                                                                   \
        } while(0);                                                                                        \
    COMBINE(elze, __LINE__) = seq_current_thread->index + 1;

#define seq_load_into_stack(thread_ptr, variable)        \
        do {                                              \
            (thread_ptr)->stack.place -= sizeof(variable); \
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

#define seq_goto_if_not(dst, cond)                                \
    seq_current_thread->index += 1;                                \
    if (seq_current_thread->index == seq_current_thread->counter) { \
        if ((cond)) {                                                \
            seq_current_thread->counter++;                            \
        } else {                                                       \
            seq_current_thread->counter = dst;                          \
        }                                                                \
    }
#endif // SEQ_H_

#ifdef SEQ_IMPLEMENTATION

#ifdef SEQ_CUSTOM_TIME
/* Provided by user. */
#elif defined(_WIN32)
int64_t seq_get_time_ns() {
    static LARGE_INTEGER frequency;
    static bool initialized = false;
    QueryPerformanceFrequency(&frequency); 
    initialized = true;

    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    counter.QuadPart *= 1000000000;
    counter.QuadPart /= frequency.QuadPart;
    return counter.QuadPart;
}

#elif defined(__unix__)
int64_t seq_get_time_ns(void) {
    struct timespec ts;
    #ifdef CLOCK_MONOTONIC
        if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
            return ts.tv_sec * 1000000000LL + ts.tv_nsec;
        }
    #endif
    #ifdef CLOCK_REALTIME
        if (clock_gettime(CLOCK_REALTIME, &ts) == 0) {
            return ts.tv_sec * 1000000000LL + ts.tv_nsec;
        }
    #endif
    puts("Error: seq_get_time_ns");
    exit(1);
}

#else
    #error\
    seq_get_time_ns() not implemented for this platform.\
    Define the SEQ_CUSTOM_TIME macro and provide your own implementation of the seq_get_time_ns() function.
#endif

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
        int64_t nanoseconds = (seconds * 1000.0f * 1000.0f * 1000.0f);
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

void seq_always_reset() {
    seq_current_thread->counter = 1;
    seq_current_thread->delay_start = -1;
}

void seq_goto(int index) {
    seq_current_thread->index += 1;
    if (seq_current_thread->index == seq_current_thread->counter) { 
        seq_current_thread->counter = index;
    }
}
int seq_label() {
    return seq_current_thread->index+1;
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

#if defined(__unix__)

int seq_scanf(const char* fmt, ...) {
    static int ret;

    // non-blocking stdin
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);

    if (flags == -1) goto on_error;
    if (fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK) == -1) goto on_error;

    seq_current_thread->index += 1;
    if (seq_current_thread->index == seq_current_thread->counter) {
        va_list args;
        va_start(args, fmt);
        int temp_ret = vscanf(fmt, args);
        va_end(args);

        if (!(temp_ret < 0 && errno == EAGAIN)) {
            seq_current_thread->counter += 1;
            ret = temp_ret;
        }
    }
    if (fcntl(STDIN_FILENO, F_SETFL, flags) == -1) goto on_error;
    return ret;

on_error:
    ret = -1;
    return ret;
}

#elif defined(_WIN32)

int seq_scanf(const char* fmt, ...) {
    static int ret;
    seq_current_thread->index += 1;
    if (seq_current_thread->index == seq_current_thread->counter) {
        #define SEQ_INPUT_EVENT_BUF_SIZE 128
        INPUT_RECORD event_buffer[SEQ_INPUT_EVENT_BUF_SIZE];

        static DWORD event_count, already_read;
        HANDLE std_input = GetStdHandle(STD_INPUT_HANDLE);
        PeekConsoleInput(std_input, event_buffer, SEQ_INPUT_EVENT_BUF_SIZE, &event_count);

        static char strbuff[1024];
        static int strbuff_index = 0;

        for (int i = already_read; i < event_count; ++i) {
            if (event_buffer[i].EventType == KEY_EVENT && event_buffer[i].Event.KeyEvent.bKeyDown) {
                char c = event_buffer[i].Event.KeyEvent.uChar.AsciiChar;
                switch (c) {
                case 0: break;
                case '\r':
                    strbuff[strbuff_index] = '\0';
                    printf("\r\n");

                    va_list args;
                    va_start(args, fmt);
                        ret = vsscanf(strbuff, fmt, args);
                    va_end(args);

                    strbuff_index = 0;
                    already_read  = 0;
                    if (ret != 0) {
                        FlushConsoleInputBuffer(std_input);
                    }
                    seq_current_thread->counter++;
                    return ret;

                case '\b':
                    strbuff_index--;
                    printf("\b \b");
                    break;

                default: 
                    strbuff[strbuff_index++] = c;
                    putchar(c);
                }
            }
        }
        already_read = event_count;
    }
    return ret;
}

#endif

#endif // SEQ_IMPLEMENTATION
