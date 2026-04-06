#include "common.h"

float get_worker_load(char *ip) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0 ||
        connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        return 999.0;
    }

    write(sock, "LOAD", 4);

    char res[32] = {0};
    read(sock, res, sizeof(res));

    close(sock);
    return atof(res);
}

int main(int argc, char *argv[]) {

   if (argc < 6 || (argc - 4) % 2 != 0) {
        printf("Usage: %s <My_User> <My_IP> <User1> <IP1> ... <file.c>\n", argv[0]);
        return 1;
    }

    char *my_user = argv[1];
    char *my_ip = argv[2];
    char *source_file = argv[argc - 1];

    printf("[MASTER] Compiling %s locally...\n", source_file);

    char comp[256];
    snprintf(comp, sizeof(comp),
             "gcc %s -o /tmp/payload_task", source_file);

    if (system(comp) != 0) {
        printf("Compilation failed!\n");
        return 1;
    }

    float min_load = 999.0;
    char best_ip[50], best_user[50];

    for (int i = 3; i < argc - 1; i += 2) {

        float load = get_worker_load(argv[i + 1]);

        printf("[MASTER] Worker %s (%s) Load: %.2f\n",
               argv[i], argv[i + 1], load);

        if (load < min_load) {
            min_load = load;
            strcpy(best_user, argv[i]);
            strcpy(best_ip, argv[i + 1]);
        }
    }

    printf("[MASTER] Selected Worker: %s (%s)\n", best_user, best_ip);

    char scp_cmd[256];
    snprintf(scp_cmd, sizeof(scp_cmd),
             "scp /tmp/payload_task %s@%s:/tmp/payload_task",
             best_user, best_ip);
    system(scp_cmd);

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in w_addr;
    w_addr.sin_family = AF_INET;
    w_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, best_ip, &w_addr.sin_addr);

    connect(sock, (struct sockaddr*)&w_addr, sizeof(w_addr));

    char run_msg[128];
    snprintf(run_msg, sizeof(run_msg),
             "RUN %s %s", my_user, my_ip);

    write(sock, run_msg, strlen(run_msg));
    close(sock);

    printf("[MASTER] Waiting for result...\n");

    sleep(5);

    printf("\n--- RESULT FROM %s ---\n", best_ip);
    system("cat /tmp/final_output.txt");

    return 0;
}
