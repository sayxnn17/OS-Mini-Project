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
    else if (strncmp(buf, "RUN ", 4) == 0) {

        char m_user[50], m_ip[50];
        sscanf(buf, "RUN %s %s", m_user, m_ip);

        system("chmod +x /tmp/payload_task");
        system("/tmp/payload_task > /tmp/output.txt");

        char scp_cmd[256];
        snprintf(scp_cmd, sizeof(scp_cmd),
                 "scp /tmp/output.txt %s@%s:/tmp/final_output.txt",
                 m_user, m_ip);

        system(scp_cmd);
    }

    close(sock);
    return NULL;
}
