#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

int main(int argc, char *argv[]) {
    char *new_argv[MAXARG];
    char line_buf[512], *p = line_buf;

    for (int i = 1; i < argc; i++) {
        new_argv[i - 1] = argv[i];
    }

    while (read(0, p, 1) > 0) {
        if (*p == '\n') {
            *p = '\0';
            new_argv[argc - 1] = line_buf;
            new_argv[argc] = 0;
            if (fork() == 0) {
                exec(new_argv[0], new_argv);
                exit(1);
            }
            wait(0);
            p = line_buf;
        } else {
            p++;
        }
    }
    exit(0);
}
