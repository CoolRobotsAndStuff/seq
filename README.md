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

        sequtil_mini_sleep(); /* Prevent busylooping. */
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
Also check the ```examples``` folder.

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

### About

Seq is a concurrency library for scheduling tasks with delays in a program where there is also code that should run constantly on a loop. You can quickly slap it on a project that needs concurrency without worrying too much about it, and the concept itself can be implemented in any language.

It can be either stackful or stackless depending on what you need (see ```examples/primes.c```). It's very configurable and extendable (see ```examples/minimal.c```). The minimal version if fully cross-platform, and the timing utilities can be easily implemented for the platform of you choice. I plan to add more out-of-the-box support for micro-controllers.

The idea originated as a way to control an Arduino robot with a bunch of servos that should move independently of one another. I then used it on another [robot controller in python](https://github.com/iita-robotica/rescate_laberinto/blob/master/src/flow_control/sequencer.py).

There are also other options for concurrency in c, most notably [protothreads](https://dunkels.com/adam/pt/) and coroutine libraries like [libdill](https://libdill.org/).

### Running examples

There is a convenient system to compile, run and debug examples on Linux. Go to the root of this repo and bootstrap it with:
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

The concept itself is quite simple. It can be boiled down to:

```c
#include <stdio.h>
#include <stdbool.h>

int index_;
int counter;

int main() {
    counter = 1;
    while (true) {
        index_ = 0; 

        index_++;
        if (index_ == counter) {
            counter++;
            puts("Do something");
        }

        index_++;
        if (index_ == counter) {
            counter++;
            puts("Do something else");
        }
        // Non sequential code can go here
    }
}
```

Adding some more stuff:

```c
#include <stdio.h>
#include <stdbool.h>

int index_;
int counter;

int main() {
    long cycle_count = 0;

    counter = 1;
    while (true) {
        index_ = 0; 

        index_++;
        if (index_ == counter) {
            counter++;
            puts("Do something");
        }

        index_++;
        if (index_ == counter && cycle_count > 1000000000) {
            cycle_count = 0;
            counter++; // wait till counter hits n cycles
        }

        index_++;
        if (index_ == counter) {
            counter++;
            puts("Do something else");
        }

        index_++;
        if (index_ == counter) {
            counter = 1; // reset sequence
        }

        cycle_count++;

        // Non sequential code can go here
    }
}
```

You can easily imagine substituting cycle_count for a timer. Now add a bit of syntax sugar:

```c
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

    counter = 1;
    while (true) {
        index_ = 0; 

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
