/* 
MIT License
 
Copyright (c) 2025 Alejandro de Ugarriza Mohnblatt
 
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
 
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
 
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

---

Example usage:

>----------------------------------------------------<
#include <stdio.h>
#define SEQ_IMPLEMENTATION
#include "../seq.h"

int main() {
    SeqThread thread1 = seq_thread();
    SeqThread thread2 = seq_thread();

    #define T1 seq_current_thread = &thread1;
    #define T2 seq_current_thread = &thread2;

    while (1) {
        T1 seq_start();      T2 seq_start();
        T1 seq puts("bim");  T2 seq puts("three");
        T1 seq_sleep(2);     T2 seq_sleep(1);
        T1 seq puts("bam");  T2 seq puts("two");
        T1 seq_sleep(2);     T2 seq_sleep(1);
        T1 seq puts("bum");  T2 seq puts("one");
        seq_sync_both(&thread1, &thread2);
                T1 seq puts("BOOOOM!");
                T1 seq break;

        sequtil_mini_sleep();
    }
    return 0;
}
>----------------------------------------------------<

Options (#define them before including the library):

    SEQ_ENABLE_STACK  - Stack is turned off by default since it susbtantially
                        adds to the memory footprint of each thread.
    SEQ_STACK_SIZE    - Customize stack size, print it to see default value
    SEQ_ADD_USER_DATA - Extend SeqThread struct with custom user data under the 
                        field 'ud'. You must also define the type SeqUserData.

    SEQ_MANUAL_NONBLOCKING_STDIN 
                      - The included nonblocking IO functions (eg. seq_scanf)
                        set stdin to nonblocking every time they are called,
                        and then immediatly revert it back.  This is very
                        innefficient, so you can turn that functionality off
                        and manage it yourself with sequtil_set_stdin_blocking()
                        and sequtil_set_stdin_nonblocking().

    SEQ_NO_CONTROL_FLOW   - Don't include control flow constructs
                            (seq_if, seq_while, etc.).
    SEQ_NO_CYCLE_COUNTER  - Don't include cycle counter and the seq_miss_cycles
                            function. Saves a byte or two.
    SEQ_NO_NONBLOCKING_IO - Don't include nonblocking input functions
                            (seq_scanf, etc.).
    SEQ_NO_TIMING         - Don't include platform deppendent timing functions
                            (seq_get_time_ns, seq_sleep, etc.).

    SEQ_MINIMAL           - Disables everything except essential functionality.
                            Equivalent to #defining all SEQ_NO_... options. To
                            re-enable one or more options you can #define these:

    SEQ_ENABLE_CONTROL_FLOW
    SEQ_ENABLE_CYCLE_COUNTER
    SEQ_ENABLE_NONBLOCKING_IO_FUNCS
    SEQ_ENABLE_TIMING

 */


// TODO Nonblocking IO for all functions in stdlib
// TODO Timing for arduino and mac

#ifndef SEQ_H_
#define SEQ_H_

#ifdef SEQ_MINIMAL
#   define  SEQ_NO_CONTROL_FLOW
#   define  SEQ_NO_CYCLE_COUNTER
#   define  SEQ_NO_NONBLOCKING_IO_FUNCS
#   define  SEQ_NO_TIMING
#endif

#ifdef SEQ_ENABLE_CONTROL_FLOW
#   undef SEQ_NO_CONTROL_FLOW
#endif
#ifdef SEQ_ENABLE_CYCLE_COUNTER
#   undef SEQ_NO_CYCLE_COUNTER
#endif
#ifdef SEQ_ENABLE_NONBLOCKING_IO_FUNCS
#   undef SEQ_NO_NONBLOCKING_IO_FUNCS
#endif
#ifdef SEQ_ENABLE_TIMING
#   undef SEQ_NO_TIMING
#endif

#include <stdbool.h>
#ifndef SEQ_NO_NONBLOCKING_IO_FUNCS
#   include <stdarg.h>
#   include <stdio.h>
#endif
#ifndef SEQ_NO_TIMING
#   include <stdint.h>
#endif
#ifdef SEQ_ENABLE_STACK
#   include <string.h>
#endif

#ifdef __unix__
#   ifndef SEQ_NO_TIMING
#       include <unistd.h>
#       include <time.h>
#   endif
#   ifndef SEQ_NO_NONBLOCKING_IO_FUNCS
#       include <unistd.h>
#       include <errno.h>
#       include <fcntl.h>
#   endif
#endif

