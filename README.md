## Seq - Flashy single-header Concurrency Library in C

> This library is in beta, the api could change at any time, if you use this in production YOUR COMPUTER WILL LITERALLY FUCKING EXPLO... but if u just wanna try it dat's fine ```:)```

### Usage

Download the ```seq.h``` file and put it in the same directory as you code.

```c
#include <stdio.h>
#define SEQ_IMPLEMENTATION
#include "seq.h"

int main() {
    SeqThread thread1 = seq_thread();
    SeqThread thread2 = seq_thread();

    #define T1 seq_current_thread = &thread1;
    #define T2 seq_current_thread = &thread2;

    while (1) {
        T1 seq_start();      T2 seq_start();
/*             |                    |                  */
        T1 seq puts("bim");  T2 seq puts("three");
/*             |                    |                  */
        T1 seq_sleep(2);     T2 seq_sleep(1);
/*             |                    |                  */
        T1 seq puts("bam");  T2 seq puts("two");
/*             |                    |                  */
        T1 seq_sleep(2);     T2 seq_sleep(1);
/*             |                    |                  */
        T1 seq puts("bum");  T2 seq puts("one");
/*              \                  /                   */
         seq_sync_both(&thread1, &thread2);
/*                       |                             */
                T1 seq puts("BOOOOM!");
                T1 seq break;
    }
    return 0;
}
```

Simpler example:

```c
#include <stdio.h>
#define SEQ_IMPLEMENTATION
#include "seq.h"

int main() {
    SeqThread t = seq_thread();
    seq_current_thread = &t;
    while (1) {
        seq puts("hello");
        seq_sleep(1);
        seq puts("world");
        seq break;
    }
    return 0;
}
```
Check ```examples``` folder for more examples.

Currently timing (seq_sleep, etc.) and some other utilities are only implemented for Windows and Linux. To compile to other platforms you must define the ```SEQ_CUSTOM_TIME``` macro and an implementation for the ```seq_get_time_ns``` function. For example, a possible UNIX implementation:

```c
#define SEQ_CUSTOM_TIME
#define SEQ_IMPLEMENTATION
#include "seq.h"

int64_t seq_get_time_ns(void) {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
        return ts.tv_sec * 1000000000LL + ts.tv_nsec;
    }
    puts("Error: seq_get_time_ns");
    return -1;
}

int main() {
    // ...
}
```

### Running examples

There is a convenient system to compile, run and debug examples on linux. Go to the root of this repo and bootstrap it with:
```bash
cc ./nob.c -o ./nob
```
Now you can easily compile and run a single example:

```./nob examples/primes run```

or all examples:

```./nob examples run```

and you can crosscompile to Windows:

```./nob examples/primes run windows```

plus some other stuff. To see more options do

```./nob```

### Inner workings

The concept itself is quite simple, and it's reminiscent of [protothreads](https://dunkels.com/adam/pt/). It can be boiled down to:

```c
#include <stdio.h>
#include <stdbool.h>

int index;
int counter;

int main() {
    long cycle_count = 0;

    counter = 1;
    while (true) {
        index = 0; 

        index++;
        if (index == counter) {
            counter++;
            puts("Do something");
        }

        index++;
        if (index == counter && cycle_count > 1000000000) {
            cycle_count = 0;
            counter++; // wait till counter hits n cycles
        }

        index++;
        if (index == counter) {
            counter++;
            puts("Do something else");
        }

        index++;
        if (index == counter) {
            counter++;
            counter = 1;
        }

        cycle_count++;

        // Non sequential code can go here
    }
}
```
You can easily imagine substituting cycle_count for a timer. Add a bit of syntax sugar:

```c
#include <stdio.h>
#include <stdbool.h>

int index;
int counter;

bool seq_check() {
    index++;
    if (index == counter) {
        counter++;
        return true;
    }
    return false;
}

#define seq if (seq_check())

bool seq_wait_until(bool cond) {
    index++;
    if (index == counter && cond) {
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

    counter = 1;
    while (true) {
        index = 0; 

        seq puts("Do something");
        seq_wait_until(cycle_count > 1000000000);
        seq cycle_count = 0;
        seq puts("Do something else");
        seq_reset();

        cycle_count++;
        // Non-sequential code can go here
    }
}
```

And you're basically there. Read the source code if you wanna know more.
