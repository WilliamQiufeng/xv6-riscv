//
// Created by willi on 24-8-14.
//

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define BUFFER_SIZE 256
#define NARGV_SIZE 16
char c = '\0';
char buf[BUFFER_SIZE];
char *buf_start = buf;
int buf_i = 0;
int nargv_start_i = 0;

void process_char(int *nargv_i, char *nargv[]) {
    if (c == '\n') {
        if (buf_start != buf + buf_i) {
            char prev_c = c;
            c = ' ';
            process_char(nargv_i, nargv);
            c = prev_c;
        }
        if (fork() == 0) {
            exec(nargv[0], nargv);
            fprintf(2, "exec %s failed\n", nargv[0]);
            exit(1);
        } else {
            int status;
            wait(&status);
        }
        *nargv_i = nargv_start_i;
        nargv[nargv_start_i + 1] = 0;
        return;
    } else if (c == ' ') {
        buf[buf_i++] = '\0';
        nargv[(*nargv_i)++ % NARGV_SIZE] = buf_start;
        buf_start = buf + buf_i;
        return;
    }
    buf[buf_i++ % BUFFER_SIZE] = c;
}

int main(int argc, char *argv[]) {
    int nargv_i = argc - 1;
    nargv_start_i = nargv_i;
    char *nargv[argc + NARGV_SIZE];
    memcpy(nargv, argv + 1, sizeof(char *) * (argc - 1));
    int n = 0;
    while ((n = read(0, &c, sizeof(c))) > 0) {
        process_char(&nargv_i, nargv);
    }
    if (c != '\n') {
        c = '\n';
        process_char(&nargv_i, nargv);
    }
    exit(0);
}
