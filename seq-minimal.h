/* Minimal version of 'seq' by Alejandro Mohnblatt
 *
 * Extremely simple multithreading library for c. This minimal version is
 * meant for systems with hard memory and performance constraints.
 * It's writen in c89 (ANSI) with no dependencies, so it probably runs on your toaster.
 * More complete versions with control flow and timing are available at:
 * https://github.com/CoolRobotsAndStuff/seq
 *
 * Example usage at the end of the file.
 * 
 * MIT License
 * 
 * Copyright (c) 2024 Alejandro de Ugarriza Mohnblatt
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

typedef struct {
    int index;
    int counter;
} SeqThread;

SeqThread seq_thread() {
    SeqThread ret;
    ret.index   = 0;
    ret.counter = 1;
    return ret;
}

SeqThread* seq_current_thread;

void seq_start() { seq_current_thread->index = 0; }

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

/* Example usage:
 *
 * #include <stdio.h>
 * #include "seq-minimal.h"
 *
 * int main(void) {
 *     SeqThread thread1 = seq_thread();
 *     SeqThread thread2 = seq_thread();
 *
 *     #define T1 seq_current_thread = &thread1;
 *     #define T2 seq_current_thread = &thread2;
 *
 *     while (1) {
 *         T1 seq_start()    ;    T2 seq_start()      ;
 *         T1 seq puts("bim");    T2 seq puts("three");
 *         T1 seq puts("bam");    T2 seq puts("two")  ;
 *         T1 seq puts("bum");    T2 seq puts("one")  ;
 *                                T2 seq puts("zero") ;
 *             seq_sync_any(&thread1, &thread2);
 *                  T1 seq puts("BOOM");
 *     }
 * }
 */