#ifdef _WIN32
#   if !defined(SEQ_NO_TIMING) || !defined(SEQ_NO_NONBLOCKING_IO_FUNCS)
#       include <windows.h>
#   endif
#   ifndef SEQ_NO_NONBLOCKING_IO_FUNCS
#       include <conio.h>
#   endif
#endif

#define SEQ_DISABLE (-100)

#ifdef SEQ_ENABLE_STACK
#   ifndef SEQ_STACK_SIZE
#       define SEQ_STACK_SIZE 1000
#   endif

    typedef struct {
        char data[SEQ_STACK_SIZE];
        size_t place;
    } SeqStack;

#   define SEQ_RESTORE_VARS          0
#   define SEQ_SAVE_VARS             1
#   define SEQ_NO_INDEPENDENT_MEMORY 2
#   define seq_independent_memory \
        for (seq_current_thread->mem_mode=0; seq_next_mem_mode(); ++seq_current_thread->mem_mode)
#else
#    define seq_independent_memory [ERROR] Define SEQ_ENABLE_STACK to use independent memory.
#endif


typedef struct {
    int index;
    int counter;

#if !defined(SEQ_NO_CYCLE_COUNTER) || defined(SEQ_ENABLE_STACK)
    int counter_bkp;
#endif

#ifndef SEQ_NO_TIMING
    int64_t delay_start;
#endif

#ifndef SEQ_NO_CYCLE_COUNTER
    unsigned int cycle_counter;
#endif

#ifndef SEQ_NO_CONTROL_FLOW
    signed char seq__do_else;
#endif

#ifdef SEQ_ENABLE_STACK
    SeqStack stack;
    int mem_mode;
#endif

#ifdef SEQ_ADD_USER_DATA
    SeqUserData ud;
#endif
} SeqThread;

SeqThread seq_thread();

void seq_start();
bool seq_check();
void seq_reset();

#ifndef SEQ_NO_TIMING
void seq_sleep(double seconds);
int64_t seq_get_time_ns(void);
#endif

int seq_label();
void seq_goto(int index);

#define seq_goto_if_not(dst, cond)                                \
    seq_current_thread->index += 1;                                \
    if (seq_current_thread->index == seq_current_thread->counter) { \
        if ((cond)) {                                                \
            seq_current_thread->counter++;                            \
        } else {                                                       \
            seq_current_thread->counter = dst;                          \
        }                                                                \
    }

void seq_sync_both(SeqThread* a, SeqThread* b);
void seq_sync_any(SeqThread* a, SeqThread* b);

#ifndef SEQ_NO_CYCLE_COUNTER
void seq_miss_cycles(unsigned int cycles);
#endif

#ifndef SEQ_NO_NONBLOCKING_IO_FUNCS
int sequtil_set_stdin_nonblocking();
int sequtil_set_stdin_blocking();
int seq_scanf(const char* fmt, ...);
#endif

#ifndef SEQ_NO_TIMING
int sequtil_msleep(long useconds);
int sequtil_usleep(long useconds);
int sequtil_mini_sleep();
#endif

#define seq if(seq_check())

#define seq_wait_until(cond) do {   \
    seq_current_thread->index++;     \
    if (seq_current_thread->index == seq_current_thread->counter && (cond)) \
        seq_current_thread->counter++; \
} while(0)

#define seq_wait_while(cond) do {   \
    seq_current_thread->index++;     \
    if (seq_current_thread->index == seq_current_thread->counter && !(cond)) \
        seq_current_thread->counter++; \
} while(0)


