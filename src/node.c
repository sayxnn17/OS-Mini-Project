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
