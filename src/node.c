#include "common.h"
#include <sys/socket.h>

// --- Global Variables ---
char best_ip[50] = ""; 
char my_real_ip[50] = ""; 
float min_load = 9999.0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;


// --- Helper: Get the correct SSH username. Returns 1 if found, 0 if not. ---
int get_ssh_user(const char *ip, char *username) {
    FILE *file = fopen("nodes.txt", "r");
    if (file != NULL) {
        char file_ip[50];
        char file_user[50];

        // Read the file line-by-line looking for a matching IP
        while (fscanf(file, "%49s %49s", file_ip, file_user) == 2) {
            if (strcmp(ip, file_ip) == 0) {
                strcpy(username, file_user); // Found a match!
                fclose(file);
                return 1; // Success
            }
        }
        fclose(file);
    }
    
    // If we reach here, the file was missing or the IP wasn't inside it.
    return 0; // Failure
}

// --- Helper: Get actual LAN IP ---
void get_my_lan_ip(char *buffer) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in serv = {0};
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr("8.8.8.8"); 
    serv.sin_port = htons(53);

    if (connect(sock, (struct sockaddr*)&serv, sizeof(serv)) == 0) {
        struct sockaddr_in name;
        socklen_t namelen = sizeof(name);
        getsockname(sock, (struct sockaddr*)&name, &namelen);
        strcpy(buffer, inet_ntoa(name.sin_addr));
    } else {
        strcpy(buffer, "127.0.0.1"); 
    }
    close(sock);
}
// ================= GOSSIP SENDER =================
// Broadcast load to network
void* shout_load(void* arg) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt));

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_UDP);
    addr.sin_addr.s_addr = inet_addr("255.255.255.255");

    while(1) {
        double load[1];
        getloadavg(load, 1);

        float true_load = load[0] / get_nprocs();

        char msg[50];
        snprintf(msg, sizeof(msg), "LOAD:%f", true_load);

        sendto(sock, msg, strlen(msg), 0,
               (struct sockaddr*)&addr, sizeof(addr));

        sleep(3);
    }
}


// ---------------- GOSSIP LISTENER ----------------
void* listen_load(void* arg) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    #ifdef SO_REUSEPORT
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
    #endif

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_UDP);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("[!] UDP Bind failed");
        return NULL;
    }

    char buf[50];
    struct sockaddr_in sender;
    socklen_t len = sizeof(sender);

    while(1) {
        memset(buf, 0, sizeof(buf));
        recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr*)&sender, &len);

        float rcv_load;
        if (sscanf(buf, "LOAD:%f", &rcv_load) == 1) {
            char sender_ip[50];
            strcpy(sender_ip, inet_ntoa(sender.sin_addr));
            
            if (strcmp(sender_ip, my_real_ip) != 0 && strcmp(sender_ip, "127.0.0.1") != 0) {
                pthread_mutex_lock(&lock);
                if (rcv_load < min_load) {
                    min_load = rcv_load;
                    strcpy(best_ip, sender_ip);
                }
                pthread_mutex_unlock(&lock);
            }
        }
    }
    return NULL;
}

/* ====== TCP SERVER(acts to accept execution requests) ======*/
void* tcp_server(void* arg) {
    int server = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_TCP);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server, (struct sockaddr*)&addr, sizeof(addr));
    listen(server, 5);

    while(1) {
        int client = accept(server, NULL, NULL);

        int *p = malloc(sizeof(int));
        *p = client;

        pthread_t t;
        pthread_create(&t, NULL, handle_task, p);
        pthread_detach(t);
    }
}

// ---------------- TASK EXECUTION ----------------
void* handle_task(void* arg) {
    int sock = *(int*)arg;
    free(arg);
    char buf[1024] = {0};

    if (read(sock, buf, sizeof(buf)) > 0 && strncmp(buf, "EXEC", 4) == 0) {
        system("chmod +x /tmp/payload");
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "timeout %s /tmp/payload > /tmp/out.txt", TIMEOUT_SEC);

        if (system(cmd) != 0) {
            const char *err = "ERROR: Task Crashed or Timed Out\n";
            write(sock, err, strlen(err));
        } else {
            FILE *f = fopen("/tmp/out.txt", "r");
            if (f) {
                int n = fread(buf, 1, sizeof(buf), f);
                fclose(f);
                write(sock, buf, n);
            }
        }
    }
    close(sock);
    return NULL;
}
