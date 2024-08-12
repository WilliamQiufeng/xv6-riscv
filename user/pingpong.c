//
// Created by willi on 24-8-12.
//
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    int rw_parent2child[2];
    int rw_child2parent[2];
    if (pipe(rw_parent2child) != 0)
        exit(1);
    if (pipe(rw_child2parent) != 0)
        exit(1);

    if (fork() == 0) {
        int pid = getpid();

        // child
        uint8 frame;
        // blocked read byte
        while (1) {
            int n = read(rw_parent2child[0], &frame, 1);
            if (n == 1) break;
            if (n <= 0)
                exit(n);
        }

        // print ping
        printf("%d: received ping\n", pid);

        // send pong
        frame = 123;
        int n = write(rw_child2parent[1], &frame, 1);
        exit(n);
    } else {
        int pid = getpid();

        uint8 frame = 111;
        int n = write(rw_parent2child[1], &frame, 1);
        if (n <= 0)
            exit(-1);
        while (1) {
            n = read(rw_child2parent[0], &frame, 1);
            if (n == 1) break;
            if (n <= 0)
                exit(n);
        }
        printf("%d: received pong\n", pid);
        exit(0);
    }
}
