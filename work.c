#include "common.h"
#include <pthread.h>

void* handle_master(void* arg) {

    int sock = *(int*)arg;
    free(arg);

    char buf[BUFFER_SIZE];
    memset(buf, 0, BUFFER_SIZE);

    read(sock, buf, BUFFER_SIZE);

    if (strncmp(buf, "LOAD", 4) == 0) {

        double load[1];
        getloadavg(load, 1);

        char load_str[32];
        snprintf(load_str, sizeof(load_str), "%.2f", load[0]);

        write(sock, load_str, strlen(load_str));
    }