#ifndef SEQ_NO_CONTROL_FLOW
    #define seq_break seq_goto(seq__loop_out)
    #define seq_while(cond, ...)                   \
        do {                                        \
            int label = seq_current_thread->index+1; \
            static int seq__loop_out;                 \
            seq_goto_if_not(seq__loop_out, (cond));    \
                __VA_ARGS__                             \
            seq_goto(label);                             \
            seq__loop_out = seq_current_thread->index+1;  \
        } while(0);

    #define SEQ__COMBINE1(X,Y) X##Y  // helper
    #define SEQ__COMBINE(X,Y) SEQ__COMBINE1(X,Y)

    #define seq_if(cond, ...)                                           \
        static int SEQ__COMBINE(seq__elze, __LINE__);                    \
        if (seq__do_else == -1) seq_current_thread->seq__do_else = true;  \
        else seq__do_else = true;                                          \
        seq_goto_if_not(SEQ__COMBINE(seq__elze, __LINE__), (cond));         \
            do {                                                             \
                static signed char seq__do_else;                              \
                __VA_ARGS__                                                    \
            } while(0);                                                         \
            seq {                                                                \
                if (seq__do_else == -1) seq_current_thread->seq__do_else = false; \
                else seq__do_else = false;                                         \
            }                                                                       \
        SEQ__COMBINE(seq__elze, __LINE__) = seq_current_thread->index + 1;

    #define seq_else_if(cond, ...)                                                        \
        static int SEQ__COMBINE(seq__elze, __LINE__);                                      \
        seq_goto_if_not(                                                                    \
            SEQ__COMBINE(seq__elze, __LINE__),                                               \
            (seq__do_else == -1 ? seq_current_thread->seq__do_else : seq__do_else) && (cond)  \
        );                                                                                     \
            do {                                                                                \
                static signed char seq__do_else;                                                 \
                __VA_ARGS__                                                                       \
            } while(0);                                                                            \
            seq {                                                                                   \
                if (seq__do_else == -1) seq_current_thread->seq__do_else = false;                    \
                else seq__do_else = false;                                                            \
            }                                                                                          \
        SEQ__COMBINE(seq__elze, __LINE__) = seq_current_thread->index + 1;

    #define seq_elif seq_else_if

    #define seq_else(...)                                                      \
        static int SEQ__COMBINE(seq__elze, __LINE__);                           \
        seq_goto_if_not(                                                         \
            SEQ__COMBINE(seq__elze, __LINE__),                                    \
            (seq__do_else == -1 ? seq_current_thread->seq__do_else : seq__do_else) \
        );                                                                          \
            do {                                                                     \
                static signed char seq__do_else;                                      \
                __VA_ARGS__                                                            \
            } while(0);                                                                 \
        SEQ__COMBINE(seq__elze, __LINE__) = seq_current_thread->index + 1;

    #define seq_for(vardef, cond, increment, ...) \
        vardef;                                    \
        seq_while(cond,                             \
            __VA_ARGS__                              \
            seq {increment;}                          \
        )

#endif /* ndef SEQ_NO_CONTROL_FLOW*/

#ifdef SEQ_ENABLE_STACK
    #define seq_load_into_stack(thread_ptr, variable) do { \
        (thread_ptr)->stack.place -= sizeof(variable);      \
        memcpy(&(thread_ptr)->stack.data[(thread_ptr)->stack.place], &variable, sizeof(variable));\
    } while(0)

    #define seqin(varname) \
        static varname;     \
        do {                 \
            seq_current_thread->stack.place -= sizeof(varname);\
            seq memcpy(&varname, &seq_current_thread->stack.data[seq_current_thread->stack.place], sizeof(varname));\
        } while(0)

    #define seq_register(varname) do {                                                        \
        if (seq_current_thread->mem_mode == SEQ_NO_INDEPENDENT_MEMORY) break;                  \
        seq_current_thread->stack.place -= sizeof(varname);                                     \
        if (seq_current_thread->counter == 1 || seq_current_thread->mem_mode == SEQ_SAVE_VARS) { \
            memcpy(&seq_current_thread->stack.data[seq_current_thread->stack.place], &varname, sizeof(varname));\
        } else if (seq_current_thread->mem_mode == SEQ_RESTORE_VARS) {                             \
            memcpy(&varname, &seq_current_thread->stack.data[seq_current_thread->stack.place], sizeof(varname));\
        }                                                                                            \
    } while(0)

    #define seqv(varname)                                              \
        static varname;                                                 \
        if (seq_current_thread->mem_mode != SEQ_NO_INDEPENDENT_MEMORY) { \
            seq_current_thread->stack.place -= sizeof(varname);           \
            if (seq_current_thread->mem_mode == SEQ_SAVE_VARS) {           \
                memcpy(&seq_current_thread->stack.data[seq_current_thread->stack.place], &varname, sizeof(varname));\
            } else if (seq_current_thread->mem_mode == SEQ_RESTORE_VARS) {   \
                memcpy(&varname, &seq_current_thread->stack.data[seq_current_thread->stack.place], sizeof(varname));\
            }                                                                  \
        }                                                                       \
        seq varname
    
    #define seqp(varname)                                              \
        static *varname;                                                \
        if (seq_current_thread->mem_mode != SEQ_NO_INDEPENDENT_MEMORY) { \
            seq_current_thread->stack.place -= sizeof(varname);           \
            if (seq_current_thread->mem_mode == SEQ_SAVE_VARS) {           \
                memcpy(&seq_current_thread->stack.data[seq_current_thread->stack.place], &varname, sizeof(varname));\
            } else if (seq_current_thread->mem_mode == SEQ_RESTORE_VARS) {   \
                memcpy(&varname, &seq_current_thread->stack.data[seq_current_thread->stack.place], sizeof(varname));\
            }                                                                  \
        }                                                                       \
        seq varname
    
    #define seqarr(varname, count, ...)     \
        static varname[count] = __VA_ARGS__; \
        seq_register(varname)
