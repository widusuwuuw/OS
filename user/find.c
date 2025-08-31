#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

const char* fmtname(const char *path) {
    const char *p;
    for(p=path+strlen(path); p >= path && *p != '/'; p--);
    p++;
    return p;
}

void find(char *path, const char *filename) {
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    if ((fd = open(path, 0)) < 0) { return; }
    if (fstat(fd, &st) < 0) { close(fd); return; }

    if (st.type == T_DIR) {
        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';
        while (read(fd, &de, sizeof(de)) == sizeof(de)) {
            if (de.inum != 0 && strcmp(de.name, ".") != 0 && strcmp(de.name, "..") != 0) {
                strcpy(p, de.name);
                find(buf, filename);
            }
        }
    } else if (st.type == T_FILE && strcmp(fmtname(path), filename) == 0) {
        printf("%s\n", path);
    }
    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc != 3) { exit(1); }
    find(argv[1], argv[2]);
    exit(0);
}
