//
// Created by willi on 24-8-12.
//
// --------------------------------------------
// HUGE gotcha: closing a pipe will decrease the file ptr's ref count
// we need to be EXTREMELY careful to not repeatedly close a file in a process, which would otherwise lead to full closing.

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define DEBUG 0
#define log(...) if (DEBUG) printf(__VA_ARGS__);

const int MaxNumber = 35;

typedef int *chan_t;

chan_t chan_make() {
    int *rw = malloc(2);
    if (pipe(rw) < 0)
        return 0;
    return rw;
}

int chan_read(const chan_t chan) {
    int value;
    read(chan[0], &value, 4);
    return value;
}

int chan_tryread(const chan_t chan, int *out) {
    const int result = read(chan[0], out, 4);
    if (result <= 0)
        return 0;
    return 1;
}

int chan_write(const chan_t chan, const int value) {
    return write(chan[1], &value, 4);
}

int child(const int test_number, const chan_t chan_to_close, const chan_t left_chan) {
    const int forked_pid = fork();

    if (forked_pid != 0) {
        // we are parent: close our reading channel
        close(left_chan[0]);
        return forked_pid;
    }

    // we are child: close our writing channel
    close(left_chan[1]);
    if (chan_to_close != 0) {
        // close unused reading channel
        close(chan_to_close[0]);
        // NOTE unused writing channel is already closed!
        // close(chan_to_close[1]);
    }

    printf("prime %d\n", test_number);
    int value;
    chan_t next_chan = 0;

    // process inputs
    while (chan_tryread(left_chan, &value)) {
        if (value % test_number == 0)
            continue;

        // proc not created: create now
        if (next_chan == 0) {
            next_chan = chan_make();
            if (next_chan == 0) {
                log("ERR test_number=%d value=%d\n", test_number, value);
                exit(1);
            }
            child(value, left_chan, next_chan);
        }

        // pass to next proc
        if (chan_write(next_chan, value) < 0)
            exit(1);
    }

    // wait
    int status = 0;
    if (next_chan != 0) {
        close(next_chan[1]);
        wait(&status);
        close(next_chan[0]);
        free(next_chan);
    }

    exit(status);
}

void host() {
    const chan_t gen_chan = chan_make();
    if (gen_chan == 0) {
        printf("Error making channel\n");
        exit(1);
    }

    child(2, 0, gen_chan);
    for (int i = 2; i <= MaxNumber; i++) {
        chan_write(gen_chan, i);
    }
    // close(gen_chan[0]);
    close(gen_chan[1]);

    int status;
    wait(&status);
    close(gen_chan[0]);
    free(gen_chan);
    exit(status);
}

int main(int argc, char *argv[]) {
    host();
}
