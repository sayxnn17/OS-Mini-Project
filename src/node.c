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

