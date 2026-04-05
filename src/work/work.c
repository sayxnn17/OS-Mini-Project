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

int main() {

    int server_fd, client_sock;
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 5);

    printf("[WORKER] Active and waiting...\n");
    


 while (1) {
        client_sock = accept(server_fd, (struct sockaddr*)&addr, &len);

        pthread_t tid;
        int *p = malloc(sizeof(int));
        *p = client_sock;

        pthread_create(&tid, NULL, handle_master, p);
        pthread_detach(tid);
    }
return 0;
}