#else
    #define seqv(varname) static varname; seq varname
    #define seqp(varname) static *varname; seq varname
    #define seqarr(varname, count, ...) static varname[count] = __VA_ARGS__
#endif // SEQ_ENABLE_STACK


#endif // SEQ_H_

#ifdef SEQ_IMPLEMENTATION

SeqThread* seq_current_thread;

/* dummy value to indicate to control flow code that it's running at the top level */
#ifndef SEQ_NO_CONTROL_FLOW
static signed char seq__do_else = -1; 
#endif

#ifdef SEQ_ENABLE_STACK
    bool seq_next_mem_mode() {
        if (seq_current_thread->mem_mode >= 2) {
            seq_current_thread->counter = seq_current_thread->counter_bkp;
            return false;
        }
        return true;
    }
#endif

SeqThread seq_thread() {
    SeqThread ret;
    ret.index   = 0;
    ret.counter = 1;
#if !defined(SEQ_NO_CYCLE_COUNTER) || defined(SEQ_ENABLE_STACK)
    ret.counter_bkp = 1;
#endif
#ifndef SEQ_NO_TIMING
    ret.delay_start = -1;
#endif
#ifndef SEQ_NO_CYCLE_COUNTER
    ret.cycle_counter = 0;
#endif
#ifndef SEQ_NO_CONTROL_FLOW
    ret.seq__do_else = 0;
#endif
#ifdef SEQ_ENABLE_STACK
    ret.mem_mode = SEQ_NO_INDEPENDENT_MEMORY;
    ret.stack.place = SEQ_STACK_SIZE;
#endif
    return ret;
}

void seq_start() {
    // Miss a single cycle so that all labels have values before beggining the program
    seq_current_thread->index = 2;
    if (seq_current_thread->counter == 1
    ||  seq_current_thread->counter == 2)
        seq_current_thread->counter += 1;

#ifdef SEQ_ENABLE_STACK
    seq_current_thread->stack.place = SEQ_STACK_SIZE;
    if (seq_current_thread->mem_mode == SEQ_SAVE_VARS) {
        seq_current_thread->counter_bkp = seq_current_thread->counter;
        seq_current_thread->counter = SEQ_DISABLE;
    } else if (seq_current_thread->mem_mode == SEQ_RESTORE_VARS) {
         seq_current_thread->counter = seq_current_thread->counter_bkp;
    }
#endif
#ifdef SEQ_ON_START
    SEQ_ON_START
#endif
}

bool seq_check() {
    seq_current_thread->index++;
    if (seq_current_thread->index == seq_current_thread->counter) {
        seq_current_thread->counter += 1;
        return true;
    }
    return false;
}

void seq_reset() {
    if (seq_check()) {
        seq_current_thread->counter = 2;
        #ifndef SEQ_NO_TIMING
        seq_current_thread->delay_start = -1;
        #endif
    }
}

void seq_always_reset() {
    seq_current_thread->counter = 2;
    #ifndef SEQ_NO_TIMING
    seq_current_thread->delay_start = -1;
    #endif
}

int seq_label() {
    return seq_current_thread->index+1;
}

void seq_goto(int index) {
    seq_current_thread->index += 1;
    if (seq_current_thread->index == seq_current_thread->counter) { 
        seq_current_thread->counter = index;
    }
}

#ifndef SEQ_NO_TIMING
void seq_sleep(double seconds) {
    SeqThread* t = seq_current_thread;
    t->index += 1;
    if (t->index == t->counter) { 
        int64_t nanoseconds = (seconds * 1000.0f * 1000.0f * 1000.0f);
        if (t->delay_start < 0) { 
            t->delay_start = seq_get_time_ns();
            // printf("started delay at %f seconds\n", t->delay_start/(1000.0f * 1000.0f * 1000.0f));
        }
        if (seq_get_time_ns() - t->delay_start > nanoseconds) {
            t->delay_start = -1;
            t->counter += 1;
        }
    }
}
#endif

#if defined(SEQ_CUSTOM_TIME) || defined(SEQ_NO_TIMING) 
/* Provided by user. */
#elif defined(_WIN32)
int64_t seq_get_time_ns() {
    static LARGE_INTEGER frequency;
    static bool initialized = false;
    if (!initialized) {
        QueryPerformanceFrequency(&frequency); 
        initialized = true;
    }

    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    uint64_t time = counter.QuadPart;
    time /= frequency.QuadPart;
    time *= 1000000000;
    return (int64_t)time;
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
    return -1;
}

#else
#   error\
    seq_get_time_ns() not implemented for this platform.\
    Define the SEQ_CUSTOM_TIME macro and provide your own implementation of the seq_get_time_ns() function.
#endif

#ifndef SEQ_NO_CYCLE_COUNTER
void seq_miss_cycles(unsigned int cycles) {
#ifdef SEQ_ENABLE_STACK
    if (seq_current_thread->mem_mode == SEQ_SAVE_VARS) return;
#endif
    seq_current_thread->cycle_counter++;
    if (seq_current_thread->cycle_counter < cycles) {
        if (seq_current_thread->counter != SEQ_DISABLE) {
            seq_current_thread->counter_bkp = seq_current_thread->counter;
            seq_current_thread->counter = SEQ_DISABLE;
        }
    } else {
        seq_current_thread->counter = seq_current_thread->counter_bkp;
        seq_current_thread->cycle_counter = 0;
    }
}
#endif /* ndef SEQ_NO_CYCLE_COUNTER */

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

void seq_sync_any_of_many(SeqThread* ts, size_t count) {
    for (int i = 0; i < count; ++i)  ts[i].index += 1;
    for (int i = 0; i < count; ++i) {
        if (ts[i].index == ts[i].counter) {
            for (int j = 0; j < count; ++j) ts[i].counter = ts[i].index+1;
            return;
        }
    }
}

void seq_sync_all(SeqThread* ts, size_t count) {
    for (int i = 0; i < count; ++i) ts[i].index += 1;
    for (int i = 0; i < count; ++i) {
        if (ts[i].index != ts[i].counter) return;
    }
    for (int i = 0; i < count; ++i) ts[i].counter += 1;
}

#ifdef SEQ_NO_NONBLOCKING_IO_FUNCS
/* do nothing */
#elif defined(__unix__)

int sequtil_set_stdin_nonblocking() {
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (flags == -1) return 1;
    if (fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK) == -1) return 1;
    return 0;
}

int sequtil_set_stdin_blocking() {
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (flags == -1) return 1;
    if (fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK) == -1) return 1;
    return 0;
}

int seq_scanf(const char* fmt, ...) {
    static int ret;

    seq_current_thread->index += 1;
    if (seq_current_thread->index == seq_current_thread->counter) {
        #ifndef SEQ_MANUAL_NONBLOCKING_STDIN
        int flags = fcntl(STDIN_FILENO, F_GETFL, 0);

        if (flags == -1) goto on_error;
        if (fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK) == -1) goto on_error;
        #endif

        va_list args;
        va_start(args, fmt);
        int temp_ret = vscanf(fmt, args);
        va_end(args);

        if (!(temp_ret < 0 && errno == EAGAIN)) {
            seq_current_thread->counter += 1;
            ret = temp_ret;
        }

        #ifndef SEQ_MANUAL_NONBLOCKING_STDIN
        if (fcntl(STDIN_FILENO, F_SETFL, flags) == -1) goto on_error;
        #endif
    }
    return ret;

#ifndef SEQ_MANUAL_NONBLOCKING_STDIN
on_error:
    ret = -1;
    return ret;
#endif
}

#elif defined(_WIN32)

int sequtil_set_stdin_nonblocking() { return 0; }
int sequtil_set_stdin_blocking() { return 0; }

int seq_scanf(const char* fmt, ...) {
    static int ret;
    seq_current_thread->index += 1;
    if (seq_current_thread->index == seq_current_thread->counter) {
        #define SEQ_INPUT_EVENT_BUF_SIZE 128
        INPUT_RECORD event_buffer[SEQ_INPUT_EVENT_BUF_SIZE];

        static DWORD event_count, already_read;
        HANDLE std_input = GetStdHandle(STD_INPUT_HANDLE);
        PeekConsoleInput(std_input, event_buffer, SEQ_INPUT_EVENT_BUF_SIZE, &event_count);

        static char strbuff[1024]; //TODO: I'm not sure what to do about this
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

#ifndef SEQ_NO_TIMING
int sequtil_usleep(long useconds) {
    #ifdef __unix__
        return usleep(useconds);
    #else
        Sleep(useconds / 1000);
        return 0;
    #endif
}

int sequtil_msleep(long mseconds) {
    #ifdef __unix__
        puts("hi");
        return usleep(mseconds*1000);
    #else
        Sleep(mseconds);
        return 0;
    #endif
}

int sequtil_mini_sleep() {
    #ifdef __unix__
        return usleep(1);
    #else
        Sleep(1);
        return 0;
    #endif
}
#endif

#endif // SEQ_IMPLEMENTATION
